#ifndef _CORRECTDETECTOR_H
#define _CORRECTDETECTOR_H

#include "DetectorAbstract.h"
#include "Param.h"
#include "Gsum.hpp"
#include <unordered_map>
#include <array>
#include <algorithm>
#include <set>
#include <queue>
#include <string>
template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE>
class CorrectDetector:DetectorAbstract<VERTEX_ID_TYPE,WEIGHT_TYPE>{
public:
    using VertexMap = std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>;
    CorrectDetector() {}
    void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
        if(_vertex.find(_v)==_vertex.end()){
            _vertex[_v] = Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>(_v);
        }
        _vertex[_v].Insert(_u,_weight);
    };
   
    std::unordered_map<std::string,WEIGHT_TYPE> GetHeavy(const int topk,const double thres_ratio){
        auto comp = [](const std::pair<VERTEX_ID_TYPE, WEIGHT_TYPE>& a, 
            const std::pair<VERTEX_ID_TYPE, WEIGHT_TYPE>& b) -> bool {
        return a.second > b.second;
        };
        std::priority_queue<std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>,std::vector<std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>>,decltype(comp)> min_heap;
        for(auto elem:_vertex){
            if(min_heap.size()<topk){
                min_heap.push(std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>(elem.first,elem.second._sum));
            }else if(elem.second._sum>min_heap.top().second){
                min_heap.pop();
                min_heap.push(std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>(elem.first,elem.second._sum));
            }
        }
        std::unordered_map<std::string,WEIGHT_TYPE> res;
        while(!min_heap.empty()){
            auto elem = min_heap.top();
            auto threshold = _vertex[elem.first]._sum * thres_ratio;
            for(auto e:_vertex[elem.first]._edges){
                if(e.second > threshold){
                    res[std::to_string(elem.first)+std::to_string(e.first)] = e.second;
                }
            }
            min_heap.pop();
        }
        return res;
    }

    std::unordered_map<VERTEX_ID_TYPE,double> GsumTopk(int topk,g_func g){
        auto comp = [](const std::pair<VERTEX_ID_TYPE, WEIGHT_TYPE>& a, 
                    const std::pair<VERTEX_ID_TYPE, WEIGHT_TYPE>& b) -> bool {
        return a.second > b.second;
        };
        std::priority_queue<std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>,std::vector<std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>>,decltype(comp)> min_heap;
        for(auto elem:_vertex){
            if(min_heap.size()<topk){
                min_heap.push(std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>(elem.first,elem.second._sum));
            }else if(elem.second._sum>min_heap.top().second){
                min_heap.pop();
                min_heap.push(std::pair<VERTEX_ID_TYPE,WEIGHT_TYPE>(elem.first,elem.second._sum));
            }
        }
        std::unordered_map<VERTEX_ID_TYPE,double> res;
        while(!min_heap.empty()){
            auto elem = min_heap.top();
            for(auto e:_vertex[elem.first]._edges){
                res[elem.first]+=g(e.second);
            }
            min_heap.pop();
        }
        return res;
    }
private:
    std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> _vertex;
};

#endif