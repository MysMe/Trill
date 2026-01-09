#pragma once
#include <string_view>
#include <string>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

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
    inline std::string project_dir = cleanFilePath(STRINGSTRING(PROJECT_DIR));
    inline std::filesystem::path project_path = project_dir;

    inline std::string resource_dir = (project_path / "resources").generic_string();
    inline std::filesystem::path resource_path = project_path / "resources";

    inline std::string avatar_dir = (project_path / "avatars").generic_string();
    inline std::filesystem::path avatar_path = project_path / "avatars";

	inline std::string config_dir = (project_path / "config").generic_string();
	inline std::filesystem::path config_path = project_path / "config";

    inline std::string log_dir = (project_path / "logs").generic_string();
    inline std::filesystem::path log_path = project_path / "logs";

    inline void setWorkingDirectory(const std::filesystem::path& workingDirectory)
    {
        auto normalized = std::filesystem::absolute(workingDirectory);
        if (!std::filesystem::exists(normalized))
        {
            throw std::runtime_error("Working directory does not exist: " + normalized.string());
        }
        if (!std::filesystem::is_directory(normalized))
        {
            throw std::runtime_error("Working directory is not a directory: " + normalized.string());
        }

        project_path = normalized;
        project_dir = project_path.generic_string();
        resource_path = project_path / "resources";
        resource_dir = resource_path.generic_string();
        avatar_path = project_path / "avatars";
        avatar_dir = avatar_path.generic_string();
        config_path = project_path / "config";
        config_dir = config_path.generic_string();
        log_path = project_path / "logs";
        log_dir = log_path.generic_string();
    }
}