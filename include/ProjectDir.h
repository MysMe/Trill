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
}