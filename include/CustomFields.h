#pragma once
#include <string>
#include "KeyValueLookup.h"
#include "Request.h"

namespace fields
{
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

	using labels = keyValueLookup<std::string, std::string>;

	customFields getCustomFields(const requestHandler& api, std::string_view boardId);
	labels getLabels(const requestHandler& api, std::string_view boardId);
}