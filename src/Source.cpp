#include <algorithm>
#include <string_view>
#include <iostream>
#include "Console.h"
#include "UI.h"
#include <fstream>
#include <chrono>



int main(int argc, char *argv[])
{
    try
    {
        constexpr std::string_view probeArgs[] = {"probe", "p", "--p", "--probe", "-p", "-probe"};
        bool probe = std::any_of(argv, argv + argc, [&probeArgs](const char *arg)
                                 { return std::ranges::find(probeArgs, arg) != std::end(probeArgs); });

        if (probe)
        {
            console::renderToConsole();
        }
        else
        {
            UI::renderToUI();
        }
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
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return main(__argc, __argv);
}
#endif