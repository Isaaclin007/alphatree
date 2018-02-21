//
// Created by yanyu on 2017/7/12.
//

#ifndef ALPHATREE_ALPHA_H
#define ALPHATREE_ALPHA_H

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "base.h"
//测试
#include <iostream>
using namespace std;

//todo 改bool
enum class CacheFlag{
    CAN_NOT_FLAG = -1,
    NO_FLAG = 0,
    NEED_CAL = 1,
    HAS_CAL = 2,
};


//计算前几天的数据累加
void* sum(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)pars[0];
    size_t d = (size_t)roundf(coff) + 1;

    for(size_t i = 1; i < historySize; ++i){
        _add((pout+i*stockSize),(pout + (i-1)*stockSize), stockSize);
    }
    for(size_t i = historySize-1; i >= d; --i){
        _reduce((pout + i * stockSize), (pout + (i-d) * stockSize), stockSize);
    }
    return pout;
}

//计算前几天数据的累积
void* product(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    memcpy(pout, pleft, historySize * stockSize * sizeof(float));
    size_t d = (size_t)roundf(coff);
    for(size_t i = d; i < historySize; ++i){
        for(int j = 1; j <= d; ++j)
            _mul(pout + i * stockSize, pleft + (i-j) * stockSize, stockSize);
        /*_mulNoZero((pout + i * stockSize), (pout + (i - 1) * stockSize), stockSize);
        if(i > d){
            _divNoZero((pout + i * stockSize), (pleft + (i - 1 - d) * stockSize), stockSize);
        }*/
    }
    return pout;
}


void* mean(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    sum(pars, coff, historySize, stockSize, pflag);
    float* pout = (float*)pars[0];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL){
            _div(pout + i * stockSize, roundf(coff) + 1, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }

    return pout;
}

