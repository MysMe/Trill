#pragma once
#include <RmlUi/Core.h>
#include "Transform.h"

namespace query
{
	struct queryModel
	{
		std::vector<api::card*> filtered;

		static std::vector<api::card*> convert(auto range)
		{
			std::vector<api::card*> result;
			for (auto card : range)
				result.push_back(card);
			return result;
		}

		queryModel() = default;
		queryModel(std::vector<api::card>& cards) : filtered(convert(cards | std::views::transform([](auto& card) { return &card; }))) {}
		queryModel(const std::vector<api::card*>& cards) : filtered(cards) {}

		void match(const UI::transform& t)
		{
			filtered = convert(filtered | std::views::filter(t));
		}

		void matchAny(std::span<const UI::transform> transforms)
		{
			filtered = convert(filtered | std::views::filter([transforms](const auto& card)
				{
					return std::ranges::any_of(transforms, [&card](const auto& t) { return t(card); });
				}));
		}

		template <class Fn>
		void filter(Fn&& fn)
		{
			filtered = convert(filtered | std::views::filter(std::forward<Fn>(fn)));
		}
	};

	void bind(Rml::DataModelConstructor& constructor);
	void setCards(keyValueLookup<std::string, std::vector<api::card>>&& cards);
	void preRender(Rml::ElementDocument& document);
}