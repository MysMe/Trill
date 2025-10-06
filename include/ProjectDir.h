#pragma once
#include <string_view>
#include <filesystem>
#include <algorithm>

#define STRING(s) #s
#define STRINGSTRING(s) STRING(s)

namespace constants
{
    static std::string cleanFilePath(const char* str)
    {
        std::string result(str);
        for (char& c : result)
        {
            if (c == '\\')
                c = '/';
        }
        result.erase(std::remove_if(result.begin(), result.end(), [](char c) { return c == '\"'; }), result.end());
        return result;
    }

	//The project directory is defined by the build type, with release builds being an empty string (aka the executable directory)
    const std::string project_dir = cleanFilePath(STRINGSTRING(PROJECT_DIR));
    const std::filesystem::path project_path = project_dir;

    const std::string resource_dir = project_dir + "/resources";
    const std::filesystem::path resource_path = resource_dir;

    const std::string avatar_dir = project_dir + "/avatars";
    const std::filesystem::path avatar_path = avatar_dir;

	const std::string config_dir = project_dir + "/config";
	const std::filesystem::path config_path = config_dir;

    const std::string log_dir = project_dir + "/logs";
    const std::filesystem::path log_path = log_dir;
}