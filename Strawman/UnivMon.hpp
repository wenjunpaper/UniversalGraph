#ifndef UNIVMON_H_INCLUDED
#define UNIVMON_H_INCLUDED

#include "CountHeap.hpp"
#include <ctime>
#include <cstdlib>
#include <vector>
#include <unordered_map>

class USketchPart
{
public:
	typedef CountHeap<key_len> HeavyHitterDetector;
	HeavyHitterDetector** sketches;
	BOBHash32** hash;
	int element_num = 0;
	uint8_t level, d;
	double gama;
	uint64_t mem_in_bytes;
	USketchPart(uint64_t _mem_in_bytes, uint32_t _d, double _gama, uint8_t _level = 14) :level(_level), mem_in_bytes(_mem_in_bytes), d(_d), gama(_gama) {
		sketches = new HeavyHitterDetector * [level];
		hash = new BOBHash32 * [level];
	}
	void initial(const USketchPart* _refer)
	{
		if (mem_in_bytes == 0)
			return;
		int mem_for_sk = int(mem_in_bytes);
		int mem = int(mem_for_sk / level);
		for (int i = 0; i < level; i++)
		{
			sketches[i] = new HeavyHitterDetector(*(_refer->sketches[i]));
			hash[i] = new BOBHash32(*_refer->hash[i]);
			sketches[i] = nullptr;
			hash[i] = nullptr;
		}
	}
	void initial()
	{
		if (mem_in_bytes == 0)
			return;
		int mem_for_sk = int(mem_in_bytes);
		int mem = int(mem_for_sk / level);
		//vector<uint32_t> rd_list = BOBHash32::get_random_prime_index_list(level);
		for (int i = 0; i < level; i++)
		{
			// sketches[i] = new HeavyHitterDetector((1 - gama) * mem, gama * mem, d);
			sketches[i] = new HeavyHitterDetector(mem-heap_max*key_len,d);
			hash[i] = new BOBHash32(i);
		}
	}
	~USketchPart() {
		delete hash;
		delete sketches;
	}
	void clear() {
		if (mem_in_bytes == 0)
			return;
		for (int i = 0; i < level; i++)
		{
			// delete sketches[i];
			// delete hash[i];
			sketches[i]->clear();
		}
	}
	void insert(key_type key, int f = 1) {
		insert((uint8_t*)&key, f);
	}
	void insert(uint8_t* key, int f = 1) {
		element_num += f;
		int hash_val;
		sketches[0]->insert(key, f);
		for (int i = 1; i < level; i++)
		{
			hash_val = hash[i]->run((const char*)key, key_len) & 2;
			if (hash_val)
			{
				sketches[i]->insert(key, f);
			}
			else
			{
				break;
			}
		}
	}
	
	double gsum(double (*g)(double))
	{
		int hash_val, coe;
		vector<pair<string, int>> result;
		vector<double> Y(level);
		for (int i = level - 1; i >= 0; i--)
		{
			sketches[i]->get_top_k_with_frequency(result);
			Y[i] = (i == level - 1) ? 0 : 2 * Y[i + 1];
			double res = 0;
			for (auto kv : result)
			{
				if (kv.second == 0)
				{
					continue;
				}
				hash_val = (i == level - 1) ? 1 : hash[i + 1]->run(kv.first.c_str(), key_len) % 2;
				coe = (i == level - 1) ? 1 : (1 - 2 * hash_val);
				Y[i] += coe * g(double(kv.second));
				res += g(double(kv.second));
			}
		}
		return abs(Y[0]);
		for (int i = level - 1; i >= 0; i--)
		{
			sketches[i]->get_top_k_with_frequency(result);
			Y[i] = 0;
			for (auto kv : result)
			{
				if (kv.second == 0)
				{
					continue;
				}
				Y[i] += g(double(kv.second));
			}
		}
		double res = 0;
		for(int i = 0;i<level;i++){
			if(i == level-1){
				res += Y[i] * (1<<i);
				continue;
			}
			res += (Y[i]-Y[i+1])*(1<<i);
		}
		return res;
	}
	
	void get_heavy_hitters(uint32_t threshold, std::vector<pair<key_type, uint32_t> >& ret,g_func g)
	{
		unordered_map<std::string, uint32_t> results;
		vector<std::pair<std::string, int>> vec_top_k;
		for (int i = level - 1; i >= 0; --i) {
			sketches[i]->get_top_k_with_frequency(vec_top_k);
			for (auto kv : vec_top_k) {
				results[kv.first] = max(results[kv.first], (uint32_t)kv.second);
			}
		}

		ret.clear();
		for (auto& kv : results) {
			if (g(kv.second) >= threshold) {
				ret.emplace_back(make_pair(*(key_type*)(kv.first.c_str()), kv.second));
			}
		}
	}
};

#endif // UNIVMON_H_INCLUDED
