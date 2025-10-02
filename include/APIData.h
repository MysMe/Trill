#pragma once
#include <string>
#include "CustomFields.h"
#include "Config.h"

namespace api
{
    struct member
    {
        member() = default;
        member(std::string name) : fullName(std::move(name)) {}
        member(std::string name, std::string profilePicturePath) : fullName(std::move(name)), profilePicturePath(std::move(profilePicturePath)) {}

		std::string fullName;
		//Potential future prospect of having profile pictures
        std::string profilePicturePath;
    };

	//General keys for all API requests
    struct data
    {
        std::string boardId;
		fields::customFields customFields;
		fields::labels labels;
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
		std::vector<std::string> labels;
		std::vector<const member*> members;
    };

    data getApiData(const requestHandler& api, const config::config& conf);

    std::vector<card> getCardsInList(const requestHandler& api, const api::data& data, const std::pair<std::string, std::string>& list);
}