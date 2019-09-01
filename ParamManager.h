#ifndef __PARAMS_MANAGER__
#define __PARAMS_MANAGER__

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>

#include "Helper.h"

class ParamManager {
	public:
		class Param {
			public:
				/*
				Fucking windows.h contains type called "small"
				Therefore I cannot name variable small cuz compiler
				thinks I use multiple types in variable declaration
				fuck micro$hit... oh irony on GitHub...

				@Edit:	Fixed using #undef small after winapi include...
						who the fuck throught that "small" is good idea for
						"char" alias...
				*/
				std::string small;
				std::string big;
				std::string description;
				std::string valueDescription;

				std::string value	= "";

				Param() {}
				Param(std::string s, std::string b, std::string d, std::string vd)
					:	small(s), big(b), description(d), valueDescription(vd)
				{}
				Param(const Param& org) {
					small				= org.small;
					big					= org.big;
					description			= org.description;
					valueDescription	= org.valueDescription;
					value				= org.value;
				}

				void print() {
					cout	<< right << setw(7) << small << (small.empty()? "": ", ") << left << setw(14) << big
							<< setw(20) << valueDescription
							<< '\t' << setw(40) << description << '\n';
				}

				bool operator==(const Param& rhs) {
					return rhs.small == small and rhs.big == big and description == rhs.description;
				}
				bool operator==(const std::string& rhs) {
					return (small == rhs) or (big == rhs);
				}
		};
	private:
		vector<Param>	params;

	public:
		ParamManager() {
			addParam("-h", "--help", "Shows help", "");
		}

		void addParam(std::string small, std::string big, std::string description, std::string valueDescription) {
			if(find(params.begin(), params.end(), small) == params.end()
			and find(params.begin(), params.end(), big) == params.end()
			) {
				params.emplace_back(small, big, description, valueDescription);
			}
		}
		void addParamSeparator() {
			params.emplace_back("", "", "", "");
		}

		bool exists(std::string flag) {
			return find_if(params.begin(), params.end(), [&](Param& p) -> bool {
				return p == flag;
			}) != params.end();
		}

		bool hasValue(std::string flag) {
			return getValueOf(flag) != "";
		}

		std::string getValueOf(std::string flag) {
			auto	it = find_if(params.begin(), params.end(), [&](Param& p) -> bool {
				return p == flag;
			});
			if(it != params.end()) {
				return (*it).value;
			}
			return "";
		}

		void printList() {
			for(Param p : params)
				p.print();
			cout	<< flush;
		}

		void printHelp() {
			cout	<< "Usage: vox2obj [OPTION [VALUE]]\n\n"
					<< "Converts VOX file(s) into OBJ format file(s)\n\n"
					<< "Available options:\n";
			printList();
			cout	<< "\nFor more visit: https://github.com/zbigniewcebula/vox2obj\n"
					<< endl;
		}

		bool process(int argc, char** argv) {
			std::string		tempStr;
			bool		paramOverload	= false;
			auto		lastParam		= params.end();
			for(int i = 1; i < argc; ++i) {
				tempStr	= argv[i];
				if(Helper::StartsWith(tempStr, "-")) {
					lastParam	= find_if(params.begin(), params.end(), [&](Param& p) -> bool {
						return p == tempStr;
					});
					if(lastParam == params.end()) {
						cerr	<< "Unknown Param \"" << tempStr << "\"! Aborting..." << endl;
						return false;
					} else if((*lastParam) == "-h") {
						printHelp();
						return false;
					} else if((*lastParam) == "-t"
					) {
						(*lastParam).value	= "1";
					} else if((*lastParam).value != "") {
						cerr	<< "Param \"" << tempStr << "\" used multiple times! Aborting..." << endl;
						return false;
					}
				} else {
					if(lastParam != params.end()) {
						(*lastParam).value	= tempStr;
					} else {
						paramOverload	= true;
					}
					lastParam	= params.end();
				}
			}
			if(paramOverload) {
				cout	<< "WARNING! Too much Params, ignoring not used..." << endl;
			}
			return true;
		}
};

#endif