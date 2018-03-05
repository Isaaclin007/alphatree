//
// Created by 严宇 on 2018/2/14.
//

//todo delete

#ifndef ALPHATREE_SIGNITERATOR_H
#define ALPHATREE_SIGNITERATOR_H

//#include <fstream>
//#include <sstream>
#include "../base/iterator.h"


class BinarySignIterator : public IBaseIterator<size_t>{
public:
    //在所有的信号中从dayBefore天前取样sampleDays的信号，机器学习一般都会需要某个信号前几天的数据，offset=-1就表示取信号发生前一天的数据
    BinarySignIterator(const char* path, size_t dayBefore, size_t sampleDays, size_t allDayNum, int offset = 0):
            dayBefore_(dayBefore),sampleDays_(sampleDays),allDays_(allDayNum),preSize_(0),curIndex_(0),offset_(offset){
        file_.open(path,ios::binary|ios::in);
        size_t curDayIndex = allDays_ - dayBefore_ - sampleDays_;
        size_t allSize = 0;
        file_.seekg(sizeof(size_t) * (allDays_-dayBefore_-1),ios::beg);
        file_.read( reinterpret_cast< char* >( &allSize ), sizeof( size_t ) );
        if(curDayIndex > 0){
            file_.seekg(sizeof(size_t) * (curDayIndex-1),ios::beg);
            file_.read(reinterpret_cast< char* >( &preSize_ ), sizeof( size_t ));
        }
        file_.seekg(sizeof(size_t) * (allDayNum + preSize_), ios::beg);
        readCurDataOffset();
        //file_.read(reinterpret_cast< char* >( &curDataOffset_ ), sizeof( size_t ));

        size_ = allSize - preSize_;
    }

    virtual ~BinarySignIterator(){
        file_.close();
    }

    virtual void operator++() {
        ++curIndex_;
        readCurDataOffset();
        //file_.read(reinterpret_cast< char* >( &curDataOffset_ ), sizeof( size_t ));
    }

    virtual void skip(long size, bool isRelative = true){
        if(isRelative)
            curIndex_ += size;
        else
            curIndex_ = size;

        file_.seekg(sizeof(size_t) * (allDays_ + preSize_ + curIndex_), ios::beg);
        readCurDataOffset();
        //cout<<":"<<curIndex_<<" "<<size_<<endl;
        //file_.read(reinterpret_cast< char* >( &curDataOffset_ ), sizeof( size_t ));
    }
    virtual bool isValid(){ return curIndex_ < size_;}

    virtual size_t&& getValue(){
        return std::move(curDataOffset_);
    }
    virtual long size(){ return size_;}
protected:
    size_t dayBefore_;
    size_t sampleDays_;
    size_t allDays_;
    size_t preSize_;
    //当前是第几个元素
    size_t curIndex_;
    //当前信号数据对应的文件偏移
    size_t curDataOffset_;
    int offset_;
    size_t size_;
    ifstream file_;

    void readCurDataOffset(){
        file_.read(reinterpret_cast< char* >( &curDataOffset_ ), sizeof( size_t ));
        //cout<<curIndex_<<" "<<curDataOffset_<<" ";
        if(offset_ < 0)
            curDataOffset_ -= (size_t)abs(offset_);
        else
            curDataOffset_ += offset_;
        //cout<<curDataOffset_<<endl;
    }
};


class Sign2FeatureIterator : public IBaseIterator<float>{
public:
    Sign2FeatureIterator(IBaseIterator<size_t>* signIter, IBaseIterator<float>* featureIter):signIter_(signIter), featureIter_(featureIter){
        signIter_->skip(0, false);
        featureIter->skip(*(*signIter), false);
    }

    virtual ~Sign2FeatureIterator(){
        delete signIter_;
        delete featureIter_;
    }

    virtual void operator++() {
        ++(*signIter_);
        featureIter_->skip(*(*signIter_), false);
    }

    virtual void skip(long size, bool isRelative = true){
        signIter_->skip(size, isRelative);
        featureIter_->skip(*(*signIter_), false);
        //cout<<"feature skip "<<*(*signIter_)<<" "<<*(*featureIter_)<<endl;
    }

    virtual bool isValid(){ return signIter_->isValid();}

    virtual float&& getValue(){
        return std::move(**featureIter_);
    }
    virtual long size(){ return signIter_->size();}
protected:
    IBaseIterator<size_t>* signIter_;
    IBaseIterator<float>* featureIter_;
};

#endif //ALPHATREE_SIGNITERATOR_H