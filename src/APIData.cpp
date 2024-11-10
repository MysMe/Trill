#include "APIData.h"
#include "Urls.h"

namespace api
{

    data getApiData(const requestHandler &api)
    {
		//Get all boards for the user
        auto boards = api.get(urls::getAllBoards);

		//Find the board with the name "Troncbox Development Support" - meaning this will only work with my specific board setup
        auto boardIt = std::find_if(boards.begin(), boards.end(), [](const auto &board)
                                    { return board["name"] == "Troncbox Development Support"; });
        if (boardIt == boards.end())
        {
            throw std::runtime_error("Board not found");
        }
        auto boardId = boardIt->at("id").get<std::string>();

		//Get all lists for the board
        auto lists = api.get(urls::getBoardLists(boardId));

		//Find the lists with the names "Bugs And Queries" and "Resolved", as above this will only work with my specific board setup
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

		//Get the custom fields for the board, this automatically infers field keys
		auto customFields = fields::getCustomFields(api, boardId);

        return {boardId, issueListId, resolutionListId, customFields};
    }
}