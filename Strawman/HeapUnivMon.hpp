#ifndef _HEAP_UNIVMON_H
#define _HEAP_UNIVMON_H

#include <vector>
#include <unordered_map>
#include "UnivMon.hpp"

template <typename ID_TYPE,typename WEIGHT_TYPE>
struct HeapNode
{
    ID_TYPE _id = 0;
    WEIGHT_TYPE _weight = 0;
    USketchPart* univmon = nullptr;
    HeapNode(){Clear();}
    void Init(uint64_t _mem_in_bytes){univmon = new USketchPart(_mem_in_bytes,d,1-count_ratio);univmon->initial();}
    void Insert(ID_TYPE u,ID_TYPE v,WEIGHT_TYPE weight){_id = u;_weight = weight;univmon->insert(v,weight);}
    void Insert(ID_TYPE v,WEIGHT_TYPE weight){_weight+=weight;univmon->insert(v,weight);}
    bool operator>(const HeapNode &e) { return _weight > e._weight; }
    bool operator<(const HeapNode &e) { return _weight < e._weight; }
    bool operator<=(const HeapNode &e){return _weight<=e._weight;}
    ID_TYPE GetKey(){return _id;}
    void Clear(){_id=0;_weight=0;if(univmon!=nullptr){univmon->clear();}}
    void operator=(const HeapNode &e){this->_id = e._id;this->univmon = e.univmon;this->_weight = e._weight;}
    static int GetMemSzie(){return sizeof(ID_TYPE)+sizeof(WEIGHT_TYPE)+(sizeof(univmon)+d*w*4+heap_max*4)*level;}
};


template <typename KEY_TYPE,typename WEIGHT_TYPE>
class HeapUnivMon{
    public:
    HeapUnivMon(){};
    HeapUnivMon(int heap_size,uint64_t mem_in_bytes){
        Init(heap_size,mem_in_bytes);
    }
    void Init(int heap_size,uint64_t mem_in_bytes){
        mem_in_bytes /= heap_size;
        _heap_size = heap_size;
        _heap.resize(_heap_size);
        for(int i = 0;i<heap_size;i++){
            _heap[i].Init(mem_in_bytes);
        }
    }
    void Clear(){
        for(auto& elem:_heap){
            elem.Clear();
        }
        _elem_count = 0;
    }
    bool Insert(const KEY_TYPE& u,const KEY_TYPE& v,WEIGHT_TYPE w){
        // std::cout<<_heap[index_map[3248203593]].univmon->sketches[0]->w<<std::endl;
        auto index = -1;
        for(int i = 0;i<_elem_count;i++){
            if(_heap[i]._id == u){
                index = i;
                break;
            }
        }
        if(index!=-1){
            _heap[index].Insert(v,w);
            AdjustDown(index);
            return true; 
        }
        if(_elem_count<_heap_size){
            _heap[_elem_count].Clear();
            _heap[_elem_count].Insert(u,v,w);
            // index_map[u] = _elem_count++; 
            _elem_count++;
            AdjustUp(_elem_count-1);
            return true;
        }
        else{
            if(_heap[0]._weight < w){
                _heap[0].Clear();
                _heap[0].Insert(u,v,w);
                AdjustDown(0);
                return true;
            }else{
                _heap[0]._weight-=w;
            }
        }
        
        return false;
    }
    std::unordered_map<KEY_TYPE,double> Gsum(double (*g)(double)){
        std::unordered_map<KEY_TYPE,double> res;
        for(int i = 0;i<_elem_count;i++){
            res[_heap[i]._id] = _heap[i].univmon->gsum(g);
        }
        return res;
    }
    std::unordered_map<std::string,WEIGHT_TYPE> GetHeavy(const double thres_ratio){
        std::unordered_map<std::string,WEIGHT_TYPE> res;
        for(int i = _elem_count-1;i>=0;i--){
            auto  threshold = _heap[i]._weight * thres_ratio;
            std::vector<pair<key_type, uint32_t> > ret;
            _heap[i].univmon->get_heavy_hitters(threshold,ret,Sum);
            for(auto elem:ret){
                res[std::to_string(_heap[i]._id)+std::to_string(elem.first)] = elem.second;
            }
        }
        return res;
    }
    private:
    std::vector<HeapNode<KEY_TYPE,WEIGHT_TYPE>> _heap;
    int _heap_size;
    int _elem_count=0;
    void AdjustUp(int index) {
		while (index > 1) {
			int parent = (index - 1) / 2;
			if (_heap[parent] <= _heap[index]) {
				break;
			}
            auto temp = _heap[index];
            _heap[index] = _heap[parent];
            _heap[parent] = temp;
			index = parent;
		}
	}
    void AdjustDown(int index) {
		while (index < _elem_count / 2) {
			int l_child = 2 * index + 1;
			int r_child = 2 * index + 2;
			int larger_one = index;
			if (l_child < _elem_count && _heap[l_child] < _heap[larger_one]) {
				larger_one = l_child;
			}
			if (r_child < _elem_count && _heap[r_child] < _heap[larger_one]) {
				larger_one = r_child;
			}
			if (larger_one != index) {
                auto temp = _heap[index];
                _heap[index] = _heap[larger_one];
                _heap[larger_one] = temp;
				AdjustDown(larger_one);
			}
			else {
				break;
			}
		}
	}
};

#endif