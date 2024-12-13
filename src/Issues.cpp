#include "Issues.h"
#include "Urls.h"
#include "APIData.h"
#include <chrono>
#include <sstream>
#include <algorithm>

namespace issues
{
	keyValueLookup<fields::priority, issueSummary> getCurrentIssues(const requestHandler &api, const api::data &data)
	{
		auto cards = api.get(urls::getCardsInList(data.issueListId));

		keyValueLookup<fields::priority, issueSummary> currentIssues;

		auto seenKeyId = data.customFields.sKeys.keys.tryGet(fields::seenState::notSeen);

		std::array ignoreKeyIds
		{
			data.customFields.sKeys.keys.tryGet(fields::seenState::blocked),
			data.customFields.sKeys.keys.tryGet(fields::seenState::needsAdditionalInfo),
			data.customFields.sKeys.keys.tryGet(fields::seenState::awaitingResponse),
			data.customFields.sKeys.keys.tryGet(fields::seenState::deferred),
		};

		for (auto &card : cards)
		{
			auto customFields = card["customFieldItems"];
			auto priority = std::find_if(customFields.begin(), customFields.end(), [&data](const auto &field)
										 { return field["idCustomField"].template get<std::string>() == data.customFields.pKeys.fieldId; });

			if (priority == customFields.end() || !priority->contains("idValue") || !(*priority)["isValue"].is_null())
			{
				continue;
			}
			auto priorityValue = (*priority)["idValue"];

			auto type = data.customFields.pKeys.keys.tryGet(priorityValue.get<std::string>());
			if (type)
			{
				auto seenState = std::find_if(customFields.begin(), customFields.end(), [&data](const auto &field)
												{ return field["idCustomField"].template get<std::string>() == data.customFields.sKeys.fieldId; });

				if (seenState != customFields.end() && seenState->contains("idValue") && !(*seenState)["idValue"].is_null())
				{
					//If this is "not seen" mark it so
					if (seenKeyId && (*seenState)["idValue"] == *seenKeyId)
					{
						++currentIssues[*type].unseen;
					}
					//Otherwise if this is in the list of types to ignore, skip it
					if (std::any_of(ignoreKeyIds.begin(), ignoreKeyIds.end(), [&seenState](const auto &id)
									 { return id && (*seenState)["idValue"] == *id; }))
					{
						++currentIssues[*type].blocked;
					}
					else
					{
						//Only count unblocked tickets
						++currentIssues[*type].count;
					}
				}


				auto lastActivityJson = card["dateLastActivity"];
				if (!lastActivityJson.is_null())
				{
					//Parse from string to timepoint
					std::tm tm = {};
					std::istringstream ss(lastActivityJson.get<std::string>());
					if (ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S"))
					{
						auto lastActivity = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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