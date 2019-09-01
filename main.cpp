#if 0
#!/bin/bash
g++ -ggdb $0 -Wall -pthread -Wno-unused-result --std=c++17 -O3 -o vox2mc
exit
#endif
#include <iostream>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;

#include "Helper.h"
#include "VOX.h"
#include "MC.h"

#include "ParamManager.h"

int main(int argc, char** argv) {
	//Checking args
	if(argc <= 1) {
		cerr	<< "No parameters given! Aborting..." << endl;
		return 1;
	}

	ParamManager	paramManager;
	paramManager.addParamSeparator();
	paramManager.addParam("-i", "--in", "Sets input VOX file", "INPUT_VOX");
	paramManager.addParam("-o", "--out", "Sets output OBJ file (overrites existing file!)", "OUTPUT_OBJ");
	paramManager.addParam(
		"-id", "--input-dir", "Sets input directory for recursively find VOX files (use with -od flag, relative dir)", "INPUT_VOX_DIR"
	);
	paramManager.addParam(
		"-od", "--output-dir", "Sets output directory for recursively found VOX files (use with -id flag, relative dir), copying -id inDirs structure", "OUTPUT_VOX_DIR"
	);
	
	paramManager.addParamSeparator();

	paramManager.addParam("-s", "--scale", "Changes scale of output OBJ, default: 0.03125", "SCALE");
	paramManager.addParam("-u", "--upscale", "Changes upscaling factor of conversion, default: 3.0", "FACTOR");
	paramManager.addParam("-t", "--time", "Shows time of VOX to OBJ conversion", "");

	if(paramManager.process(argc, argv) == false)
		return 1;
	//--

	float	scale	= paramManager.hasValue("-s")? Helper::String2Float(paramManager.getValueOf("-s")): 0.03125f;
	float	upscale	= paramManager.hasValue("-u")? Helper::String2Float(paramManager.getValueOf("-u")): 3.0f;

	//Time
	bool								timeShow	= paramManager.hasValue("-t");
	time_point<high_resolution_clock>	start		= high_resolution_clock::now();
	time_point<high_resolution_clock>	overallTime	= high_resolution_clock::now();

	//Convertion
	bool multipleFiles = paramManager.hasValue("-id") and paramManager.hasValue("-od");
	if(multipleFiles) {
		string 		inDir	= Helper::GetAbsolutePath(paramManager.getValueOf("-id"));
		if(Helper::IsDir(inDir)) {
			string	outDir	= Helper::GetAbsolutePath(paramManager.getValueOf("-od"));

			cout << "[Directories] Scanning input directories tree..." << endl;
			vector<string> inDirs;
			inDirs.push_back(inDir);
			Helper::GetDirectoriesList(inDirs);

			//Output dirs
			vector<string> outDirs;
			for(size_t i = 0; i < inDirs.size(); ++i) {
				outDirs.push_back(Helper::ReplaceAll(inDirs[i], inDir, outDir));
			}

			cout << "[Directories] Creating new directories tree..." << endl;
			if(not Helper::CreateDirList(outDirs)) {
				cerr	<< "[Error] Problem with output directory creation!" << endl;
				return  1;
			}

			//Output files
			cout << "[Files] Scanning files in directories tree..." << endl;
			auto 	fileToConvert	= Helper::FindFilesWithExtension(inDirs, "vox");
			int		idx				= 1;
			for(string& entry : fileToConvert) {
				if(timeShow)
					start = high_resolution_clock::now();

				cout << "[" << idx++ << "] " << entry << flush;
				
				VOX model;
				if(not model.LoadFile(entry)) {
					cerr	<< "[Error] Cannot open input file!" << endl;
					return 1;
				}
				cout << '.' << flush;

				MarchingCubeModel output;
				output.LoadVoxels(model, scale, upscale);

				cout << '.' << flush;

				string 	 outPath = Helper::ReplaceAll(
					entry, inDir, outDir
				);
				
				//Naive replace of VOX to OBJ in filename
				outPath.replace(outPath.length() - 4, 4, ".obj");

				output.SaveOBJ(outPath);

				cout << '.' << flush;

				if(timeShow) {
					cout	<< " (" << duration_cast<milliseconds>(high_resolution_clock::now() - start).count()
							<< "ms)";
				}
				cout << endl;
			}
		}
	} else {
		string 	in	= Helper::GetAbsolutePath(paramManager.getValueOf("-i"));
		string	out	= Helper::GetAbsolutePath(paramManager.getValueOf("-o"));

		if(Helper::IsFile(in)) {
			VOX model;
			if(not model.LoadFile(in)) {
				cerr	<< "[Error] Cannot open input file!" << endl;
				return 1;
			}

			MarchingCubeModel output;
			output.LoadVoxels(model, scale);
			output.SaveOBJ(out);
		}
	}
	if(timeShow)
		cout	<< "Done in: " 
				<< duration_cast<seconds>(high_resolution_clock::now() - overallTime).count()
				<< "s" << endl;

	return 0;
}