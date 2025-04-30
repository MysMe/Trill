#pragma once
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>

struct keys
{
    std::string apiKey;
    std::string userToken;
    std::vector<std::string> listFilter;

	//Keys should be stored locally relative to the executable, we do not store the keys in the repository
    static keys loadKeys(const std::string& path)
    {
        keys result;
        std::ifstream api_key(path + "api_key.txt");
        if (!api_key)
        {
            throw std::runtime_error("Error opening file: " + path + "api_key.txt");
        }
        std::getline(api_key, result.apiKey);

        std::ifstream user_token(path + "user_token.txt");
        if (!user_token)
        {
            throw std::runtime_error("Error opening file: " + path + "user_token.txt");
        }
        std::getline(user_token, result.userToken);

		std::ifstream list_filter(path + "list_filter.txt");
        std::string list;
        while (std::getline(list_filter, list))
        {
            result.listFilter.push_back(list);
		}

        return result;
    }

	static keys loadKeys(const std::filesystem::path& path)
	{
		return loadKeys(path.string());
	}
};
