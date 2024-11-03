#pragma once
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>

struct keys
{
    std::string apiKey;
    std::string userToken;

    static keys loadKeys(const std::string& path)
    {
        keys result;
        //Open the "api_key.txt" file
        std::ifstream api_key(path + "api_key.txt");
        //Read the entire contents of the file into a string
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
        return result;
    }

	static keys loadKeys(const std::filesystem::path& path)
	{
		return loadKeys(path.string());
	}
};
