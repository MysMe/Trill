#include <algorithm>
#include <string_view>
#include <iostream>
#include "Console.h"
#include "UI.h"



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
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
    }
    return 0;
}