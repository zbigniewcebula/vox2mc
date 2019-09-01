@echo off
g++ -static-libstdc++ -static-libgcc main.cpp -Wall -pthread -Wno-unused-result --std=c++17 -O3 -o vox2mc.exe