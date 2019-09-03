@echo off
mkdir bin
g++  -static-libgcc -static-libstdc++ -Wl,-Bstatic -Wl,-Bdynamic ./src/main.cpp^
	-ggdb -msse2 -msse4.1 ^
	-Wall -Wno-unused-result --std=c++17 -O2 -o ./bin/vox2mc.exe
