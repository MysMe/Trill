#pragma once
#include <string_view>
#include <filesystem>

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

    const std::string project_dir = cleanFilePath(STRINGSTRING(PROJECT_DIR));
    const std::filesystem::path project_path = project_dir;
}