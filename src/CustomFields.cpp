#include "CustomFields.h"
#include "Request.h"
#include "Urls.h"


namespace fields
{
	//As defined in the Trello board
	//Note that the order of the strings must match the order of the enum
	constexpr std::string_view priorityStrings[] = { "Hotfix", "Regular", "Low", "Very Low" };
	constexpr std::string_view seenStrings[] = { "Not Seen", "Seen", "In Progress", "Blocked", "Needs Additional Info", "Awaiting Response", "Actioned", "Deferred" };

	customFields getCustomFields(const requestHandler& api, std::string_view boardId)
	{
		auto fieldResponse = api.get(urls::getCustomFields(boardId));

		customFields fields;

		for (const auto& field : fieldResponse)
		{
			auto typeString = field["type"].get<std::string>();
			constexpr auto types = std::array{ "list", "date", "number", "checkbox", "text" };
			auto match = std::ranges::find(types, typeString);
			if (match == types.end())
				continue;

			dynamicField dField;
			dField.type = static_cast<fieldType>(std::distance(types.begin(), match));
			dField.fieldId = field["id"].get<std::string>();
			if (dField.type == fieldType::list)
			{
				for (const auto& option : field["options"])
				{
					auto id = option["id"].get<std::string>();
					auto name = option["value"]["text"].get<std::string>();
					dField.keys[id] = name;
				}
			}
			else
			{
				dField.isSingleValue = true;
			}
			fields[field["name"].get<std::string>()] = dField;
		}

		return fields;
	}
}