#include "APIData.h"
#include "Urls.h"

namespace api
{

    data getApiData(const requestHandler &api)
    {
		//Get all boards for the user
        auto boards = api.get(urls::getAllBoards);

		//Find the board with the name "Troncbox Development Support" - meaning this will only work with my specific board setup
        auto boardIt = std::find_if(boards.begin(), boards.end(), [](const auto &board)
                                    { return board["name"] == "Troncbox Development Support"; });
        if (boardIt == boards.end())
        {
            throw std::runtime_error("Board not found");
        }
        auto boardId = boardIt->at("id").get<std::string>();

		//Get all lists for the board
        auto lists = api.get(urls::getBoardLists(boardId));
		keyValueLookup<std::string, std::string> namedLists;

		for (auto list : lists)
		{
			namedLists[list["name"].get<std::string>()] = list["id"].get<std::string>();
		}

		//Get the custom fields for the board, this automatically infers field keys
		auto customFields = fields::getCustomFields(api, boardId);

		auto memberResponse = api.get(urls::getBoardMembers(boardId));

		keyValueLookup<std::string, member> members;
		for (auto member : memberResponse)
		{
			auto memberId = member["id"].get<std::string>();
			auto memberName = member["fullName"].get<std::string>();
			members[memberId] = { memberName };
		}

        return {boardId, customFields, namedLists, members};
    }

	std::vector<card> getCardsInList(const requestHandler& api, const api::data& data, const std::string& listId)
	{
		auto cards = api.get(urls::getCardsInList(listId));

		std::vector<card> result;

		for (auto& current : cards)
		{
			card value;

			value.id = current["id"].get<std::string>();
			value.name = current["name"].get<std::string>();
			value.url = current["url"].get<std::string>();

			for (auto memberId : current["idMembers"])
			{
				auto member = data.members.tryGet(memberId.get<std::string>());
				if (member)
				{
					value.members.push_back(member);
				}
			}

			//Add all dynamic fields
			for (const auto& [name, fieldContent] : data.customFields)
			{
				auto id = fieldContent.fieldId;
				auto field = std::find_if(current["customFieldItems"].begin(), current["customFieldItems"].end(), [&id](const auto& field)
					{ return field["idCustomField"] == id; });

				//If the field is not found, skip it
				if (field == current["customFieldItems"].end())
					continue;

				if (fieldContent.type == fields::fieldType::date)
				{
					auto date = (*field)["value"]["date"].get<std::string>();
					value.customFields[name] = date;
				}
				else if (fieldContent.type == fields::fieldType::number)
				{
					auto num = (*field)["value"]["number"].get<std::string>();
					value.customFields[name] = num;
				}
				else if (fieldContent.type == fields::fieldType::list)
				{
					auto valueId = (*field)["idValue"].get<std::string>();
					//Try and find the matching value, if one is present
					auto match = fieldContent.keys.tryGet(valueId);
					if (!match)
						continue;
					//Add both key and value to the card
					value.customFields[name] = *match;
				}
				else if (fieldContent.type == fields::fieldType::checkbox)
				{
					auto isChecked = (*field)["value"]["checked"].get<std::string>();
					value.customFields[name] = isChecked;
				}
				else if (fieldContent.type == fields::fieldType::text)
				{
					auto text = (*field)["value"]["text"].get<std::string>();
					value.customFields[name] = text;
				}

			}


			auto lastActivityJson = current["dateLastActivity"];
			if (!lastActivityJson.is_null())
			{
				//Parse from string to timepoint
				std::tm tm = {};
				std::istringstream ss(lastActivityJson.get<std::string>());
				if (ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S"))
				{
					auto lastActivity = std::chrono::system_clock::from_time_t(std::mktime(&tm));
					value.lastActivity = lastActivityJson.get<std::string>();
					value.lastUpdate = lastActivity;
				}
			}

			result.emplace_back(std::move(value));
		}
		return result;
	}
}