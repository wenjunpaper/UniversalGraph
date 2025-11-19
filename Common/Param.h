#ifndef _PARAM_H
#define _PARAM_H

#include <cstdint>
#include <assert.h>

//Global Config
using VERTEX_ID_TYPE = uint32_t;
using FP_TYPE = uint8_t;
using WEIGHT_TYPE = uint32_t;

//Memory List
const static int memory_list[] = {100,200,300,400};

//Heavy Hitter Detection
const double lambda=0.15;

//HeapUnivMon
const double count_ratio=0.8;
const int d=5;
const int heap_max = 5;
const int w = 10;
const int level = 14;

int vertex_top_k = 500;

//Universal Graph
double us_ratio = 0.3;
double elastic_lamba = 1;

//Cold Layer
double widthheightRate = 0.65;

//Cold Part
int us_level = 8;

//Hot Part
int edge_top_k = 6;
int vertex_bucket_size = 4;
int edge_bucket_size = edge_top_k * vertex_bucket_size;
int id_bucket_size = edge_bucket_size;

const int BUBBLE_ARRAY_SIZE = 2;
const int MAX_KICK_OUT = 1;

const int COUNT_BITS = 20;
const int FP_BITS = 8;
const int FLAG_BITS = 4;

#endif