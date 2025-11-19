#ifndef _DETECTORABSTRACT_H
#define _DETECTORABSTRACT_H

#include <vector>
#include <unordered_map>
#include <algorithm>
template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
struct Vertex{
    VERTEX_ID_TYPE _v=0;
    WEIGHT_TYPE _sum=0;
    std::unordered_map<VERTEX_ID_TYPE,WEIGHT_TYPE> _edges;
    Vertex(){
        _edges.clear();
    }
    Vertex(VERTEX_ID_TYPE v,WEIGHT_TYPE sum = 0){
        _v = v;
        _sum = sum;
        _edges.clear();
    }
    void Insert(VERTEX_ID_TYPE u,WEIGHT_TYPE weight){
        if(_edges.find(u)==_edges.end()){
            _edges[u] = 0;
        }
        _edges[u] += weight;
        _sum += weight;
    }
    bool operator==(const Vertex& v){
        if (this->_sum != v._sum || this->_v != v._v || this->_edges != v._edges ){
            return false;
        }
        return true;
    }
};

template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE>
class DetectorAbstract{
public:
    virtual void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight) = 0;
};

#endif