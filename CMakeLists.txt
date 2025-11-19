cmake_minimum_required(VERSION 3.16)
project(UniversalGraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
set(CMAKE_BUILD_TYPE Debug)
include_directories(.)
include_directories(Common)
include_directories(Dataset)
include_directories(Detector)
include_directories(UniversalGraph)
include_directories(Strawman)

add_executable(gsum gsum_estimation.cpp Common/BOBHash32.cpp)
add_executable(heavy heavy_hitter_detection.cpp Common/BOBHash32.cpp)
# rm -rf CMakeCache.txt CMakeFiles/ Makefile cmake_install.cmake