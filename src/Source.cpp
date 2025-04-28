#include <algorithm>
#include <string_view>
#include <iostream>
#include "UI.h"
#include <fstream>
#include <chrono>
#include <array>
#include <ranges>
#include <thread>

template <class T>
void logError(const T& e)
{
    std::cerr << e.what() << std::endl;
    // Write error to log file
    std::ofstream logFile("error.log", std::ios::app);
    logFile << std::chrono::system_clock::now() << "#\t" << e.what() << std::endl;
    //Add a small delay to avoid spamming the network
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main(int argc, char *argv[])
{
    bool firstRun = true;
reset:
    //When resetting, add a delay to avoid spamming the network
    if (!firstRun)
        std::this_thread::sleep_for(std::chrono::seconds(5));
    firstRun = false;

    try
    {
        //Remove log file if one exists
        std::remove("error.log");
        
        constexpr std::string_view clickArgs[] = {"click", "c", "--c", "--click", "-c", "-click"};
        bool canClick = std::any_of(argv, argv + argc, [&clickArgs](const char *arg)
                                { return std::ranges::find(clickArgs, arg) != std::end(clickArgs); });

        UI::renderToUI(canClick);
    }
    catch (const std::exception &e)
    {
        logError(e);
        goto reset;
    }
    catch (...)
    {
        logError(std::runtime_error("Unknown error"));
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