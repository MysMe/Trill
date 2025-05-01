#pragma once
#include <string>
#include "CustomFields.h"

namespace api
{
    struct member
    {
		std::string fullName;
		//Potential future prospect of having profile pictures
        std::string profilePicturePath;
    };

	//General keys for all API requests
    struct data
    {
        std::string boardId;
		fields::customFields customFields;
        keyValueLookup<std::string, std::string> lists;
        keyValueLookup<std::string, member> members;
    };

    struct card
    {
        std::string id;
        std::string name;
        std::string url;
        std::chrono::system_clock::time_point lastUpdate;

        keyValueLookup<std::string, std::string> customFields;
		std::vector<const member*> members;
    };

    data getApiData(const requestHandler& api);

    std::vector<card> getCardsInList(const requestHandler& api, const api::data& data, const std::string& listId);
}