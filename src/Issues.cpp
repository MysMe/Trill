#include "Issues.h"
#include "Urls.h"
#include "APIData.h"
#include <chrono>
#include <sstream>

namespace issues
{
    keyValueLookup<fields::priority, issueSummary> getCurrentIssues(const requestHandler &api, const api::data& data)
    {
        auto cards = api.get(urls::getCardsInList(data.issueListId));

		keyValueLookup<fields::priority, issueSummary> currentIssues;

		auto seenKeyId = data.customFields.sKeys.keys.tryGet(fields::seenState::notSeen);

		for (auto& card : cards)
		{
			auto customFields = card["customFieldItems"];
			auto priority = std::find_if(customFields.begin(), customFields.end(), [&data](const auto& field)
				{ return field["idCustomField"].template get<std::string>() == data.customFields.pKeys.fieldId; });

			if (priority == customFields.end() || !priority->contains("idValue") || !(*priority)["isValue"].is_null())
			{
				continue;
			}
            auto priorityValue = (*priority)["idValue"];

            auto type = data.customFields.pKeys.keys.tryGet(priorityValue.get<std::string>());
            if (type)
            {
                ++currentIssues[*type].count;

				if (seenKeyId)
				{
					auto seenState = std::find_if(customFields.begin(), customFields.end(), [&data](const auto& field)
						{ return field["idCustomField"].template get<std::string>() == data.customFields.sKeys.fieldId; });


					if (seenState != customFields.end() && seenState->contains("idValue") && !(*seenState)["idValue"].is_null() && (*seenState)["idValue"] == *seenKeyId)
					{
						++currentIssues[*type].unseen;
					}
				}

				auto lastActivityJson = card["dateLastActivity"];
				if (!lastActivityJson.is_null())
				{
					//parse from string to timepoint
					auto stream = std::istringstream(lastActivityJson.get<std::string>());
					std::chrono::time_point<std::chrono::system_clock> lastActivity;
					if (std::chrono::from_stream(stream, "%Y-%m-%dT%H:%M:%S", lastActivity))
					{
						if (lastActivity > currentIssues[*type].lastUpdate)
						{
							currentIssues[*type].lastUpdate = lastActivity;
						}
					}
				}
            }
        }

        return currentIssues;
    }
}