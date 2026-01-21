#include "APIData.h"
#include "Urls.h"
#include "ProjectDir.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include "ImgConv.h"

struct apiLoggingSettings
{
	static constexpr inline auto logName = "api";
};


namespace api
{

    data getApiData(const requestHandler &api, const config::config& conf)
    {
		//Get all boards for the user
        auto boards = api.get(urls::getAllBoards);

        auto boardIt = std::find_if(boards.begin(), boards.end(), [&](const auto &board)
                                    { return board["name"] == conf.boardName; });
        if (boardIt == boards.end())
        {
			logger::logFor<apiLoggingSettings>().flogLn("Board not found: {}", conf.boardName);
            throw std::runtime_error("Board not found");
        }
        auto boardId = boardIt->at("id").get<std::string>();

		//Get all lists for the board
        auto lists = api.get(urls::getBoardLists(boardId));
		keyValueLookup<std::string, std::string> namedLists;

		logger::logFor<apiLoggingSettings>().flogLn("Found {} lists for board {}", lists.size(), conf.boardName);
		for (auto list : lists)
		{
			namedLists[list["name"].get<std::string>()] = list["id"].get<std::string>();
			logger::logFor<apiLoggingSettings>().flogLn("List: {}, ID: {}", list["name"].get<std::string>(), list["id"].get<std::string>());
		}

		//Get the custom fields for the board, this automatically infers field keys
		auto customFields = fields::getCustomFields(api, boardId);
		if (logger::logFor<apiLoggingSettings>().isEnabled())
		{
			for (const auto& field : customFields)
			{
				logger::logFor<apiLoggingSettings>().flogLn("Field: {}, ID: {}", field.first, field.second.fieldId);
			}
		}

		//Get the labels for the board
		auto labels = fields::getLabels(api, boardId);
		if (logger::logFor<apiLoggingSettings>().isEnabled())
		{
			for (const auto& label : labels)
			{
				logger::logFor<apiLoggingSettings>().flogLn("Label: {}, ID: {}", label.second, label.first);
			}
		}


		auto memberResponse = api.get(urls::getBoardMembers(boardId));

		//Ensure the avatar directory exists
		if (!std::filesystem::exists(constants::avatar_dir))
		{
			std::filesystem::create_directory(constants::avatar_dir);
		}

		keyValueLookup<std::string, member> members;
		for (auto member : memberResponse)
		{
			auto memberId = member["id"].get<std::string>();
			auto memberName = member["fullName"].get<std::string>();
			members[memberId] = { memberName };

			//If they don't have an avatar, skip them
			if (member["avatarHash"].is_null())
			{
				logger::logFor<apiLoggingSettings>().flogLn("Member {} has no avatar.", memberName);
				continue;
			}

			//Check if we already have a copy of the member profile picture

			std::string tgaFilename = std::format("{}/{}.tga", constants::avatar_dir, member["avatarHash"].get<std::string>());

			if (!std::filesystem::exists(tgaFilename))
			{
				std::string pngFilename = std::format("{}/{}.png", constants::avatar_dir, member["avatarHash"].get<std::string>());
				std::string fileUrl = std::format("{}/170.png", member["avatarUrl"].get<std::string>());
				//Download the image
				curlpp::Easy request;
				request.setOpt<curlpp::options::Url>(fileUrl);

				std::ofstream response(pngFilename, std::ios::out | std::ios::binary);
				if (!response)
				{
					throw std::runtime_error("Failed to open file for writing: " + pngFilename);
				}
				request.setOpt<curlpp::options::WriteStream>(&response);
				request.perform();

				response.close();
				//Convert to TGA format
				imgConv::convertPngToTga(pngFilename, tgaFilename);

				//Delete the PNG file
				std::filesystem::remove(pngFilename);

				logger::logFor<apiLoggingSettings>().flogLn("Downloaded and converted avatar for member {} to {}", memberName, tgaFilename);
			}
			else
			{
				logger::logFor<apiLoggingSettings>().flogLn("Avatar for member {} already exists at {}", memberName, tgaFilename);
			}

			members[memberId].profilePicturePath = tgaFilename;
		}

		logger::logFor<apiLoggingSettings>().flush();

        return {boardId, customFields, labels, namedLists, members};
    }

	std::vector<card> getCardsInList(const requestHandler& api, const api::data& data, const std::pair<std::string, std::string>& list)
	{
		logger::logFor<apiLoggingSettings>().flogLn("Getting cards in list {}", list.first);
		auto cards = api.get(urls::getCardsInList(list.second));

		std::vector<card> result;

		for (auto& current : cards)
		{
			card value;

			value.id = current["id"].get<std::string>();
			value.name = current["name"].get<std::string>();
			value.url = current["url"].get<std::string>();

			logger::logFor<apiLoggingSettings>().flogLn("\tCard: {}, ID: {}", value.name, value.id);

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
					logger::logFor<apiLoggingSettings>().flogLn("\t\tField: {}, Value: {}", name, date);
					value.customFields[name] = date;
				}
				else if (fieldContent.type == fields::fieldType::number)
				{
					auto num = (*field)["value"]["number"].get<std::string>();
					logger::logFor<apiLoggingSettings>().flogLn("\t\tField: {}, Value: {}", name, num);
					value.customFields[name] = num;
				}
				else if (fieldContent.type == fields::fieldType::list)
				{
					auto valueId = (*field)["idValue"].get<std::string>();
					//Try and find the matching value, if one is present
					auto match = fieldContent.keys.tryGet(valueId);
					if (!match)
						continue;
					logger::logFor<apiLoggingSettings>().flogLn("\t\tField: {}, Value: {}", name, *match);
					//Add both key and value to the card
					value.customFields[name] = *match;
				}
				else if (fieldContent.type == fields::fieldType::checkbox)
				{
					auto isChecked = (*field)["value"]["checked"].get<std::string>();
					logger::logFor<apiLoggingSettings>().flogLn("\t\tField: {}, Value: {}", name, isChecked);
					value.customFields[name] = isChecked;
				}
				else if (fieldContent.type == fields::fieldType::text)
				{
					auto text = (*field)["value"]["text"].get<std::string>();
					logger::logFor<apiLoggingSettings>().flogLn("\t\tField: {}, Value: {}", name, text);
					value.customFields[name] = text;
				}

			}

			//Add any labels
			if (!current["idLabels"].is_null())
			{
				for (const auto& labelId : current["idLabels"])
				{
					auto label = data.labels.tryGet(labelId.get<std::string>());
					if (label)
					{
						logger::logFor<apiLoggingSettings>().flogLn("\t\tLabel: {}", *label);
						value.labels.push_back(*label);
					}
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
					value.lastUpdate = lastActivity;
					logger::logFor<apiLoggingSettings>().flogLn("\t\tLast activity: {}", std::chrono::system_clock::to_time_t(lastActivity));
				}
			}

			result.emplace_back(std::move(value));
		}
		logger::logFor<apiLoggingSettings>().flush();
		return result;
	}
}