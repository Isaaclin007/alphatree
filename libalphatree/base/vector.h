//
// Created by godpgf on 18-1-23.
//

#ifndef ALPHATREE_VECTOR_H
#define ALPHATREE_VECTOR_H
#include <string.h>
#include "iterator.h"
#define MAX_VECTOR_NAME_LEN 256

template <class T>
class IVector: public IteratorClient<T>{
public:
    //virtual Iterator<T>&& iter(){ return Iterator<T>(this);};
    virtual T& operator[](int index) = 0;
    virtual size_t getSize() = 0;
    virtual void initialize(T data) = 0;
    virtual ~IVector(){}
};

template <class T>
class Vector : public IVector<T>{
public:
    Vector(size_t size):isLocalMemory_(true), size_(size){
        memory_ = new T[size_];
    }

    Vector(T* memory, size_t size):isLocalMemory_(false), memory_(memory), size_(size){
    }

    virtual void initialize(T data){
        for(size_t i = 0; i < size_; ++i)
            memory_[i] = data;
    }

    virtual ~Vector(){
        if(isLocalMemory_)
            delete []memory_;
    }

    virtual T& operator[](int index){
        if(index < 0)
            index = ((int)size_) + index;
        return memory_[index];
    }

    virtual size_t getSize(){
        return size_;
    }

    virtual BaseIterator<T>* createIter(){
        return (IBaseIterator<T>*)(new MemoryIterator<T>(memory_, size_));
    }
protected:

    bool isLocalMemory_ = {false};
    T* memory_ = {nullptr};
    size_t size_ = {0};
};

#endif //ALPHATREE_VECTOR_H
