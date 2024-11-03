#include "APIData.h"
#include "Urls.h"

namespace api
{

    data getApiData(const requestHandler &api)
    {
        auto boards = api.get(urls::getAllBoards);

        auto boardIt = std::find_if(boards.begin(), boards.end(), [](const auto &board)
                                    { return board["name"] == "Troncbox Development Support"; });
        if (boardIt == boards.end())
        {
            throw std::runtime_error("Board not found");
        }
        auto boardId = boardIt->at("id").get<std::string>();

        auto lists = api.get(urls::getBoardLists(boardId));

        auto issueListIt = std::find_if(lists.begin(), lists.end(), [](const auto &list)
                                        { return list["name"] == "Bugs And Queries"; });
        if (issueListIt == lists.end())
        {
            throw std::runtime_error("Bugs list not found");
        }
        auto issueListId = issueListIt->at("id").get<std::string>();

        auto resolutionListIt = std::find_if(lists.begin(), lists.end(), [](const auto &list)
                                             { return list["name"] == "Resolved"; });
        if (resolutionListIt == lists.end())
        {
            throw std::runtime_error("Resolved list not found");
        }
        auto resolutionListId = resolutionListIt->at("id").get<std::string>();

		auto customFields = fields::getCustomFields(api, boardId);

        return {boardId, issueListId, resolutionListId, customFields};
    }
}