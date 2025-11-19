#ifndef _HOT_PART_H
#define _HOT_PART_H

#include "../Common/BOBHash32.h"
#include "hashTable.h"
#include <unordered_map>
#pragma pack(1)

// #define OVERFLOW_DETECT
// #define OVERFLOW_CORRECT

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE,typename FP_TYPE>
class HotPart{
    using VertexMap = std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>;
    class EdgeEntry{
        public:
            bool IsEmpty(){return _count == 0;}
            EdgeEntry(){};
            EdgeEntry(VERTEX_ID_TYPE u):_u(u){};
            EdgeEntry(VERTEX_ID_TYPE u,WEIGHT_TYPE count):_u(u),_count(count){};
            bool Equal(VERTEX_ID_TYPE u,WEIGHT_TYPE flag){return u == _u &&  _flag == flag;}
            bool EqualFp(FP_TYPE fp,WEIGHT_TYPE flag){
                static FP_TYPE fp_mask = ((uint64_t)1<<FP_BITS)-1;
                if(_flag != flag){
                    return false;
                }
                fp = fp & fp_mask;
                return _fp == fp;
            }
            bool EqualFlag(WEIGHT_TYPE flag){return flag == _flag;}
            WEIGHT_TYPE GetFlag(){return _flag;}
            void SetFlag(int flag){_flag = flag;}
            FP_TYPE GetFp(){return _fp;} 
            void Insert(WEIGHT_TYPE weight){
                #ifdef OVERFLOW_DETECT
                auto prev_count = _count;
                #endif
                _count+=weight;
                #ifdef OVERFLOW_CORRECT
                if(_count < prev_count){
                    _count=0;
                    _count--;
                }
                #endif
                #ifdef OVERFLOW_DETECT
                assert(_count>=prev_count);
                #endif
            }
            void InsertId(VERTEX_ID_TYPE u,WEIGHT_TYPE weight,WEIGHT_TYPE flag){_u = u;_count = weight;_flag = flag;}
            void InsertFp(FP_TYPE fp,WEIGHT_TYPE weight,WEIGHT_TYPE flag){_fp = fp;_count = weight;_flag = flag;}
            bool operator>(const EdgeEntry &e) { return _count > e._count; }
            void Clear(){_u = 0;_count = 0;_fp = 0;_flag=0;}
            void Loss(WEIGHT_TYPE weight){_count-=weight;}
            VERTEX_ID_TYPE GetId(){return _u;}
            WEIGHT_TYPE GetCount(){return _count;}
            EdgeEntry& operator=(const EdgeEntry& other) {
                if (this == &other) return *this; 
                this->_count = other._count;
                this->_u = other._u;
                this->_flag = other._flag;
                this->_fp = other._fp;
                return *this;
            }
        public:
            VERTEX_ID_TYPE _u = 0;
            WEIGHT_TYPE _fp:FP_BITS = 0;
            WEIGHT_TYPE _count:COUNT_BITS = 0;
            WEIGHT_TYPE _flag:FLAG_BITS = 0;
    };
    class VertexEntry{
        public:
            VertexEntry(){};
            VertexEntry(VERTEX_ID_TYPE v):_v(v){};
            VertexEntry(VERTEX_ID_TYPE v,WEIGHT_TYPE count):_v(v),_count(count){};
            WEIGHT_TYPE GetFlag(){return _flag;}
            void SetFlag(WEIGHT_TYPE flag){_flag = flag;}
            void SetId(VERTEX_ID_TYPE id){_v = id;
            }
            bool IsEmpty(){return _count == 0;}
            bool Equal(VERTEX_ID_TYPE v){return v == _v;}
            WEIGHT_TYPE GetCount(){return _count;}
            VertexEntry& operator=(const VertexEntry& other) {
                if (this == &other) return *this; 
                this->_count = other._count;
                this->_v = other._v;
                this->_flag = other._flag;
                return *this;
            }
            void Insert(WEIGHT_TYPE weight){
                    #ifdef OVERFLOW_DETECT
                    auto prev_count = _count;
                    #endif
                    _count+=weight;
                    #ifdef OVERFLOW_CORRECT
                    if(_count < prev_count){
                        _count=0;
                        _count--;
                    }
                    #endif
                    #ifdef OVERFLOW_DETECT
                    assert(_count>=prev_count);
                    #endif
                    assert(_count!=0);
            }
            void Loss(WEIGHT_TYPE weight){
                #ifdef OVERFLOW_DETECT
                assert(_count>=weight);
                #endif
                _count-=weight;
            }
            VERTEX_ID_TYPE GetId(){return _v;}
            void Clear(){_count = 0;_v = 0;}
        private:
            VERTEX_ID_TYPE _v = 0;
            WEIGHT_TYPE _flag:FLAG_BITS = 0;
            WEIGHT_TYPE _count:COUNT_BITS = 0;
    };
    class VertexBucket{
        public:
            static void ClearSwap(){
                u_vec.clear();
                v_vec.clear();
                w_vec.clear();
            }
            static void Recycle(VERTEX_ID_TYPE v,VERTEX_ID_TYPE u,WEIGHT_TYPE w){
                v_vec.emplace_back(v);
                u_vec.emplace_back(u);
                w_vec.emplace_back(w);
            }
            VertexBucket(){
                _entries.resize(vertex_bucket_size);
                assert((1<<FLAG_BITS)-1>=_entries.size());
                for(int i = 0;i<_entries.size();i++){
                    _entries[i].SetFlag(i+1);
                }
                _edge_entries.resize(edge_bucket_size);
                _counter = 0;
                _edge_counter = 0;
            }
            bool Equal(int index, VERTEX_ID_TYPE id) {
                return _entries[index].Equal(id);
            }
            void Insert(int index,const VertexEntry& entry,const std::vector<EdgeEntry>& id_edge_entry){
                auto prev_flag = _entries[index].GetFlag();
                _entries[index] = entry;
                _entries[index].SetFlag(prev_flag);
        
                for(auto edge:id_edge_entry){
                    edge.SetFlag(prev_flag);
                    EdgeInsertById(_entries[index].GetId(),edge.GetId(),edge.GetCount(),edge.GetFlag());
                }
            
            }
            std::vector<EdgeEntry> FindEdgeWithId(int index){
                auto flag = GetEntry(index).GetFlag();
                std::vector<EdgeEntry> res;
                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(flag)){
                        res.emplace_back(_edge_entries[i]);
                    }
                }
                return res;
            }
            void EdgeInsertById(VERTEX_ID_TYPE _v,VERTEX_ID_TYPE _u,WEIGHT_TYPE _weight,WEIGHT_TYPE _flag){
                int index = -1;
                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].Equal(_u,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertId(_u,_weight,_flag);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    return;
                }
                
                _edge_counter+=_weight;
                if(_edge_entries[id_bucket_size-1].GetCount()*elastic_lamba < _edge_counter){
                    Recycle(_v,_edge_entries[id_bucket_size-1].GetId(),_edge_entries[id_bucket_size-1].GetCount());
                    _edge_entries[id_bucket_size-1].InsertId(_u,_weight,_flag);
                    _edge_counter = 0;
                    index = id_bucket_size - 1;
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                }
            }
            int EdgeInsert(VERTEX_ID_TYPE _v,VERTEX_ID_TYPE _u,FP_TYPE _fp,WEIGHT_TYPE _weight,WEIGHT_TYPE _flag){
                int index = -1;
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].EqualFp(_fp,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    if(_edge_entries[index] > _edge_entries[id_bucket_size-1]){
                        _edge_entries[id_bucket_size-1]._fp = HotPart::HashFp(_edge_entries[id_bucket_size-1]._u);
                        _edge_entries[index]._u = _u;
                        swap(_edge_entries[index],_edge_entries[id_bucket_size-1]);
                        while(index+1<edge_bucket_size && _edge_entries[index+1]>_edge_entries[index]){
                            swap(_edge_entries[index],_edge_entries[index+1]);
                            index++;
                        }
                        index = id_bucket_size-1;
                        while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                            swap(_edge_entries[index],_edge_entries[index-1]);
                            index--;
                        }
                    }
                    return _weight;
                }

                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].Equal(_u,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertId(_u,_weight,_flag);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    return _weight;
                }
                
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertFp(_fp,_weight,_flag);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    if(_edge_entries[index] > _edge_entries[id_bucket_size-1]){
                        _edge_entries[id_bucket_size-1]._fp = HotPart::HashFp(_edge_entries[id_bucket_size-1]._u);
                        _edge_entries[index]._u = _u;
                        swap(_edge_entries[index],_edge_entries[id_bucket_size-1]);
                        while(index+1<edge_bucket_size && _edge_entries[index+1]>_edge_entries[index]){
                            swap(_edge_entries[index],_edge_entries[index+1]);
                            index++;
                        }
                        index = id_bucket_size-1;
                        while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                            swap(_edge_entries[index],_edge_entries[index-1]);
                            index--;
                        }
                    }
                    return _weight;
                }
                
                //loss strategy
                _edge_counter+=_weight;
                if(_edge_entries[id_bucket_size-1].GetCount()*elastic_lamba< _edge_counter){
                    Recycle(_v,_edge_entries[id_bucket_size-1].GetId(),_edge_entries[id_bucket_size-1].GetCount());
                    _edge_entries[id_bucket_size-1].InsertId(_u,_weight,_flag);
                    _edge_counter = 0;
                    index = id_bucket_size - 1;
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                }
                
                return _weight;
            }
            void ClearEdge(int index,bool recycle = false){
                auto entry = GetEntry(index);
                bool clear_flag = false;
                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(entry.GetFlag())){
                        _edge_entries[i].Clear();
                        clear_flag = true;
                        if(recycle){
                            Recycle(entry.GetId(),_edge_entries[i].GetId(),_edge_entries[i].GetCount());
                        }
                    }
                }
                if(clear_flag){
                    std::sort(_edge_entries.begin(),_edge_entries.begin()+id_bucket_size,[](auto &a, auto &b) {
                        return a.GetCount() > b.GetCount();
                    });
                }
                clear_flag = false;
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(entry.GetFlag())){
                        _edge_entries[i].Clear();
                        clear_flag = true;
                    }
                }
                if(clear_flag){
                    std::sort(_edge_entries.begin()+id_bucket_size,_edge_entries.end(),[](auto &a, auto &b) {
                        return a.GetCount() > b.GetCount();
                    });
                }
            }
            void Insert(int index,VERTEX_ID_TYPE u,WEIGHT_TYPE weight){
                auto fp = HashFp(u); 
                EdgeInsert(_entries[index].GetId(),u,fp,weight,_entries[index].GetFlag());
                _entries[index].Insert(weight);
            }
            VertexEntry& GetEntry(int index){
                return _entries[index];
            }
            WEIGHT_TYPE GetEntryCount(int index){
                return _entries[index].GetCount();
            }
            WEIGHT_TYPE GetEntryFlag(int index){return _entries[index].GetFlag();}
            bool IsEmpty(int index){
                return _entries[index].IsEmpty();
            }
            void SetId(int index,VERTEX_ID_TYPE id){
                _entries[index].SetId(id);
            }
            void BubbleSort(int index){
                while(true){
                    if(index <= 0 ){
                        break;
                    }
                    if(_entries[index].GetCount() <= _entries[index-1].GetCount()){
                        break;
                    }
                    std::swap(_entries[index],_entries[index-1]);
                    index--;
                }
            }
            void DownStairs(int index){
                int cur_index = vertex_bucket_size-1;
                ClearEdge(cur_index,true);
                if(cur_index<=index){
                    return;
                }
                auto prev_flag = _entries[cur_index].GetFlag();
                while(cur_index > index){
                    _entries[cur_index] = _entries[cur_index-1];
                    cur_index--;
                }
                _entries[cur_index].SetFlag(prev_flag);
            }
            void Remove(int index){
                auto prev_flag = _entries[index].GetFlag();
                ClearEdge(index);
                while(index+1<vertex_bucket_size){
                    _entries[index] = _entries[index+1];
                    index++;
                }
                _entries[index].SetFlag(prev_flag);
                _entries[index].Clear();
            }
           
            void UpdateCounter(WEIGHT_TYPE w){
                _counter+=w;
            }
            void ClearCounter(){
                _counter = 0;
            }
            WEIGHT_TYPE GetCounter(){
                return _counter;
            }
        private:
            std::vector<VertexEntry> _entries;
            WEIGHT_TYPE _counter;
            WEIGHT_TYPE _edge_counter;
        public:
            std::vector<EdgeEntry> _edge_entries;
            inline static std::vector<VERTEX_ID_TYPE> v_vec{};
            inline static std::vector<VERTEX_ID_TYPE> u_vec{};
            inline static std::vector<WEIGHT_TYPE> w_vec{};
        };
    private:
        std::vector<std::vector<VertexBucket>> _buckets;
        int _topk;
        int _f_max;
        int _thres;
        int _bucket_num;
        static uint32_t Hash(VERTEX_ID_TYPE id){return (*hfunc[1])((unsigned char*)(&id), sizeof(id));}
        static uint32_t Hash(uint8_t fp){return (*hfunc[1])((unsigned char*)(&fp), sizeof(fp));} 
        static uint32_t HashFp(VERTEX_ID_TYPE id){return (*hfunc[0])((unsigned char*)(&id), sizeof(id));}
        const static FP_TYPE fp_mask = ((uint64_t)1<<FP_BITS)-1;
    public: 
        HotPart(int mem_in_bytes){
            auto bucket_num = mem_in_bytes / 2 / (vertex_bucket_size * sizeof(typename HotPart<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>::VertexEntry) + id_bucket_size*(sizeof(VERTEX_ID_TYPE)+(COUNT_BITS+FLAG_BITS)/8) + 2 *sizeof(WEIGHT_TYPE));
            _buckets.resize(BUBBLE_ARRAY_SIZE,std::vector<VertexBucket>(bucket_num));
            _bucket_num = bucket_num;
            _f_max = 0;
            _thres = 0;
            _topk = vertex_top_k;
        }
        
        bool KickOut(int kick_num,uint32_t hash_value,VertexBucket& cur_bucket,int entry_index,int array_index){
            
            if(kick_num == 0){
                return false;
            }
            auto v = cur_bucket.GetEntry(entry_index).GetId();
            uint32_t fp = HashFp(v);
#ifdef TEST1
            uint32_t next_hash_value = hash_value;
            if (array_index == 0) {
                next_hash_value += (fp&fp_mask);
            } else {
                next_hash_value -= (fp&fp_mask);
            }
#else
            uint32_t next_hash_value = hash_value ^ Hash(static_cast<uint8_t>(fp & fp_mask));
#endif 
            VertexBucket &next_bucket = _buckets[1 - array_index][next_hash_value % _bucket_num];
            if (cur_bucket.GetEntryCount(entry_index) >
                next_bucket.GetEntryCount(0)) {
                next_bucket.DownStairs(0);
                next_bucket.Insert(0, cur_bucket.GetEntry(entry_index),cur_bucket.FindEdgeWithId(entry_index));
                return true;
            }

            if (KickOut(kick_num - 1, next_hash_value, next_bucket, 0,
                        1 - array_index)) {
                next_bucket.DownStairs(0);
                next_bucket.Insert(0, cur_bucket.GetEntry(entry_index),cur_bucket.FindEdgeWithId(entry_index));
                return true;
            }
            return false;
        }      
        
        void Insert(VERTEX_ID_TYPE _v,VERTEX_ID_TYPE _u,WEIGHT_TYPE _weight){
            VertexBucket::ClearSwap();
            uint32_t hash_key = Hash(_v);
            uint32_t fp = HashFp(_v);
#ifdef TEST1
            uint32_t hash_value[2] = {hash_key, hash_key + (fp & fp_mask)};
#else
            uint32_t hash_value[2] = {hash_key,
                              hash_key ^ Hash(static_cast<uint8_t>(fp & fp_mask))};
#endif
            uint32_t keys[2] = {hash_value[0]%_bucket_num,hash_value[1]%_bucket_num};
            VertexBucket& bucket0 = _buckets[0][keys[0]];
            VertexBucket& bucket1 = _buckets[1][keys[1]];
            if(bucket0.Equal(0,_v)){
                bucket0.Insert(0,_u,_weight);
                if(bucket0.GetEntryCount(0)>_f_max){
                    _f_max = bucket0.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return;
            }
            if(bucket0.IsEmpty(0)){
                bucket0.ClearEdge(0,true);
                bucket0.SetId(0,_v);
                bucket0.Insert(0,_u,_weight);
                return;
            }
            if(bucket1.Equal(0,_v)){
                bucket1.Insert(0,_u,_weight);
                if(bucket1.GetEntryCount(0)>_f_max){
                    _f_max = bucket1.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return;
            }
            if(bucket1.IsEmpty(0)){
                bucket1.ClearEdge(0,true);
                bucket1.SetId(0,_v);
                bucket1.Insert(0,_u,_weight);
                return;
            }
            for(int i = 1;i<vertex_bucket_size;i++){
                if(bucket0.Equal(i,_v)){
                    bucket0.Insert(i,_u,_weight);
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return;
                }
                if (bucket0.IsEmpty(i)) {
                    bucket0.ClearEdge(i,true);
                    bucket0.SetId(i,_v);
                    bucket0.Insert(i,_u,_weight);
                    return;
                }
                if(bucket1.Equal(i,_v)){
                    bucket1.Insert(i,_u,_weight);
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return;
                }
                if (bucket1.IsEmpty(i)) {
                    bucket1.ClearEdge(i,true);
                    bucket1.SetId(i,_v);
                    bucket1.Insert(i,_u,_weight);
                    return;
                }
            }
            
            //loss strategy
            auto min_bucket = bucket0.GetEntryCount(vertex_bucket_size - 1) < bucket1.GetEntryCount(vertex_bucket_size - 1)?&bucket0:&bucket1;
            min_bucket->UpdateCounter(_weight);
            if(min_bucket->GetEntryCount(vertex_bucket_size-1)*elastic_lamba < min_bucket->GetCounter()){
                
                min_bucket->ClearEdge(vertex_bucket_size-1,true);
                min_bucket->SetId(vertex_bucket_size-1,_v);
                min_bucket->Insert(vertex_bucket_size-1,_u,_weight);
                min_bucket->BubbleSort(vertex_bucket_size-1);
                if(min_bucket->GetEntryCount(1)>_thres){
                    if(KickOut(MAX_KICK_OUT,Hash(min_bucket->GetEntry(1).GetId()),*min_bucket,1,0)){
                        min_bucket->Remove(1);
                    }
                }
                min_bucket->ClearCounter();
                return;
            }
            VertexBucket::Recycle(_v,_u,_weight);
        }
        
        void GetSwap(std::vector<VERTEX_ID_TYPE>** swap_v,std::vector<VERTEX_ID_TYPE>** swap_u,std::vector<WEIGHT_TYPE>** swap_w){
            *swap_v = &VertexBucket::v_vec;
            *swap_u = &VertexBucket::u_vec;
            *swap_w = &VertexBucket::w_vec;
            assert(swap_v!=nullptr);
        }
    
        VertexMap QueryAll(){
            VertexMap res;
            for(int i = 0;i<BUBBLE_ARRAY_SIZE;i++){
                for(int j = 0;j<_bucket_num;j++){
                    for(int k = 0;k<vertex_bucket_size;k++){
                        if(!_buckets[i][j].GetEntry(k).IsEmpty()){
                            auto v = _buckets[i][j].GetEntry(k).GetId();
                            res[v] = Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>(v);
                            auto edges = _buckets[i][j].FindEdgeWithId(k);
                            for(auto e:edges){
                                res[v].Insert(e.GetId(),e.GetCount());
                            }
                            res[v]._sum = _buckets[i][j].GetEntry(k).GetCount();
                        }
                    }
                }
            }
            return res;
        }

        std::unordered_map<std::string,WEIGHT_TYPE> GetHeavy(const int topk,const double thres_ratio){
            auto _vertex = QueryAll();
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

        std::unordered_map<VERTEX_ID_TYPE,double> GetTopk(int topk,g_func g){
            auto _vertex = QueryAll();
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
};

#endif