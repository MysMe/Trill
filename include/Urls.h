#pragma once
#include <string_view>
#include <string>
#include <format>

namespace urls
{
    //Gets all boards for the user
    constexpr std::string_view getAllBoards = "https://api.trello.com/1/members/me/boards?fields=name";

	//Gets all lists for the board with the given ID
    inline std::string getBoardLists(std::string_view boardId)
    {
        return std::format("https://api.trello.com/1/boards/{}/lists?fields=name", boardId);
    }

	//Gets all cards for the list with the given ID
    inline std::string getCardsInList(std::string_view listId)
    {
        return std::format("https://api.trello.com/1/lists/{}/cards?fields=name,dateLastActivity,url,idMembers&customFieldItems=true", listId);
    }

	//Gets the custom fields for the board with the given ID
    inline std::string getCustomFields(std::string_view boardId)
    {
        return std::format("https://api.trello.com/1/boards/{}/customFields", boardId);
    }

	//Gets all members for the board with the given ID
    inline std::string getBoardMembers(std::string_view boardId)
    {
        return std::format("https://api.trello.com/1/boards/{}/members?fields=fullName,id", boardId);
	}
}