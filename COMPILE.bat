@echo off
g++ -static-libstdc++ -static-libgcc main.cpp -msse2 -msse4.1 -fopenmp -Wall -pthread -Wno-unused-result --std=c++17 -O2 -o vox2mc.exe