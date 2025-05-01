#pragma once
#include <RmlUi/Core.h>
#include "Transform.h"

namespace query
{
	struct queryModel
	{
		std::vector<api::card*> filtered;

		queryModel() = default;
		queryModel(std::vector<api::card>& cards) : filtered(cards | std::views::transform([](auto& card) { return &card; }) | std::ranges::to<std::vector<api::card*>>()) {}
		queryModel(const std::vector<api::card*>& cards) : filtered(cards) {}

		void match(const UI::transform& t)
		{
			filtered = filtered | std::views::filter(t) | std::ranges::to<std::vector<api::card*>>();
		}

		void matchAny(std::span<const UI::transform> transforms)
		{
			filtered = filtered | std::views::filter([transforms](const auto& card)
				{
					return std::ranges::any_of(transforms, [&card](const auto& t) { return t(card); });
				}) | std::ranges::to<std::vector<api::card*>>();
		}
	};

	void bind(Rml::DataModelConstructor& constructor);
	void setCards(keyValueLookup<std::string, std::vector<api::card>>&& cards);
}