void* lerp(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _lerp(pleft + i * stockSize, pleft + i * stockSize, pright + i * stockSize, coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pleft;
}

void* delta(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    size_t d = (size_t)roundf(coff);
    float* pout = (float*)pars[0];
    for(size_t i = historySize-1; i >= d; --i) {
        if(pflag[i] == CacheFlag::NEED_CAL){
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _reduce(pout + i * stockSize, pout + (i - d) * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* meanRise(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    coff = roundf(coff);
    size_t d = (size_t)roundf(coff);
    for(size_t i = historySize-1; i >= d; --i) {
        if(pflag[i] == CacheFlag::NEED_CAL){
            _reduce(pout + i * stockSize, pout + (i - d) * stockSize, stockSize);
            _div(pout + i * stockSize, roundf(coff), stockSize);
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            //_reduce(pout + i * stockSize, pleft + (i - d) * stockSize, stockSize);
            //_div(pout + i * stockSize, roundf(coff), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* div(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    //memcpy(pout, pleft, historySize * stockSize * sizeof(float));
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    float* pout = pleft;
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _div((pout + i * stockSize), (pleft + i * stockSize), (pright + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* meanRatio(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    memcpy(pout,pleft,stockSize*historySize* sizeof(float));
    mean(pars+1, coff, historySize, stockSize, pflag);
    return div(pars, coff, historySize, stockSize, pflag);
}

void* add(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    //float* pleft = (float*)(pars[0]);
    float* pright = (float*)(pars[1]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _add((pout + i * stockSize), (pright + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* addFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    //float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _add((pout + i * stockSize), coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* reduce(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    //float* pleft = (float*)(pars[0]);
    float* pright = (float*)(pars[1]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _reduce((pout + i * stockSize), (pright + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* reduceFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    //float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _reduce(coff, (pout + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* reduceTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    //float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _reduce((pout + i * stockSize), coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }

    return pout;
}

void* mul(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pright = (float*)pars[1];
//float* pout = pleft;
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _mul((pout + i * stockSize), (pright + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* mulFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
                float* pout = (float*)pars[0];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _mul((pout + i * stockSize), coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* divFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _div(coff, pout + i * stockSize, stockSize);
        }
        //else {
         //   memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* divTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
     float* pout = (float*)pars[0];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _div(pout + i * stockSize, coff, stockSize);
        }
        //else {
          //  memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* signAnd(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _signAnd((pout + i * stockSize), (pleft + i * stockSize), (pright + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* signOr(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _signOr((pout + i * stockSize), (pleft + i * stockSize), (pright + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* mid(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _add((pout + i * stockSize), (pright + i * stockSize), stockSize);
            _div(pout + i * stockSize, 2.0f, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

//标准差相关------------------------------------------
void* calStddevData(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag, int dataType){
    float* pmean = (float*)pars[1];
    float* pout = pmean;
    float* pleft = (float*)pars[0];
    memcpy(pmean,pleft,stockSize * historySize * sizeof(float));
    mean(pars+1, coff, historySize, stockSize, pflag);

    size_t d = (size_t)roundf(coff);
    //int blockSize = historySize * stockSize;
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL && i >= d){
            for(size_t j = 0; j < stockSize; ++j){
                float sum = 0;
                float meanValue = pmean[i * stockSize + j];
                for(size_t k = 0; k <= d; ++k){
                    float tmp = pleft[(i-k) * stockSize + j] - meanValue;
                    sum += tmp * tmp;
                }
                sum /= (d+1);
                switch(dataType){
                    case 0:
                        //stddev
                        pout[i * stockSize + j] = sqrtf(sum);
                        break;
                    case 1:
                        //up
                        pout[i * stockSize + j] = sqrtf(sum) + meanValue;
                        break;
                    case 2:
                        //down
                        pout[i * stockSize + j] = meanValue - sqrtf(sum);
                        break;
                }
            }
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}

    }
    return pout;
}
void* stddev(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    return calStddevData(pars, coff, historySize, stockSize, pflag, 0);
}
void* up(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    return calStddevData(pars, coff, historySize, stockSize, pflag, 1);
}
void* down(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    return calStddevData(pars, coff, historySize, stockSize, pflag, 2);
}
//----------------------------------------------------------------------
void* ranking(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = pout;
    float* pindex = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            float* curIndex = pindex + i * stockSize;
            //flagNan(curOut, curStockFlag, stockSize);
            _ranksort(curIndex, (pleft + i * stockSize), stockSize);
            _rankscale((pout + i * stockSize), curIndex, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}
//void* rankSort(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
//    for(size_t i = 0; i < historySize; ++i){
//        if(pflag[i] == CacheFlag::NEED_CAL){
//            float* curOut = pout + i * stockSize;
//            //flagNan(curOut, curStockFlag, stockSize);
//            _ranksort(curOut, (pleft + i * stockSize), (pStockFlag + i * stockSize), stockSize);
//        } else {
//            memset(pout + i * stockSize, 0, stockSize * sizeof(float));
//        }
//    }
//    return pout;
//}
//
//void* rankScale(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
//    for(size_t i = 0; i < historySize; ++i){
//        if(pflag[i] == CacheFlag::NEED_CAL){
//            _rankscale((pout + i * stockSize), (pleft + i * stockSize), stockSize);
//        } else {
//            memset(pout + i * stockSize, 0, stockSize * sizeof(float));
//        }
//    }
//    return pout;
//}



void* powerMid(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            _powerMid(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* tsRank(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    memset(pout, 0, sizeof(float)*historySize*stockSize);
    size_t d = (size_t)roundf(coff);
    for(size_t i = 1; i < d + 1; ++i){
        for(size_t j = i; j <historySize; ++j){
            int curBlockIndex = j * stockSize;
            int beforeBlockIndex = (j-i) * stockSize;
            if(pflag[j] == CacheFlag::NEED_CAL){
                _tsRank(pout + curBlockIndex, pleft + curBlockIndex, pleft + beforeBlockIndex, stockSize);
            }
        }
    }
    size_t size = historySize * stockSize;
    for(size_t i = 0; i < size; ++i)
        pout[i] /= d;
    return pout;
}

void* delay(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    size_t d = (size_t)roundf(coff);
    for(size_t i = historySize-1; i >= d; --i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            //cout<<(pout + i * stockSize)[19]<<":"<<(pleft + (i-d) * stockSize)[19]<<endl;
            memcpy(pout + i * stockSize, pleft + (i - d) * stockSize, stockSize * sizeof(float));
            //cout<<(pout + i * stockSize)[19]<<endl;
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* correlation(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    float* pright = (float*)(pars[1]);
    size_t d = (size_t)roundf(coff);
    for(size_t i = historySize-1; i >= d; --i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            //计算第日股票前d天的相关系数
            for(size_t j = 0; j < stockSize; ++j){
                //计算当前股票的均值和方差
                float meanLeft = 0;
                float meanRight = 0;
                float sumSqrLeft = 0;
                float sumSqrRight = 0;
                for(size_t k = 0; k <= d; ++k){
                    int curBlock = (i - k) * stockSize;
                    meanLeft += pleft[curBlock + j];
                    sumSqrLeft += pleft[curBlock + j] * pleft[curBlock + j];
                    meanRight += pright[curBlock + j];
                    sumSqrRight += pright[curBlock + j] * pright[curBlock + j];
                }
                meanLeft /= (d+1);
                meanRight /= (d+1);


                float cov = 0;
                for(size_t k = 0; k <= d; ++k){
                    int curBlock = (i - k) * stockSize;
                    cov += (pleft[curBlock + j] - meanLeft) * (pright[curBlock + j] - meanRight);
                }
                //cov /= (d+1);
                float xDiff2 = (sumSqrLeft - meanLeft*meanLeft*(d+1));
                float yDiff2 = (sumSqrRight - meanRight*meanRight*(d+1));
                if(xDiff2 < 0.000000001 || yDiff2 < 0.000000001)
                    pout[i * stockSize + j] = 1;
                else
                    pout[i * stockSize + j] = cov / sqrtf(xDiff2) / sqrtf(yDiff2);

                /*float test_ydiff2 =0;
                for(size_t k = 0; k <= d; ++k){
                    int curBlock = (i - k) * stockSize;
                    test_ydiff2 += (pright[curBlock + j] - meanRight) * (pright[curBlock + j] - meanRight);
                }
                cout<<j<<":"<<meanLeft<<" "<<meanRight<<" "<<xDiff2<<" "<<yDiff2<<" "<<test_ydiff2<<" "<<cov<<endl;*/
            }
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* scale(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)(pars[0]);
    //float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i)
        if(pflag[i] == CacheFlag::NEED_CAL){
            //memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
            _scale(pout + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    return pout;
}

void* decayLinear(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    //memset(pout, 0, sizeof(float)*historySize*stockSize);
    float* pout = (float*)(pars[0]);
    size_t d = (size_t)roundf(coff) + 1;
    float allWeight = (d + 1) * d * 0.5;
    for(size_t i = historySize; i >= d-1; --i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            for(size_t j = 0; j < d; j++){
                if(j == 0)
                    _mul(pout + i * stockSize, (d - j) / allWeight, stockSize);
                else
                    _addAndMul(pout + i * stockSize, pout + (i - j) * stockSize, (d - j) / allWeight, stockSize);
            }
        }
    }
    return pout;
}

void* tsMin(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    size_t d = (size_t)roundf(coff);
    _tsMinIndex(pout, pleft, historySize, stockSize, d);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            for(size_t j = 0; j < stockSize; ++j){
                int minId = (int)pout[i * stockSize + j];
                pout[i * stockSize + j] = pleft[minId * stockSize + j];
            }
        }

    }
    return pout;
}

void* tsMax(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    size_t d = (size_t)roundf(coff);
    _tsMaxIndex(pout, pleft, historySize, stockSize, d);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            for(size_t j = 0; j < stockSize; ++j){
                int maxId = (int)pout[i * stockSize + j];
                pout[i * stockSize + j] = pleft[maxId * stockSize + j];
            }
        }

    }
    return pout;
}

void* min(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    float* pright = (float*)(pars[1]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {

            //for (int k = 0; k < 20; ++k)
            //    cout<<(pleft + i * stockSize)[k]<<" "<<(pright + i * stockSize)[k]<<endl;
            //cout<<endl;

            _min(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* max(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    float* pright = (float*)(pars[1]);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _max(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* tsArgMin(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    size_t d = (size_t)roundf(coff);
    _tsMinIndex(pout, pleft, historySize, stockSize, d);
    for(size_t i = 0; i < historySize; ++i){
        for(size_t j = 0; j < stockSize; ++j){
            int minId = (int)pout[i * stockSize + j];
            if(minId + d >= i)
                pout[i * stockSize + j] = minId + d - i;
            else
                pout[i * stockSize + j] = 0;
        }
    }
    return pout;
}

void* tsArgMax(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[1]);
    float* pleft = (float*)(pars[0]);
    size_t d = (size_t)roundf(coff);
    _tsMaxIndex(pout, pleft, historySize, stockSize, d);
    for(size_t i = 0; i < historySize; ++i){
        for(size_t j = 0; j < stockSize; ++j){
            int maxId = (int)pout[i * stockSize + j];
            if(maxId + d >= i)
                pout[i * stockSize + j] = maxId + d - i;
            else
                pout[i * stockSize + j] = 0;
        }
    }
    return pout;
}

void* sign(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _sign((pout + i * stockSize), (pleft + i * stockSize), stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* abs(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _abs(pout + i * stockSize, pleft + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }

    return pout;
}

void* log(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float logmax = logf(0.0001);
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _log(pout + i * stockSize, pleft + i * stockSize, stockSize, logmax);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* signedPower(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    float* pright =(float*)(pars[1]);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _pow(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);

        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* signedPowerFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _pow(pout + i * stockSize, coff, pleft + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* signedPowerTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)(pars[0]);
    float* pleft = (float*)(pars[0]);
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _pow(pout + i * stockSize, pleft + i * stockSize, coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* lessCond(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _lessCond(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* moreCond(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    lessCond(pars, coff, historySize, stockSize, pflag);
    return reduceFrom(pars, 1, historySize, stockSize, pflag);
}

void* lessCondFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    //float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _lessCond(pout + i * stockSize, coff, pleft + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* lessCondTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    //float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _lessCond(pout + i * stockSize, pleft + i * stockSize, coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* moreCondFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    //float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _moreCond(pout + i * stockSize, coff, pleft + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* moreCondTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    //float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _moreCond(pout + i * stockSize, pleft + i * stockSize, coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* elseCond(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _elseCond(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* elseCondTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _elseCond(pout + i * stockSize, pleft + i * stockSize, coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* ifCond(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _ifCond(pout + i * stockSize, pleft + i * stockSize, pright + i * stockSize, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* ifCondTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL) {
            _ifCond(pout + i * stockSize, pleft + i * stockSize, coff, stockSize);
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* indneutralize(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    /*for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            memcpy(pout + i * stockSize, pleft + i * stockSize, stockSize * sizeof(float));
        } else {
            memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        }
    }*/
    return pars[0];
}

void* kd(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    int curIndex = 0;
    for(size_t i = 0; i < historySize; ++i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            for(size_t j = 0; j < stockSize; ++j){
                curIndex = i * stockSize + j;
                if(i > 0)
                    pout[curIndex] = pout[curIndex - stockSize] * (2.f / 3.f) + pleft[curIndex] * (1.f / 3.f);
                else
                    pout[curIndex] = 0;
            }
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* cross(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    int curIndex = 0;
    size_t d = (size_t)roundf(coff);
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t i = historySize-1; i >= d; --i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            for(size_t j = 0; j < stockSize; ++j){
                curIndex = i * stockSize + j;
                pout[curIndex] = (pleft[curIndex - d * stockSize] < pright[curIndex - d * stockSize] && pleft[curIndex] > pright[curIndex]) ? 1 : 0;
            }
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* crossFrom(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    int curIndex = 0;
    size_t d = (size_t)roundf(coff);
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    for(size_t i = historySize-1; i >= d; --i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            for(size_t j = 0; j < stockSize; ++j){
                curIndex = i * stockSize + j;
                pout[curIndex] = (coff < pleft[curIndex - d*stockSize] && coff > pleft[curIndex]) ? 1 : 0;
            }
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* crossTo(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag) {
    int curIndex = 0;
    size_t d = (size_t)roundf(coff);
    float* pout = (float*)pars[0];
    float* pleft = (float*)pars[0];
    for(size_t i = historySize-1; i >= d; --i) {
        if(pflag[i] == CacheFlag::NEED_CAL) {
            for(size_t j = 0; j < stockSize; ++j){
                curIndex = i * stockSize + j;
                pout[curIndex] = (pleft[curIndex - d*stockSize] < coff && pleft[curIndex] > coff) ? 1 : 0;
            }
        }
        //else {
        //    memset(pout + i * stockSize, 0, stockSize * sizeof(float));
        //}
    }
    return pout;
}

void* match(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    //cout<<"a\n";
    mulFrom(pars, -1, historySize, stockSize, pflag);
    int curIndex = 0;
    float* pout = (float*)pars[0];
    //float* pleft = (float*)pars[0];
    float* pright = (float*)pars[1];
    for(size_t j = 0; j < stockSize; ++j){
        //cout<<j<<endl;
        int buyIndex = -1;
        for(size_t i = 0; i < historySize; ++i){
            if(pflag[i] == CacheFlag::NEED_CAL){
                curIndex = i * stockSize + j;
                if(buyIndex > 0 && pright[curIndex] > 0){
                    pout[buyIndex] = (curIndex - buyIndex) / stockSize;
                    buyIndex = -1;
                }
                if(pout[curIndex] < 0 && buyIndex < 0){
                    buyIndex = curIndex;
                }
            }
        }
    }
    //cout<<"f\n";
    return pout;
}



void* ftSharp(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    //size_t dataSize = historySize*stockSize;

    float* pout = (float*)pars[2];
    float* sign = (float*)pars[0];
    float* close = (float*)pars[1];
    //float* pright = (float*)pars[2];

    int curIndex = 0;
    float maxDropdown = 0;
    float maxPrice = 0;
    float meanDropdown = 0;
    float sumSqrDropdown = 0;
    size_t signCount = 0;
    for(size_t j = 0; j < stockSize; ++j){
        for(size_t i = 0; i < historySize; ++i){
            if(pflag[i] == CacheFlag::NEED_CAL){
                curIndex = i * stockSize + j;
                if(sign[curIndex] > 0 ){
                    maxPrice = close[curIndex];
                    maxDropdown = 0;
                    for(size_t k = 1; k <= sign[curIndex]; ++k){
                        maxPrice = max(close[curIndex + k * stockSize],maxPrice);
                        maxDropdown = max((maxPrice - close[curIndex + k * stockSize]) / maxPrice, maxDropdown);
                    }
                    pout[curIndex] = maxDropdown;
                    meanDropdown += maxDropdown;
                    ++signCount;
                }
            }
        }
    }
    meanDropdown /= signCount;
    for(size_t j = 0; j < stockSize; ++j){
        for(size_t i = 0; i < historySize; ++i){
            if(pflag[i] == CacheFlag::NEED_CAL){
                curIndex = i * stockSize + j;
                if(sign[curIndex] > 0 ){
                    sumSqrDropdown += powf(pout[curIndex]-meanDropdown, 2);
                }
            }
        }
    }
    //dropdown的不确定性
    float std = sqrtf(sumSqrDropdown / signCount);
    for(size_t j = 0; j < stockSize; ++j){
        for(size_t i = 0; i < historySize; ++i){
            if(pflag[i] == CacheFlag::NEED_CAL){
                curIndex = i * stockSize + j;
                if(sign[curIndex] > 0 ){
                    //风险=dropdown的不确定性+dropdown本身
                    pout[curIndex] = (close[curIndex + (int)sign[curIndex] * stockSize] - close[curIndex])/close[curIndex]/(pout[curIndex]+std);
                }
            }
        }
    }
    return pout;
}


void* resEratio(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* buy = (float*)pars[0];
    float* close = (float*)pars[1];
    float* atr = (float*)pars[2];
    char* pout = (char*)pars[3];

    float MAE[] = {0,0,0,0,0,0};
    float MFE[] = {0,0,0,0,0,0};
    int signCounts[] = {0,0,0,0,0,0};
    float returns[] = {0,0,0,0,0,0};
    float returnsSqr[] = {0,0,0,0,0,0};
    float holdDay = 0;

    float maxPrice = 0;
    float minPrice = FLT_MAX;

    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            for(size_t j = 0; j < stockSize; ++j){
                auto curIndex = i * stockSize + j;
                if(buy[curIndex] > 0){
                    int buyDay = (int)buy[curIndex];
                    holdDay += buyDay;
                    maxPrice = close[curIndex];
                    minPrice = close[curIndex];
                    for(int k = 1; k <= buyDay; ++k){
                        //已经买入,等待卖出
                        maxPrice = max(maxPrice, close[curIndex + k * stockSize]);
                        minPrice = min(minPrice, close[curIndex + k * stockSize]);

                        if(k <= 5){
                            ++signCounts[k-1];
                            MFE[k-1] += (maxPrice - close[curIndex]) / max(atr[curIndex],0.001f);
                            MAE[k-1] += (close[curIndex] - minPrice) / max(atr[curIndex],0.001f);
                            returns[k-1] += (close[curIndex + k * stockSize] - close[curIndex]) / max(close[curIndex],0.001f);
                            returnsSqr[k-1] += powf((close[curIndex + k * stockSize] - close[curIndex]) / max(close[curIndex],0.001f), 2);
                        }
                    }
                    ++signCounts[5];
                    //cout<<(maxPrice - close[curIndex])<<"/"<<(close[curIndex] - minPrice)<<endl;
                    MFE[5] += (maxPrice - close[curIndex]) / max(atr[curIndex],0.001f);
                    MAE[5] += (close[curIndex] - minPrice) / max(atr[curIndex],0.001f);
                    returns[5] += (close[curIndex + buyDay * stockSize] - close[curIndex]) / max(close[curIndex],0.001f);
                    returnsSqr[5] += powf((close[curIndex + buyDay * stockSize] - close[curIndex]) / max(close[curIndex],0.001f), 2);
                }
            }
        }
    }

    sprintf(pout, "{\"eratio\" : [%.4f, %.4f, %.4f, %.4f, %.4f, %.4f], \"sharp\" : [%.4f, %.4f, %.4f, %.4f, %.4f, %.4f],\"cnt\" : [%d, %d, %d, %d, %d, %d], \"avg_hold_day\" : %.4f}",
            MAE[0] == 0 ? 1 : MFE[0]/MAE[0],
            MAE[1] == 0 ? 1 : MFE[1]/MAE[1],
            MAE[2] == 0 ? 1 : MFE[2]/MAE[2],
            MAE[3] == 0 ? 1 : MFE[3]/MAE[3],
            MAE[4] == 0 ? 1 : MFE[4]/MAE[4],
            MAE[5] == 0 ? 1 : MFE[5]/MAE[5],
            (returns[0] / signCounts[0]) / sqrtf(returnsSqr[0] / signCounts[0] - powf(returns[0] / signCounts[0], 2)),
            (returns[1] / signCounts[1]) / sqrtf(returnsSqr[1] / signCounts[1] - powf(returns[1] / signCounts[1], 2)),
            (returns[2] / signCounts[2]) / sqrtf(returnsSqr[2] / signCounts[2] - powf(returns[2] / signCounts[2], 2)),
            (returns[3] / signCounts[3]) / sqrtf(returnsSqr[3] / signCounts[3] - powf(returns[3] / signCounts[3], 2)),
            (returns[4] / signCounts[4]) / sqrtf(returnsSqr[4] / signCounts[4] - powf(returns[4] / signCounts[4], 2)),
            (returns[5] / signCounts[5]) / sqrtf(returnsSqr[5] / signCounts[5] - powf(returns[5] / signCounts[5], 2)),
            signCounts[0],signCounts[1],signCounts[2],signCounts[3],signCounts[4],signCounts[5],
            signCounts[5] == 0 ? 0 : holdDay / (float)signCounts[5]
    );
    return pout;
}

void* optShape(void** pars, float coff, size_t historySize, size_t stockSize, CacheFlag* pflag){
    float* buy = (float*)pars[0];
    float* close = (float*)pars[1];
    float* pout = (float*)pars[0];


    int signCounts = 0;
    float returns = 0;
    float returnsSqr = 0;

    float maxPrice = 0;
    float minPrice = FLT_MAX;

    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            for(size_t j = 0; j < stockSize; ++j){
                auto curIndex = i * stockSize + j;
                if(buy[curIndex] > 0){
                    int buyDay = (int)buy[curIndex];
                    maxPrice = close[curIndex];
                    minPrice = close[curIndex];
                    for(int k = 1; k <= buyDay; ++k){
                        //已经买入,等待卖出
                        maxPrice = max(maxPrice, close[curIndex + k * stockSize]);
                        minPrice = min(minPrice, close[curIndex + k * stockSize]);
                    }
                    ++signCounts;
                    returns += (close[curIndex + buyDay * stockSize] - close[curIndex]) / max(close[curIndex],0.001f);
                    returnsSqr += powf((close[curIndex + buyDay * stockSize] - close[curIndex]) / max(close[curIndex],0.001f), 2);
                }
            }
        }
    }

    float s = (returns / signCounts) / sqrtf(returnsSqr / signCounts - powf(returns / signCounts, 2));

    for(size_t i = 0; i < historySize; ++i){
        if(pflag[i] == CacheFlag::NEED_CAL){
            size_t curIndex = i * stockSize;
            pout[curIndex] = s;
        }
    }

    return pout;
}


#endif //ALPHATREE_ALPHA_H
