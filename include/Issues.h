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
    struct issueSummary
    {
        size_t count;
        size_t unseen;
        std::chrono::system_clock::time_point lastUpdate = std::chrono::system_clock::time_point::min();
    };

    keyValueLookup<fields::priority, issueSummary> getCurrentIssues(const requestHandler &api, const api::data &data);
}