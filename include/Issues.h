#pragma once
#include <string>
#include <string_view>
#include "KeyValueLookup.h"
#include "Request.h"
#include "CustomFields.h"

namespace api
{
    struct data;
}

namespace issues
{
	//Summary of issues for a single priority
    struct issueSummary
    {
        size_t count;
        size_t unseen;
        std::optional<std::chrono::system_clock::time_point> lastUpdate;
    };

    keyValueLookup<fields::priority, issueSummary> getCurrentIssues(const requestHandler &api, const api::data &data);
}