#pragma once
#include <string>
#include <fstream>
#include <format>
#include <ranges>
#include <vector>
#include "ProjectDir.h"

template <class T>
concept logSettings = requires(T t) {
    { t.logName } -> std::convertible_to<std::string>;
};

class logger
{
    static std::string getTime()
    {
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), now };
        return std::format("{:%Y-%m-%d %H:%M:%S}", localTime.get_local_time());
    }

    template <logSettings T>
    class loggerImpl
    {
        static inline bool enabled = true;
        static inline std::ofstream output;
        static inline bool timeOnLine = false;

        void writeTime()
        {
            if (!timeOnLine)
            {
                output << getTime() << ": ";
                timeOnLine = true;
			}
        }

        void open()
        {
            if (!std::filesystem::exists(constants::log_dir))
            {
                std::filesystem::create_directory(constants::log_dir);
            }

            const auto filePath = constants::log_dir + "/" + T::logName + ".log";
            output.open(filePath, std::ios::trunc);
            if (!output)
            {
                throw std::runtime_error("Failed to open log file: " + filePath);
            }
        }

    public:
        loggerImpl(const std::vector<std::string>& enabledTypes)
        {
            enabled = std::ranges::contains(enabledTypes, T::logName);
            if (enabled)
            {
                open();
            }
        }

        loggerImpl& log(std::string_view message)
        {
            if (enabled)
            {
                writeTime();
                output << message;
            }
            return *this;
        }

        loggerImpl& logLn(std::string_view message)
        {
            if (enabled)
            {
                writeTime();
                output << getTime() << ": " << message << '\n';
				timeOnLine = false;
            }
            return *this;
        }

        template <class... Args>
        loggerImpl& flog(std::string_view format, Args&&... args)
        {
            if (enabled)
            {
                writeTime();
                output << std::vformat(format, std::make_format_args(args...));
            }
            return *this;
        }

        template <class... Args>
        loggerImpl& flogLn(std::string_view format, Args&&... args)
        {
            if (enabled)
            {
                writeTime();
                output << std::vformat(format, std::make_format_args(args...)) << '\n';
				timeOnLine = false;
            }
            return *this;
        }

        loggerImpl& addEmptyLines(int count)
        {
            if (enabled)
            {
                for (int i = 0; i < count; ++i)
                {
                    output << '\n';
                }
            }
            return *this;
		}

        loggerImpl& flush()
        {
            if (enabled)
            {
                output << std::flush;
            }
            return *this;
        }

        bool isEnabled() const { return enabled; }
    };

    static inline std::vector<std::string> enabledLogTypes;

public:
	template <logSettings T>
    static loggerImpl<T> logFor()
    {
		static loggerImpl<T> instance(enabledLogTypes);
        return instance;
    }

    static void setEnabledLogTypes(const std::vector<std::string>& logTypes)
    {
        enabledLogTypes = logTypes;
	}
};