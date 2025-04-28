#pragma once
#include <string>
#include "KeyValueLookup.h"
#include "Request.h"

namespace fields
{

	//As taken from Trello
	enum class priority
	{
		hotfix,
		regular,
		low,
		veryLow
	};

	//As taken from Trello
	enum class seenState
	{
		notSeen,
		seen,
		inProgress,
		blocked,
		needsAdditionalInfo,
		awaitingResponse,
		actioned,
		deferred
	};

	//Field and key identifiers for the Priority field
	struct priorityKeys
	{
		std::string fieldId;
		keyValueLookup<std::string, priority> keys;
	};

	//Field identifier for the Resolution field
	struct resolutionKeys
	{
		std::string fieldId;
	};

	//Field and key identifiers for the Seen State field
	struct seenKeys
	{
		std::string fieldId;
		keyValueLookup<seenState, std::string> keys;
	};

	enum class fieldType
	{
		list,
		date,
		number,
		checkbox,
		text
	};

	struct dynamicField
	{
		std::string fieldId;
		keyValueLookup<std::string, std::string> keys;
		bool isSingleValue = false;
		fieldType type = fieldType::list;
	};

	using customFields = keyValueLookup<std::string, dynamicField>;

	customFields getCustomFields(const requestHandler& api, std::string_view boardId);
}