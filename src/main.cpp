#if 0
#!/bin/bash
g++ $0 -Wall -fpermissive -pthread -msse2 -msse4.1 -fopenmp\
	-Wno-unused-result --std=c++17 -O2 -o ./../bin/vox2mc
exit
#endif


#include <iostream>
#include <chrono>
#include <algorithm>

using std::string;
using std::cerr;
using std::cout;
using std::flush;
using std::endl;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::time_point;

#include "Helper.h"
#include "ParamManager.h"

#include "VOX.h"
#include "MC.h"

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
		"-id", "--input-dir", "Sets input directory for recursively find VOX files (use with -od flag)",
		"INPUT_VOX_DIR"
	);
	paramManager.addParam(
		"-od", "--output-dir", "Sets output directory for final OBJ files (use with -id flag), copies '-id inDirs' structure", 
		"OUTPUT_VOX_DIR"
	);
	
	paramManager.addParamSeparator();

	paramManager.addParam("-s", "--scale", "Changes scale of output OBJ, default: 0.03125", "SCALE");
	paramManager.addParam("-u", "--upscale", "Changes upscaling factor of conversion, default: 3.0", "FACTOR");

	paramManager.addParam("-fx", "--flip-x", "Flips model by mirroring X axis", "");
	paramManager.addParam("-fy", "--flip-y", "Flips model by mirroring Y axis", "");
	paramManager.addParam("-fz", "--flip-z", "Flips model by mirroring Z axis", "");

	paramManager.addParamSeparator();

	paramManager.addParam("-t", "--time", "Shows time of VOX to OBJ conversion", "");

	if(paramManager.process(argc, argv) == false)
		return 1;
	//--

	//Gathering values
	float	scale	= paramManager.hasValue("-s")?
		Helper::String2Float(paramManager.getValueOf("-s")): 0.03125f;
	float	upscale	= paramManager.hasValue("-u")?
		Helper::String2Float(paramManager.getValueOf("-u")): 3.0f;

	bool	flipX	= paramManager.hasValue("-fx")?
		paramManager.getValueOf("-fx") == "1": false;
	bool	flipY	= paramManager.hasValue("-fy")?
		paramManager.getValueOf("-fy") == "1": false;
	bool	flipZ	= paramManager.hasValue("-fz")?
		paramManager.getValueOf("-fz") == "1": false;

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

			//Gathering input directories tree
			cout << "[Directories] Scanning input directories tree..." << endl;
			vector<string> inDirs;
			inDirs.push_back(inDir);
			Helper::GetDirectoriesList(inDirs);

			//Output dirs
			vector<string> outDirs;
			for(size_t i = 0; i < inDirs.size(); ++i) {
				outDirs.push_back(Helper::ReplaceAll(inDirs[i], inDir, outDir));
			}

			//Making directory tree copy
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

				//Load
				cout << "[" << idx++ << "] " << entry << " [" << flush;
				
				VOX model;
				if(not model.LoadFile(entry)) {
					cerr	<< "[Error] Cannot open input file!" << endl;
					return 1;
				}
				cout << 'L' << flush;

				//Optional flip
				if(flipX or flipY or flipZ) {
					model.Flip(flipX, flipY, flipZ);
					cout << 'F' << flush;
				}

				//Convert
				MarchingCubeModel output;
				output.LoadVoxels(model, scale, upscale);

				cout << 'V' << flush;

				string 	 outPath = Helper::ReplaceAll(
					entry, inDir, outDir
				);
				
				//Naive replace of VOX to OBJ in filename
				outPath.replace(outPath.length() - 4, 4, ".obj");

				//Save
				output.SaveOBJ(outPath);

				cout << "S]" << flush;

				if(timeShow) {
					cout	<< " (" << duration_cast<milliseconds>(high_resolution_clock::now() - start).count()
							<< "ms)";
				}
				cout << endl;
			}
		} else {
			cerr << "[Directory] Input directory is inaccesible, does not exists or is not a directory!" << endl;
			return 1;
		}
	} else {
		string 	in	= Helper::GetAbsolutePath(paramManager.getValueOf("-i"));
		string	out	= Helper::GetAbsolutePath(paramManager.getValueOf("-o"));

		if(Helper::IsFile(in)) {
			cout << "[*] " << in << " [" << flush;

			//Load
			VOX model;
			if(not model.LoadFile(in)) {
				cerr	<< "[Error] Cannot open input file!" << endl;
				return 1;
			}
			cout << 'L' << flush;

			//Optional flip
			if(flipX or flipY or flipZ) {
				model.Flip(flipX, flipY, flipZ);
				cout << 'F' << flush;
			}

			//Convert & Save
			MarchingCubeModel output;
			output.LoadVoxels(model, scale, upscale);
			cout << 'V' << flush;

			output.SaveOBJ(out);
			cout << "S]" << endl;
		} else {
			cerr << "[File] Input file is inaccesible, does not exists or is not a file!" << endl;
		}
	}
	if(timeShow)
		cout	<< "Done in: " 
				<< (duration_cast<milliseconds>(
			high_resolution_clock::now() - overallTime
		).count() / 1000.0)
				<< "s" << endl;

	return 0;
}