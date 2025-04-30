#pragma once
#include <string_view>
#include "Keys.h"
#include <nlohmann/json.hpp>

class requestHandler
{
    keys accessKeys;
public:
    requestHandler(keys accessKeys) : accessKeys(std::move(accessKeys)) {}

    nlohmann::json get(std::string_view url) const;

    bool isFilteredList(std::string_view listName) const
    {
		return accessKeys.listFilter.empty() || std::ranges::contains(accessKeys.listFilter, listName);
	}
};