#include <algorithm>
#include <string_view>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include "UI.h"
#include <fstream>
#include <chrono>
#include <array>
#include <ranges>
#include <thread>
#include "Logging.h"
#include "ProjectDir.h"

struct errorLogSettings
{
	static constexpr inline std::string logName = "error";
};

int main(int argc, char *argv[])
{
    //Always enable errors by default, that way if an error occurs when parsing the settings the log is generated
    //If errors are then disabled in the config, they won't re-enable
    logger::setEnabledLogTypes({ "error" });

    struct commandLineOptions
    {
        bool interactive = false;
        std::filesystem::path workingDirectory;
    };

    const auto options = [&]()
    {
        commandLineOptions parsed;
        constexpr std::string_view interactiveArgs[] = { "interactive", "i", "-i", "--interactive" };
        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg(argv[i]);
            if (std::ranges::find(interactiveArgs, arg) != std::end(interactiveArgs))
            {
                parsed.interactive = true;
                continue;
            }

            if (arg == "--working-dir" || arg == "-w")
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("Missing value for working directory argument.");
                }
                parsed.workingDirectory = argv[++i];
            }
        }
        return parsed;
    }();

    try
    {
        if (!options.workingDirectory.empty())
        {
            constants::setWorkingDirectory(options.workingDirectory);
        }
    }
    catch (const std::exception &e)
    {
        logger::logFor<errorLogSettings>().logLn(e.what()).flush();
        return 1;
    }

    bool firstRun = true;
reset:
    //When resetting, add a delay to avoid spamming the network
    if (!firstRun)
        std::this_thread::sleep_for(std::chrono::seconds(5));
    firstRun = false;

    try
    {      
		UI::renderToUI(options.interactive);
    }
    catch (const std::exception &e)
    {
        logger::logFor<errorLogSettings>().logLn(e.what()).flush();
        goto reset;
    }
    catch (...)
    {
        logger::logFor<errorLogSettings>().logLn("Unknown error occurred");
        goto reset;
    }
    return 0;
}

#ifdef _WIN32
#include <Windows.h>
//Windows requires a WinMain entry point, we want this when we don't want a console window
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return main(__argc, __argv);
}
#endif