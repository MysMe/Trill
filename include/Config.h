#pragma once
#include <string>
#include <vector>
#include "ProjectDir.h"
#include <fstream>
#include <stdexcept>

namespace config
{
	struct config
	{
		bool interactive = false;
		std::string boardName;
		std::vector<std::string> listFilter;

		static config load()
		{
			std::string fileName = constants::config_dir + "/config.txt";
			auto file = std::ifstream(fileName);
			if (!file.is_open())
			{
				throw std::runtime_error("Could not open config file: " + fileName);
			}

			config config;
			std::string line;
			while (std::getline(file, line))
			{
				if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
				auto delimiterPos = line.find('=');
				if (delimiterPos == std::string::npos) continue; // Skip invalid lines
				auto key = line.substr(0, delimiterPos);
				auto value = line.substr(delimiterPos + 1);
				if (key == "interactive")
					config.interactive = (value == "true");
				else if (key == "board")
					config.boardName = value;
				else if (key == "list")
					config.listFilter.push_back(value);
			}

			if (config.boardName.empty())
			{
				throw std::runtime_error("No board name specified in config file: " + fileName);
			}

			return config;
		}

		bool listInFilter(std::string_view listName) const
		{
			return listFilter.empty() || std::ranges::contains(listFilter, listName);
		}
	};
}