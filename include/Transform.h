#pragma once
#include <string>
#include "APIData.h"

namespace UI
{

	struct transform
	{
		enum class operation
		{
			none = 0,
			equal = 1,
			notEqual = 2,
			less = 3,
			greater = 4,
			lessEqual = 5,
			greaterEqual = 6,
			startsWith = 7,
			endsWith = 8,
			contains = 9,
		};
		std::string key;
		std::string value;
		operation op = operation::none;

		bool operator()(const api::card* card) const
		{
			const std::string* content = card->customFields.tryGet(key);
			switch (op)
			{
			default:
				return false;
			case operation::equal:
				return content && *content == value;
			case operation::notEqual:
				return content && *content != value;
			case operation::less:
				return content && *content < value;
			case operation::greater:
				return content && *content > value;
			case operation::lessEqual:
				return content && *content <= value;
			case operation::greaterEqual:
				return content && *content >= value;
			case operation::startsWith:
				return content && content->starts_with(value);
			case operation::endsWith:
				return content && content->ends_with(value);
			case operation::contains:
				return content && content->find(value) != std::string::npos;
			}
		}

		static std::optional<transform> parse(std::string_view content)
		{
			static constexpr std::array<std::string_view, 9> transforms = { "==", "!=", "<", ">", "<=", ">=", "/%", "%/", "%%" };

			//Find the first transform that matches the string
			auto it = std::ranges::find_if(transforms, [content](const auto& t) { return content.find(t) != std::string_view::npos; });
			if (it == transforms.end())
				return std::nullopt;

			transform result;
			result.key = content.substr(0, content.find(*it));
			result.value = content.substr(result.key.size() + it->size());
			result.op = static_cast<operation>(std::distance(transforms.begin(), it) + 1);

			if (result.key.empty() || result.value.empty())
				return std::nullopt;

			return result;
		}
	};

}