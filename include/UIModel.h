#pragma once
#include <string>
#include <cstdint>
#include <RmlUi/Core.h>
#include <ranges>
#include <list>
#include "Query.h"

namespace UI
{

	struct model
	{
		static std::string timeToString(const std::optional<std::chrono::system_clock::time_point>& time)
		{
			if (!time.has_value())
				return "";
			auto time_t = std::chrono::system_clock::to_time_t(time.value());
			std::tm tm = *std::localtime(&time_t);
			std::string result(20, '\0');
			std::strftime(result.data(), result.size(), "%F %T", &tm);
			return result;
		};

		std::string syncLast;
		std::vector<std::string> cardListNames;

		model() = default;
		//The UI holds a reference to the model, so we need to make sure it's not copied or moved.
		model(const model&) = delete;
		model& operator=(const model&) = delete;
		model(model&&) = delete;
		model& operator=(model&&) = delete;
		~model() = default;


		void set(keyValueLookup<std::string, std::vector<api::card>>&& cards)
		{
			query::setCards(std::move(cards));
			syncLast = timeToString(std::chrono::system_clock::now());
		}

		void bind(Rml::Context* context, Rml::DataModelHandle& model, const api::data& api)
		{
			Rml::DataModelConstructor constructor = context->CreateDataModel("model");
			if (!constructor)
				return;

			constructor.Bind("syncLast", &syncLast);

			query::bind(constructor);

			cardListNames.clear();
			cardListNames.reserve(api.lists.size());

			for (const auto& list : api.lists)
			{
				std::string name = list.first;
				std::ranges::replace(name, ' ', '_');
				//Remove any non-alphanumeric characters
				name.erase(std::remove_if(name.begin(), name.end(), [](char c) { return !std::isalnum(static_cast<unsigned char>(c)) && c != '_'; }), name.end());

				if (name.empty())
					continue;

				cardListNames.push_back(list.first);

				constructor.Bind(name, &cardListNames.back());
			}
			model = constructor.GetModelHandle();
		}
	};
}