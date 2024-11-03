#pragma once
#include <string>
#include "Issues.h"
#include "Resolutions.h"

namespace api
{
    struct data
    {
        std::string boardId;
        std::string issueListId;
        std::string resolvedListId;
		fields::customFields customFields;
    };

    data getApiData(const requestHandler& api);
}