#pragma once
#include <string_view>
#include <string>
#include <format>

namespace urls
{
    constexpr std::string_view getAllBoards = "https://api.trello.com/1/members/me/boards?fields=name";
    inline std::string getBoard(std::string_view boardId)
    {
        return std::format("https://api.trello.com/1/boards/{}?fields=name", boardId);
    }

    inline std::string getBoardLists(std::string_view boardId)
    {
        return std::format("https://api.trello.com/1/boards/{}/lists?fields=name", boardId);
    }

    inline std::string getCardsInList(std::string_view listId)
    {
        return std::format("https://api.trello.com/1/lists/{}/cards?fields=name,dateLastActivity&customFieldItems=true", listId);
    }

    inline std::string getCustomFields(std::string_view boardId)
    {
        return std::format("https://api.trello.com/1/boards/{}/customFields", boardId);
    }
}