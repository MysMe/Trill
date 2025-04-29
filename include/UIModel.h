#pragma once
#include <string>
#include <cstdint>
#include <RmlUi/Core.h>
#include <ranges>
#include <list>

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

		static std::optional<transform> fromString(std::string_view content)
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
			
			return result;
		}
	};

	struct queryModel
	{
		std::vector<api::card*> filtered;

		queryModel() = default;
		queryModel(std::vector<api::card>& cards) : filtered(cards | std::views::transform([](auto& card) { return &card; }) | std::ranges::to<std::vector<api::card*>>()) {}
		queryModel(const std::vector<api::card*>& cards) : filtered(cards) {}

		static inline std::vector<queryModel> queryCache;
		static inline keyValueLookup<std::string, std::vector<api::card*>> queryStore;

		static void initCache(queryModel&& model)
		{
			queryCache.clear();
			queryCache.emplace_back(std::move(model));
		}

		void match(const transform& t)
		{
			filtered = filtered | std::views::filter(t) | std::ranges::to<std::vector<api::card*>>();
		}

		void matchAny(std::span<const transform> transforms)
		{
			filtered = filtered | std::views::filter([transforms](const auto& card)
				{
					return std::ranges::any_of(transforms, [&card](const auto& t) { return t(card); });
				}) | std::ranges::to<std::vector<api::card*>>();
		}
		
		static Rml::Variant where(const Rml::VariantList& args)
		{
			if (queryCache.empty())
				return Rml::Variant{};
			//Intentional copy
			auto list = queryCache.back();
			for (int i = 1; i < args.size(); i++)
			{
				transform t = transform::fromString(args[i].Get<std::string>()).value();
				list.match(t);
			}
			queryCache.emplace_back(std::move(list));
			return Rml::Variant{ "Unresolved Query" };
		}

		static Rml::Variant whereAny(const Rml::VariantList& args)
		{
			if (queryCache.empty())
				return Rml::Variant{};
			//Intentional copy
			auto list = queryCache.back();
			std::vector<transform> transforms;
			for (int i = 1; i < args.size(); i++)
			{
				transforms.emplace_back(transform::fromString(args[i].Get<std::string>()).value());
			}
			list.matchAny(transforms);
			queryCache.emplace_back(std::move(list));
			return Rml::Variant{ "Unresolved Query" };
		}

		static Rml::Variant today(const Rml::VariantList& args)
		{
			if (queryCache.empty())
				return Rml::Variant{};
			auto now = std::chrono::system_clock::now();
			auto today = std::format("{:%Y-%m-%d}", now);

			//Intentional copy
			auto list = queryCache.back();
			for (int i = 1; i < args.size(); i++)
			{
				transform t;
				t.key = args[i].Get<std::string>();
				t.op = transform::operation::startsWith;
				t.value = today;
				list.match(t);
			}
			queryCache.emplace_back(std::move(list));
			return Rml::Variant{ "Unresolved Query" };
		}

		static Rml::Variant transformCount(const Rml::VariantList&)
		{
			if (queryCache.empty())
				return Rml::Variant{};
			auto& list = queryCache.back();
			int count = static_cast<int>(list.filtered.size());
			return Rml::Variant{ count };
		}

		static Rml::Variant transformMostRecent(const Rml::VariantList&)
		{
			if (queryCache.empty())
				return Rml::Variant{};
			auto& list = queryCache.back();
			if (list.filtered.empty())
			{
				return Rml::Variant{};
			}
			auto mostRecent = std::ranges::max_element(list.filtered, [](const auto& a, const auto& b)
				{
					return a->lastUpdate < b->lastUpdate;
				});
			auto lastUpdate = (*mostRecent)->lastUpdate;
			//Format the time to a string
			auto time_t = std::chrono::system_clock::to_time_t(lastUpdate);
			std::tm tm = *std::localtime(&time_t);
			std::string result(20, '\0');
			std::strftime(result.data(), result.size(), "%F %T", &tm);
			return Rml::Variant{ result };
		}


		static Rml::Variant saveQuery(const Rml::VariantList& args)
		{
			if (args.size() != 2 || queryCache.empty())
				return Rml::Variant{};
			auto name = args[1].Get<std::string>();
			auto& list = queryCache.back();
			queryStore[name] = list.filtered;
			//Note that we don't want to provide output after saving the query, as we can therefore save from content
			return Rml::Variant{};
		}

		static Rml::Variant loadQuery(const Rml::VariantList& args)
		{
			if (args.size() != 1)
				return Rml::Variant{};
			auto name = args[0].Get<std::string>();
			auto it = queryStore.tryGet(name);
			if (!it)
				return Rml::Variant{};
			initCache(queryModel(*it));
			return Rml::Variant{ "Unresolved Query" };
		}

		static void bind(Rml::DataModelConstructor& constructor)
		{
			auto handle = constructor.RegisterStruct<queryModel>();

			constructor.RegisterTransformFunc("where", where);
			constructor.RegisterTransformFunc("whereAny", whereAny);
			constructor.RegisterTransformFunc("count", transformCount);
			constructor.RegisterTransformFunc("mostRecent", transformMostRecent);
			constructor.RegisterTransformFunc("today", today);
			constructor.RegisterTransformFunc("as", saveQuery);
			constructor.RegisterTransformFunc("from", loadQuery);
		}
	};

	struct model
	{
		static std::string timeToString(const std::optional<std::chrono::system_clock::time_point> &time)
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

		model() = default;
		//The UI holds a reference to the model, so we need to make sure it's not copied or moved.
		model(const model&) = delete;
		model& operator=(const model&) = delete;
		model(model&&) = delete;
		model& operator=(model&&) = delete;
		~model() = default;

		keyValueLookup<std::string, std::vector<api::card>> cardLists;

		void set(keyValueLookup<std::string, std::vector<api::card>>&& cards)
		{
			cardLists = std::move(cards);
			queryModel::queryStore.clear();
			syncLast = timeToString(std::chrono::system_clock::now());
		}

		void bind(Rml::Context* context, Rml::DataModelHandle& model, const api::data& api)
		{
			Rml::DataModelConstructor constructor = context->CreateDataModel("model");
			if (!constructor)
				return;

			constructor.Bind("syncLast", &syncLast);

			constructor.RegisterStruct<api::card>();
			constructor.RegisterArray<std::vector<api::card>>();

			queryModel::bind(constructor);
			static std::string emptyQuery{ "Unresolved Query" };
			constructor.Bind("query", &emptyQuery);

			for (const auto& list : api.lists)
			{
				auto accessor = [list, lists = &cardLists](const Rml::VariantList&)
					{
						auto value = lists->tryGet(list.first);
						if (!value)
							return Rml::Variant{};
						queryModel::initCache(*value);
						return Rml::Variant{ "Unresolved Query" };
					};

				std::string name = list.first;
				std::ranges::replace(name, ' ', '_');
				constructor.RegisterTransformFunc(name, accessor);
			}
			model = constructor.GetModelHandle();
		}
	};
}