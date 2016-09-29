#pragma once
#include <memory>
#include <iostream>
#include <fstream>
#include <regex>
#include <map>
#include <vector>

namespace libmumbot {
	class MumBotConfig {
	public:
		MumBotConfig(std::string iniFilePath);
		MumBotConfig(){};
		void loadConfig(std::string iniFilePath);
		std::string getScriptForInput(std::string input);
	private:
		std::map< std::string, std::shared_ptr< std::map <std::string, std::string> > > configSections_;
		std::vector<std::shared_ptr<std::map<std::string,std::string>>> scriptSections_;

		void strToLower(std::string &source);

	};
}
