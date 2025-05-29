#pragma once
#include <string>
#include <vector>
#include "ProjectDir.h"
#include <fstream>
#include <stdexcept>
#include <charconv>
#include "Logging.h"

namespace config
{
	struct config
	{
		bool interactive = false;
		int refreshSeconds = 60;
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

			std::vector<std::string> logTypes;

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
				else if (key == "refreshSeconds")
				{
					auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), config.refreshSeconds);
					if (ec != std::errc())
					{
						throw std::runtime_error("Invalid value for refreshSeconds in config file: " + fileName);
					}
				}
				else if (key == "log")
					logTypes.push_back(value);
			}

			if (config.boardName.empty())
			{
				throw std::runtime_error("No board name specified in config file: " + fileName);
			}

			logger::setEnabledLogTypes(std::move(logTypes));

			return config;
		}

		bool listInFilter(std::string_view listName) const
		{
			return listFilter.empty() || std::ranges::contains(listFilter, listName);
		}
	};
}