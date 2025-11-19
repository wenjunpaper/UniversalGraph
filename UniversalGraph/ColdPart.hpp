#ifndef _COLD_PART_H
#define _COLD_PART_H

#include "ColdLayer.hpp"

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
class ColdPart {
public:
    using VertexMap = std::unordered_map<ID_TYPE,Vertex<ID_TYPE,WEIGHT_TYPE>>;
    ColdLayer<ID_TYPE, FP_TYPE, WEIGHT_TYPE>** _counter;

    BOBHash32** _hash;
    int _element_num = 0;
    uint8_t _level;
    double _mem_ratio;
    uint64_t _mem_bytes;
    ColdPart(uint64_t mem_in_bytes,uint8_t level=14){
        level = us_level;
        _mem_bytes = mem_in_bytes;
        _level = level;
        _hash = new BOBHash32*[_level];
        _counter = new ColdLayer<ID_TYPE, FP_TYPE, WEIGHT_TYPE>*[_level];
    
        int top_mem_for_sketch = _mem_bytes / ((1<<_level)-1);
        //vector<uint32_t> random_vec = BOBHash32::get_random_prime_index_list(level);
        for(int i = 0; i < _level; i++){
            _counter[i] = new ColdLayer<ID_TYPE, FP_TYPE, WEIGHT_TYPE>(top_mem_for_sketch);
            top_mem_for_sketch*=2;
            _hash[i] = new BOBHash32(i);
        }
    }
    ~ColdPart(){
        delete _hash;
        delete _counter;
    }
    double Gsum(ID_TYPE u,g_func g){
        int exp = 0;
        for (int i = _level-1; i >= 0; i--)
		{
            auto hash_val = (1<<exp) & u;
            exp++;
			if (hash_val)
			{
                if(i == 0){
                    return _counter[i]->Gsum(u,g);
                }
				continue;
			} 
			else
			{
                return _counter[i]->Gsum(u,g);
				break;
			}
		}
        return 0;
    }
    void Insert(ID_TYPE u,ID_TYPE v,WEIGHT_TYPE w){
        _element_num += w;
        int hash_val;
        
        int exp = 0;
        for (int i = _level-1; i >= 0; i--)
		{
            hash_val = (1<<exp) & u;
            exp++;
			if (hash_val)
			{
                if(i == 0){
                    _counter[i]->Insert(u,v,w);
                }
				continue;
			}
			else
			{
                _counter[i]->Insert(u,v,w);
				break;
			}
		}
    }
};

#endif