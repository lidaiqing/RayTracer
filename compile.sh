#!/bin/sh
g++ -O3 -g -fopenmp -std=c++11 svdDynamic.c RayTracer.c utils.c ./Fast-BVH/BBox.cpp ./Fast-BVH/BVH.cpp ./tinyply/source/tinyply.cpp -msse3 -lm -o RayTracer
