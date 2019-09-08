#ifndef __HELPER__
#define __HELPER__

#include <string>
#include <algorithm>
#include <vector>

#include <cstdio>

#include <dirent.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __unix__
	#include <linux/limits.h>
	#include <libgen.h>
#else
	#include "windows.h"
	#undef small
#endif

using std::vector;

class Helper {
	public:
		static string GetAbsolutePath(const char* anyPath) {
#ifdef __unix__
			char	pathBuffer[PATH_MAX];
			realpath(anyPath, pathBuffer);
#else
			char	pathBuffer[MAX_PATH];
			GetFullPathNameA(
				anyPath,
				MAX_PATH,
				pathBuffer,
				nullptr
			);
#endif
			string ret	= string(pathBuffer);
			replace(ret.begin(), ret.end(), '\\', '/');
			return ret;
		}
		static inline string GetAbsolutePath(string anyPath) {
			return GetAbsolutePath(anyPath.c_str());
		}

		static string GetParentPath(const char* anyPath) {
			string	path	= GetAbsolutePath(anyPath);
			char*	pathBuffer;
#ifdef __unix__
			pathBuffer		= dirname(const_cast<char*>(path.c_str()));
#else
			replace(path.begin(), path.end(), '\\', '/');

			size_t	slash	= path.find_last_of('/');
			path			= path.substr(0, slash);
			pathBuffer		= const_cast<char*>(path.c_str());
#endif
			string ret	= string(pathBuffer);
			replace(ret.begin(), ret.end(), '\\', '/');
			return ret;
		}
		static inline string GetParentPath(string anyPath) {
			return GetParentPath(anyPath.c_str());
		}

		static bool IsDir(string path) {
			struct stat info;
			if(stat(path.c_str(), &info) not_eq 0)
				return false;
			return bool(info.st_mode & S_IFDIR);
		}
		static bool IsFile(string path) {
			FILE* handle = fopen(path.c_str(), "r");
			if(handle == nullptr)
				return false;
			fclose(handle);
			return true;
		}

		static bool CreateDir(string path) {
			return CreateDir(path.c_str());	
		}
		static bool CreateDir(const char* path) {
#ifndef __unix__
			return CreateDirectory(path, NULL);
#else
			return mkdir(path, 0755) == 0;
#endif
		}

		static bool StartsWith(const string& haystack, const string& needle) {
			return	needle.length() <= haystack.length() 
			and		equal(needle.begin(), needle.end(), haystack.begin());
		}
		static bool EndsWith(const string& haystack, const string& needle) {
			size_t	pos	= haystack.find(needle);
			return pos not_eq string::npos and pos == (haystack.length() - needle.length());
		}
		static string ReplaceAll(const string& haystack, const string& what, const string& to) {
			string copy	= haystack;
			for(size_t i = copy.find(what); i not_eq string::npos; i = copy.find(what, i + 1))
				copy.replace(i, what.length(), to);
			
			return copy;
		}

		static string ToLower(string input) {
			string ret	= input;
			transform(input.begin(), input.end(), ret.begin(), 
				[](unsigned char c) -> unsigned char {
					return tolower(c);
				}
			);
			return ret;
		}

		static float String2Float(const string& str) {
		#if __unix__
			return stof(str);
		#else
			return atof(str.c_str());
		#endif
		}

		static void GetDirectoriesList(vector<string>& searchIn) {
			vector<string> found;

			for(string path : searchIn) {
				if(IsDir(path)) {
					DIR*	dir	= opendir(path.c_str());
					dirent*	dp	= readdir(dir);
					do {
						string name	= dp->d_name;
						if(name not_eq "." and name not_eq "..") {
							name = GetAbsolutePath(
								(path + "/" + name).c_str()
							);
							if(IsDir(name)) {
								found.push_back(name);
							}
						}
						dp	= readdir(dir);
					} while(dp not_eq NULL);
					closedir(dir);
				}
			}
			if(found.size() == 0)
				return;

			GetDirectoriesList(found);
			searchIn.insert(searchIn.end(), found.begin(), found.end());
		}

		static vector<string> FindFilesWithExtension(vector<string>& searchIn, string ext) {
			vector<string> found;
			ext = "." + ToLower(ext);

			for(string path : searchIn) {
				if(IsDir(path)) {
					DIR*	dir	= opendir(path.c_str());
					dirent*	dp	= readdir(dir);
					do {
						string name	= dp->d_name;
						if(name not_eq "." and name not_eq "..") {
							if(not IsDir(name) and EndsWith(name, ext)) {
								found.push_back(GetAbsolutePath(
									(path + "/" + name).c_str()
								));	
							}
						}
						dp	= readdir(dir);
					} while(dp not_eq NULL);
					closedir(dir);
				}
			}

			return found;
		}

		static bool CreateDirList(vector<string>& list) {
			bool good	= true;
			for(string dir : list) {
				if(not IsDir(dir) and not CreateDir(dir)) {
					good = false;
				}
			}
			return good;
		}
};


#endif