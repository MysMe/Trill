#include <algorithm>
#include <string_view>
#include <iostream>
#include "Console.h"
#include "UI.h"
#include <fstream>
#include <chrono>
#include <curlpp/Options.hpp>
#include <array>
#include <ranges>
#include <thread>

int main(int argc, char *argv[])
{
reset:
    try
    {
        constexpr std::string_view probeArgs[] = {"probe", "p", "--p", "--probe", "-p", "-probe"};
        bool probe = std::any_of(argv, argv + argc, [&probeArgs](const char *arg)
                                 { return std::ranges::find(probeArgs, arg) != std::end(probeArgs); });

        constexpr std::string_view clickArgs[] = {"click", "c", "--c", "--click", "-c", "-click"};
        bool canClick = std::any_of(argv, argv + argc, [&clickArgs](const char *arg)
                                { return std::ranges::find(clickArgs, arg) != std::end(clickArgs); });
        if (probe)
        {
            console::renderToConsole();
        }
        else
        {
            UI::renderToUI(canClick);
        }
    }
    catch (const curlpp::RuntimeError &e)
    {
        std::cerr << e.what() << std::endl;
        // Write error to log file
        std::ofstream logFile("error.log", std::ios::app);
        logFile << std::chrono::system_clock::now() << "#\t" << e.what() << std::endl;
        //Add a small delay to avoid spamming the network
        std::this_thread::sleep_for(std::chrono::seconds(5));
        goto reset;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        //Write error to log file
		std::ofstream logFile("error.log");
		logFile << std::chrono::system_clock::now() << "#\t" << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
		//Write error to log file
		std::ofstream logFile("error.log");
		logFile << std::chrono::system_clock::now() << "#\tUnknown exception." << std::endl;
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