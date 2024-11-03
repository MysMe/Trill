#include "Console.h"
#include <iostream>
#include <osmanip/manipulators/colsty.hpp>
#include "KeyValueLookup.h"
#include "Issues.h"
#include "Resolutions.h"
#include <format>
#include <chrono>
#include "ProjectDir.h"
#include "APIData.h"

namespace console
{
    void renderToConsole()
    {
        std::cout << osm::feat(osm::sty, "bold") << "Probing Trello..." << osm::feat(osm::rst, "all") << std::endl;
        const auto api = requestHandler(keys::loadKeys(constants::project_path / "keys/"));

        std::cout << "Getting API Data..." << std::endl;
        const auto data = api::getApiData(api);

        std::cout << "Getting Tickets..." << std::endl;
        const auto issues = issues::getCurrentIssues(api, data);
        const auto resolutions = resolutions::getTicketsResolvedToday(api, data);

		size_t hotfix = issues.getOr(fields::priority::hotfix, {}).count;
		size_t regular = issues.getOr(fields::priority::regular, {}).count;
		size_t low = issues.getOr(fields::priority::low, {}).count;
		size_t veryLow = issues.getOr(fields::priority::veryLow, {}).count;

        size_t total = hotfix + regular + low + veryLow;

        auto decimals = [](size_t num)
        {
            return num == 0 ? 1 : static_cast<size_t>(std::log10(num) + 1);
        };
        auto leftWidths = std::array<size_t, 4>{decimals(hotfix), decimals(regular), decimals(total), 4};
        auto rightWidths = std::array<size_t, 4>{decimals(low), decimals(veryLow), decimals(resolutions), 4};

        auto leftWidth = *std::max_element(leftWidths.begin(), leftWidths.end());
        auto rightWidth = *std::max_element(rightWidths.begin(), rightWidths.end());

        auto lastUpdate = std::format("{:%F %T}",
                                      // Remove fractional seconds
                                      std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));

        auto rst = osm::feat(osm::rst, "all");
        auto red = hotfix ? osm::feat(osm::col, "red") : "";
        auto yellow = regular ? osm::feat(osm::col, "yellow") : "";
        auto blue = low ? osm::feat(osm::col, "blue") : "";
        auto cyan = veryLow ? osm::feat(osm::col, "cyan") : "";
        auto green = resolutions ? osm::feat(osm::col, "green") : "";

        std::cout << std::format("| {}Hotfix{} | {:{}} | {}Reg.{}   | {:{}} |\n",
                                 red, rst, hotfix, leftWidth, yellow, rst, regular, leftWidth);
        std::cout << std::format("| {}Low{}    | {:{}} | {}V. Low{} | {:{}} |\n",
                                 blue, rst, low, leftWidth, cyan, rst, veryLow, leftWidth);
        std::cout << std::format("| Total  | {:{}} | {}Rsd.{}   | {:{}} |\n",
                                 total, leftWidth, green, rst, resolutions, rightWidth);
        auto totalWidth = leftWidth + rightWidth + 12;
        std::cout << std::format("| Run At | {:^{}} |\n", lastUpdate, totalWidth);
    }
}