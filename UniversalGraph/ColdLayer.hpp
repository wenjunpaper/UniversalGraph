#ifndef _COLD_LAYER_H
#define _COLD_LAYER_H

#include <vector>
#include <cstring>
#include <cstdint>
#include "../Common/BOBHash32.h"

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
class ColdLayer{
public:
    using VertexMap = std::unordered_map<ID_TYPE,Vertex<ID_TYPE,WEIGHT_TYPE>>;
    class GraphEntry{
    public:
        FP_TYPE v_;
        FP_TYPE u_;
        WEIGHT_TYPE weight_;
        WEIGHT_TYPE overflow_count; 
        explicit GraphEntry():v_(0), u_(0), weight_(0),overflow_count(0){}

        inline void Clear(){v_ = 0;u_=0;weight_=0;overflow_count=0;}
        inline bool IsEmpty(){return weight_ == 0;}

    };

public:
    int width_;
    int height_;
    std::vector<std::vector<GraphEntry>> matrix_;
    BOBHash32* hash_;
public:
    ColdLayer(int mem){
        auto elem_mem = sizeof(typename ColdLayer<ID_TYPE,FP_TYPE,WEIGHT_TYPE>::GraphEntry)-sizeof(ID_TYPE);
        int entryAmount = mem / elem_mem;
        int width = static_cast<int>(sqrt(entryAmount * widthheightRate));
        int height = width / widthheightRate;
        assert(width* height <= entryAmount);
        width_=width;
        height_=height;
        matrix_.resize(height_);
        for(auto &it : matrix_){
            it.resize(width_);
        }
        hash_ = new BOBHash32(0);
    }
    ~ColdLayer(){
        delete hash_;
    }
    void Insert(ID_TYPE _v, ID_TYPE _u, WEIGHT_TYPE _weight);
    
    double Gsum(ID_TYPE _v,g_func g){
        uint32_t vHash = hash_->run((char*)(&_v),sizeof(ID_TYPE));
        uint32_t colIndex = (vHash >> ((sizeof(ID_TYPE) - sizeof(FP_TYPE)) * 8)  ) % height_;
        FP_TYPE colFp = vHash;
        double res = 0;
        for(int rowIndex = 0;rowIndex<width_;rowIndex++){
            auto& entry = matrix_[colIndex][rowIndex];
            if(entry.v_ == colFp){
                res +=g(entry.weight_);
            }
        }
        return res;
    }
};

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
void ColdLayer<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::Insert(ID_TYPE _v, ID_TYPE _u, WEIGHT_TYPE _weight) {
    uint32_t vHash = hash_->run((char*)(&_v),sizeof(ID_TYPE));
    uint32_t uHash = 0;
    uHash = hash_->run((char*)(&_u),sizeof(ID_TYPE));
    uint32_t colIndex = (vHash >> ((sizeof(uint32_t) - sizeof(FP_TYPE)) * 8)  ) % height_;
    uint32_t rowIndex = (uHash >> ((sizeof(uint32_t) - sizeof(FP_TYPE)) * 8)  ) % width_;
    FP_TYPE colFp = vHash;
    FP_TYPE rowFp = uHash;
    auto& entry = matrix_[colIndex][rowIndex];
    if(entry.IsEmpty()){
        entry.v_ = colFp;
        entry.u_ = rowFp;
        entry.weight_ = _weight;
    }else if(entry.v_ == colFp && entry.u_ == rowFp){
        entry.weight_ += _weight;
    }else{
        entry.overflow_count += _weight;
        if(entry.overflow_count > elastic_lamba * entry.weight_){
            entry.Clear();
            entry.v_ = colFp;
            entry.u_ = rowFp;
            entry.weight_ = _weight;
        }
    }
    
}

#endif
