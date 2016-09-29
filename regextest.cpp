#include <iostream>
#include <fstream>
#include <regex>
#include <map>
#include <memory>
using namespace std;


int main() {
	ifstream inifile("test.ini");
	smatch matchObj;
	regex sectionRegex("(^\\[)(.*)(\\])");
	regex keyValPairRegex("(^\\w*)(\\s*=\\s*)(.*)");
	multimap<string,shared_ptr<map<string,string>>> configOptions_;

	//regex sectionRegex("^(\\[)");
	shared_ptr<map<string,string>> currentKeyValPtr;

	for (string line;getline(inifile,line);) {
		regex_match(line,matchObj,keyValPairRegex);
		if (matchObj.size() == 4) {
			string key = matchObj[1].str();
			string val = matchObj[3].str();
			(*currentKeyValPtr)[key] = val;
		}
		else {
			regex_match(line, matchObj, sectionRegex);
			if (matchObj.size() == 4) {
				string section = matchObj[2].str();
				currentKeyValPtr = make_shared<map<string,string>>();
				configOptions_.insert(pair<string,shared_ptr<map<string,string>>>(section,currentKeyValPtr));
			}
		}

	}

	for (auto section : configOptions_) {
		cout << "Section:" << section.first << "\n";
		for (auto keyValue: *section.second) {
			cout << "Key:" << keyValue.first << "\n";
			cout << "Value:" << keyValue.second << "\n";
		}
	}
	return 0;
}
