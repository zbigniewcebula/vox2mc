#if 0
#!/bin/bash
g++ -ggdb $0 -Wall -pthread -Wno-unused-result --std=c++17 -O3 -o vox2mc
exit
#endif
#include <iostream>

using namespace std;

#include "VOX.h"
#include "MC.h"

int main(int argc, char** argv) {
	if(argc != 3) {
		cerr	<< "[Error] Incorrect number of arguments given!" << endl;
		return 1;
	}

	VOX model;
	if(not model.LoadFile(argv[1])) {
		cerr	<< "[Error] Cannot open input file!" << endl;
		return 1;
	}

	MarchingCubeModel output;
	output.LoadVoxels(model);
	output.SaveOBJ(argv[2]);

	return 0;
}