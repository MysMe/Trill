#pragma once
#include <string>
#include "KeyValueLookup.h"
#include "Request.h"

namespace fields
{

	enum class priority
	{
		hotfix,
		regular,
		low,
		veryLow
	};

	enum class seenState
	{
		notSeen,
		seen,
		inProgress,
		onHold,
		actioned,
		deferred
	};

	struct priorityKeys
	{
		std::string fieldId;
		keyValueLookup<std::string, priority> keys;
	};

	struct resolutionKeys
	{
		std::string fieldId;
	};

	struct seenKeys
	{
		std::string fieldId;
		keyValueLookup<seenState, std::string> keys;
	};

	struct customFields
	{
		priorityKeys pKeys;
		resolutionKeys rKeys;
		seenKeys sKeys;
	};


	customFields getCustomFields(const requestHandler& api, std::string_view boardId);
}