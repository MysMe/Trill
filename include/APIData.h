#pragma once
#include <string>
#include "Issues.h"

namespace api
{
	//General keys for all API requests
    struct data
    {
        std::string boardId;
		fields::customFields customFields;
        keyValueLookup<std::string, std::string> lists;
    };

    struct card
    {
        std::string id;
        std::string name;
        std::string url;
        std::string lastActivity;
        std::chrono::system_clock::time_point lastUpdate;

        keyValueLookup<std::string, std::string> customFields;
    };

    data getApiData(const requestHandler& api);

    std::vector<card> getCardsInList(const requestHandler& api, const api::data& data, const std::string& listId);
}