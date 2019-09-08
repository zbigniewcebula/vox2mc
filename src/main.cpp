#if 0
#!/bin/bash
g++ $0 -Wall -fpermissive -pthread -msse2 -msse4.1 -fopenmp\
	-Wno-unused-result --std=c++17 -O2 -o ./../bin/vox2mc
exit
#endif


#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>

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

void CreateMTL(string texturePath, string mtlPath);

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

	paramManager.addParam("-mtl", "--material", "Creates simple MTL file (included in OBJ) with given texture", "TEXTURE_PATH");	

	paramManager.addParamSeparator();

	paramManager.addParam("-s", "--scale", "Changes scale of output OBJ, default: 0.03125", "SCALE");
	paramManager.addParam("-u", "--upscale", "Changes upscaling factor of conversion, default: 3.0", "FACTOR");

	paramManager.addParam("-fx", "--flip-x", "Flips model by mirroring X axis", "");
	paramManager.addParam("-fy", "--flip-y", "Flips model by mirroring Y axis", "");
	paramManager.addParam("-fz", "--flip-z", "Flips model by mirroring Z axis", "");

	paramManager.addParam("-ox", "--offset-x", "Adds offset on X axis", "VOXELS_IN_X");
	paramManager.addParam("-oy", "--offset-y", "Adds offset on Y axis", "VOXELS_IN_Y");
	paramManager.addParam("-oz", "--offset-z", "Adds offset on Z axis", "VOXELS_IN_Z");

	paramManager.addParamSeparator();

	paramManager.addParam("-t", "--time", "Shows time of VOX to OBJ conversion", "");

	if(paramManager.process(argc, argv) == false)
		return 1;
	//--

	//Gathering values
	float	scale	= paramManager.getValueOfFloat("-s", 0.03125f);
	float	upscale	= paramManager.getValueOfFloat("-u", 3.0f);

	bool	flipX	= paramManager.hasValue("-fx")?
		paramManager.getValueOf("-fx") == "1": false;
	bool	flipY	= paramManager.hasValue("-fy")?
		paramManager.getValueOf("-fy") == "1": false;
	bool	flipZ	= paramManager.hasValue("-fz")?
		paramManager.getValueOf("-fz") == "1": false;

	vec<float>	offset(
		paramManager.getValueOfFloat("-ox", 0),
		paramManager.getValueOfFloat("-oy", 0),
		paramManager.getValueOfFloat("-oz", 0)
	);

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

			//MTL Creation
			if(paramManager.hasValue("-mtl"))
				CreateMTL(paramManager.getValueOf("-mtl"), outDir + "/");

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
				output.offset.Set(offset);
				output.LoadVoxels(model, scale, upscale);

				cout << 'V' << flush;

				string 	 outPath = Helper::ReplaceAll(
					entry, inDir, outDir
				);
				
				//Naive replace of VOX to OBJ in filename
				outPath.replace(outPath.length() - 4, 4, ".obj");

				//Fetching model name
				size_t	idx		= outPath.find_last_of('/');
				size_t	idxEnd	= outPath.find_last_of('.');
				output.name 	= outPath.substr(idx + 1, idxEnd - idx - 1);

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

		//MTL Creation
		if(paramManager.hasValue("-mtl"))
			CreateMTL(paramManager.getValueOf("-mtl"), out);

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
			output.offset.Set(offset);
			output.LoadVoxels(model, scale, upscale);
			cout << 'V' << flush;

			//Fetching model name
			size_t	idx		= out.find_last_of('/');
			size_t	idxEnd	= out.find_last_of('.');
			output.name 	= out.substr(idx + 1, idxEnd - idx - 1);

			//Save
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

void CreateMTL(string texturePath, string mtlPath) {
	//Path correction
	mtlPath	= Helper::GetParentPath(mtlPath);

	//MTL Creation
	ofstream	hTex(mtlPath + "/material.mtl", std::ios::trunc bitor std::ios::out);
	if(hTex.good()) {
		if(not Helper::IsFile(texturePath))
			cerr	<< "[Material/Warning] Texture file from given path does not exists!" << endl;

		hTex	<<	"newmtl texture\n"
					"illum 0\n"
					"Ka 0.000 0.000 0.000\n"
					"Kd 1.000 1.000 1.000\n"
					"Ks 0.000 0.000 0.000\n"
					"map_Kd " << texturePath
		<< endl;
	} else
		cerr << "[Material] Cannot open nor create material file! (Ignoring)" << endl;
	hTex.close();
}