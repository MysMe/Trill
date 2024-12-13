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
            if (field["name"] == "Priority")
            {
                priorityKeys keys;
				keys.fieldId = field["id"].get<std::string>();
				for (const auto& option : field["options"])
				{
					auto id = option["id"].get<std::string>();
					auto name = option["value"]["text"].get<std::string>();
					//Relies on the order of the strings and the enum to match
					auto idx = std::find(std::begin(priorityStrings), std::end(priorityStrings), name);
					if (idx != std::end(priorityStrings))
					{
						keys.keys[id] = static_cast<priority>(std::distance(std::begin(priorityStrings), idx));
					}
				}
				fields.pKeys = keys;
            }
            if (field["name"] == "Resolved At")
            {
				fields.rKeys = { field["id"] };
            }
            if (field["name"] == "Opsyte Status")
            {
				seenKeys keys;
				keys.fieldId = field["id"].get<std::string>();
				for (const auto& option : field["options"])
				{
					auto id = option["id"].get<std::string>();
					auto name = option["value"]["text"].get<std::string>();
					//Relies on the order of the strings and the enum to match
					auto idx = std::find(std::begin(seenStrings), std::end(seenStrings), name);
					if (idx != std::end(seenStrings))
					{
						seenState state = static_cast<seenState>(std::distance(std::begin(seenStrings), idx));
						keys.keys[state] = id;
					}
				}
				fields.sKeys = keys;
            }
        }

		return fields;
    }
}