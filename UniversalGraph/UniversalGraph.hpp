#ifndef _UNIVERSAL_GRAPH_H
#define _UNIVERSAL_GRAPH_H

#include "ColdPart.hpp"
#include "HotPart.hpp"
#include <unordered_map>

template <typename ID_TYPE,typename FP_TYPE,typename WEIGHT_TYPE>
class UniversalGraph{
    public:
        using VertexMap = std::unordered_map<ID_TYPE,Vertex<ID_TYPE,WEIGHT_TYPE>>;
        ColdPart<ID_TYPE,FP_TYPE,uint16_t>* _us=nullptr;
        HotPart<ID_TYPE,WEIGHT_TYPE,FP_TYPE>* _topk=nullptr;
        UniversalGraph(int mem_in_bytes,double ratio = 1.0){
            auto us_mem = mem_in_bytes * ratio;
            auto topk_mem = mem_in_bytes - us_mem;
            _us = new ColdPart<ID_TYPE,FP_TYPE,uint16_t>(us_mem);
            if(topk_mem!=0)
                _topk = new HotPart<ID_TYPE,WEIGHT_TYPE,FP_TYPE>(topk_mem);
        }
        void Insert(ID_TYPE v,ID_TYPE u,WEIGHT_TYPE w){
            static std::vector<ID_TYPE>* swap_v =nullptr;
            static std::vector<ID_TYPE>* swap_u = nullptr;
            static std::vector<WEIGHT_TYPE>* swap_w = nullptr;
            if(_topk!=nullptr){
                _topk->Insert(v,u,w);
                _topk->GetSwap(&swap_v,&swap_u,&swap_w);
                for(int i = 0;i<swap_v->size();i++){
                    _us->Insert((*swap_v)[i],(*swap_u)[i],(*swap_w)[i]);
                }
            }else{
                _us->Insert(v,u,w);
            }
        }
        
        std::unordered_map<ID_TYPE,double> GetTopk(int topk,g_func g){
            auto res = _topk->GetTopk(topk,g);
            for(auto& v:res){
                res[v.first] += g==Cardinality?0: _us->Gsum(v.first,g); 
            }
            return res;
        }   
        
        std::unordered_map<std::string,WEIGHT_TYPE> GetHeavy(const int topk,const double thres_ratio){
            return _topk->GetHeavy(topk,thres_ratio);
            
        }
};

#endif 