#include "MumBotConfig.h"


using namespace std;
namespace libmumbot {

	void MumBotConfig::strToLower(string &source) {
		const std::locale loc;

		for (int i=0;i < source.length();i++) {
			source[i] = tolower(source[i],loc);
		}
	}

	list<string>  MumBotConfig::getScriptsForInput(string input) {
		list<string> commands;

		for (auto command : scriptSections_) {
			string testRegexStr;
			try {
				testRegexStr = command->at("regex");
				regex testRegex(testRegexStr);
				if (regex_match(input,testRegex)) {
					string commandStr = command->at("script");
					commands.push_back(commandStr);
				}
			}
			catch (const std::exception& e) { //command doesnt have a regex section
				std::cout << e.what();
			}


		}
		return commands;
	}
	string MumBotConfig::getSectionSetting(string section, string setting) {

		try {
			auto ptr = configSections_.at(section);
			string val = ptr->at(setting);
			return val;

		}
		catch (exception& e) {

		}
		return "";
	}

	void MumBotConfig::loadConfig(string iniPath) {
		ifstream inifile(iniPath);
		smatch matchObj;
		regex sectionRegex("(^\\[)(.*)(\\])");
		regex keyValPairRegex("(^\\w*)(\\s*=\\s*)(.*)");
		regex whiteLineRegex("^[\\s]*$");
		shared_ptr<map<string,string>> currentKeyValPtr;

		for (string line;getline(inifile,line);) {

			if (regex_match(line,matchObj,keyValPairRegex) && matchObj.size() == 4) { //key value setting
				string key = matchObj[1].str();
				string val = matchObj[3].str();
				strToLower(key);
				(*currentKeyValPtr)[key] = val;
			}
			else if (regex_match(line, matchObj, sectionRegex) && (matchObj.size() == 4)){ //section
				string section = matchObj[2].str();
				strToLower(section);
				currentKeyValPtr = make_shared<map<string,string>>();
				if (section != "command") {
					configSections_[section] = currentKeyValPtr;
				}
				else {
					scriptSections_.push_back(currentKeyValPtr);
				}
			}
			else if (regex_match(line,matchObj,whiteLineRegex)) {

			}
			else if (line[0] == ';') {

			}
			else {
				cout << "Malformed config file line:" << " " << line << "\n";
			}
		}

		for (auto section : configSections_) {
			cout << "Section:" << section.first << "\n";
			for (auto keyValue: *section.second) {
				cout << "Key:" << keyValue.first << "\n";
				cout << "Value:" << keyValue.second << "\n";
			}
		}
	}

	MumBotConfig::MumBotConfig(string iniPath) {
		loadConfig(iniPath);
	}
}
