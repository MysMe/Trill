#include "Query.h"
#include "APIData.h"
#include "Logging.h"

using namespace query;

struct queryLogSettings
{
	static constexpr inline std::string logName = "query";
};

using postLoadEvent = std::function<void(Rml::ElementDocument&)>;

class queryCache
{
	std::optional<queryModel> queryCache;
	keyValueLookup<std::string, std::vector<api::card*>> queryStore;
	keyValueLookup<std::string, std::vector<api::card>> cardLists;

	std::vector<postLoadEvent> preRenderEvents;

public:
	queryModel* getOrCreateQuery(const Rml::VariantList& args)
	{
		//Query already in progress, so continue that one
		if (queryCache)
			return &queryCache.value();
		//No running query and we can't start one
		if (args.empty())
		{
			logger::logFor<queryLogSettings>().logLn("Unable to start query, no args provided.");
			return nullptr;
		}
		std::string name;
		if (!args[0].GetInto(name))
		{
			logger::logFor<queryLogSettings>().logLn("Unable to start query, arg was not a string.");
			return nullptr;
		}
		auto list = cardLists.tryGet(name);
		if (!list)
		{
			logger::logFor<queryLogSettings>().flogLn("Unable to start query, list '{}' was not found.", name);
			return nullptr;
		}
		queryCache.emplace(*list);
		logger::logFor<queryLogSettings>().flog("New query on '{}': ", name);
		return &queryCache.value();
	}

	void setCards(keyValueLookup<std::string, std::vector<api::card>>&& cards)
	{
		if (cardLists.size() != 0)
		{
			logger::logFor<queryLogSettings>().addEmptyLines(2).flush();
		}
		clearCache();
		clearStore();
		cardLists = std::move(cards);
		logger::logFor<queryLogSettings>().flogLn("Registered {} card list(s).", cardLists.size());
	}

	bool queryCached() const { return queryCache.has_value(); }
	bool saveQuery(std::string_view name)
	{
		if (queryCache)
		{
			queryStore[name] = queryCache->filtered;
			return true;
		}
		return false;
	}
	std::vector<api::card*>* getQuery(std::string_view name)
	{
		return queryStore.tryGet(name);
	}
	void pushQuery(std::vector<api::card*>& cards)
	{
		queryCache.emplace(cards);
	}

	void clearCache() { queryCache.reset(); }
	void clearStore() { queryStore.clear(); }

	void addPreRenderEvent(postLoadEvent&& event)
	{
		preRenderEvents.push_back(std::move(event));
	}

	void executePreRender(Rml::ElementDocument& document)
	{
		for (auto& event : preRenderEvents)
		{
			event(document);
		}
		preRenderEvents.clear();
	}
};

queryCache& cache()
{
	static queryCache instance;
	return instance;
}


using queryFilter = bool(*)(queryModel&, const Rml::VariantList&);
using queryTransform = std::optional<std::string>(*)(const queryModel&, const Rml::VariantList&);

void bindQueryFilter(Rml::DataModelConstructor& constructor, const std::string& name, queryFilter filter)
{
	constructor.RegisterTransformFunc(name, [filter](const Rml::VariantList& args) -> Rml::Variant
		{
			auto query = cache().getOrCreateQuery(args);
			if (!query)
				return Rml::Variant{};
			auto result = filter(*query, args);
			if (result)
				return Rml::Variant{ "Unresolved Query" };
			return Rml::Variant{};
		});
}

void bindQueryTransform(Rml::DataModelConstructor& constructor, const std::string& name, queryTransform transform)
{
	constructor.RegisterTransformFunc(name, [transform](const Rml::VariantList& args) -> Rml::Variant
		{
			auto query = cache().getOrCreateQuery(args);
			if (!query)
				return Rml::Variant{};
			auto result = transform(*query, args);
			cache().clearCache();
			if (result)
				return Rml::Variant{ *result };
			return Rml::Variant{};
		});
}

namespace filters
{

	bool where(queryModel& query, const Rml::VariantList& args)
	{
		logger::logFor<queryLogSettings>().log("where all: ");
		for (int i = 1; i < args.size(); i++)
		{
			auto t = UI::transform::parse(args[i].Get<std::string>());
			if (!t)
			{
				logger::logFor<queryLogSettings>().flogLn("Unable to parse transform '{}' ", args[i].Get<std::string>());
				return false;
			}
			logger::logFor<queryLogSettings>().flog("'{}' ", args[i].Get<std::string>());
			query.match(*t);
		}
		return true;
	}

	bool whereAny(queryModel& query, const Rml::VariantList& args)
	{
		logger::logFor<queryLogSettings>().log("where any: ");
		std::vector<UI::transform> transforms;
		for (int i = 1; i < args.size(); i++)
		{
			auto t = UI::transform::parse(args[i].Get<std::string>());
			if (!t)
			{
				logger::logFor<queryLogSettings>().flogLn("Unable to parse transform '{}' ", args[i].Get<std::string>());
				return false;
			}
			logger::logFor<queryLogSettings>().flog("'{}' ", args[i].Get<std::string>());
			transforms.emplace_back(*t);
		}
		query.matchAny(transforms);
		return true;
	}


	bool today(queryModel& query, const Rml::VariantList& args)
	{
		auto now = std::chrono::system_clock::now();
		auto today = std::format("{:%Y-%m-%d}", now);

		logger::logFor<queryLogSettings>().log("Matching today's date ");

		for (int i = 1; i < args.size(); i++)
		{
			UI::transform t;
			t.key = args[i].Get<std::string>();
			t.op = UI::transform::operation::startsWith;
			t.value = today;
			query.match(t);
		}
		return true;
	}

	bool withLabel(queryModel& query, const Rml::VariantList& args)
	{
		if (args.size() < 2)
		{
			logger::logFor<queryLogSettings>().flogLn("withLabel: Expected at least 1 argument");
			return false;
		}
		std::vector<std::string> labels;
		for (int i = 1; i < args.size(); i++)
		{
			auto label = args[i].Get<std::string>();
			labels.push_back(label);
		}
		logger::logFor<queryLogSettings>().flogLn("withLabel: ({}).", labels.size());
		query.filter([&labels](const api::card* card)
			{
				return std::ranges::any_of(card->labels, [&labels](const auto& label) -> bool
					{
						return std::ranges::contains(labels, label);
					});
			});
		return true;
	}

	bool withoutLabel(queryModel& query, const Rml::VariantList& args)
	{
		if (args.size() < 2)
		{
			logger::logFor<queryLogSettings>().flogLn("withoutLabel: Expected at least 1 argument");
			return false;
		}
		std::vector<std::string> labels;
		for (int i = 1; i < args.size(); i++)
		{
			auto label = args[i].Get<std::string>();
			labels.push_back(label);
		}
		logger::logFor<queryLogSettings>().flogLn("withoutLabel: ({}).", labels.size());
		query.filter([&labels](const api::card* card)
			{
				return std::ranges::none_of(card->labels, [&labels](const auto& label) -> bool
					{
						return std::ranges::contains(labels, label);
					});
			});
		return true;
	}
}

namespace transforms
{
	std::optional<std::string> count(const queryModel& query, const Rml::VariantList&)
	{
		int count = static_cast<int>(query.filtered.size());
		logger::logFor<queryLogSettings>().flogLn("Count: ({}).", count);
		return std::to_string(count);
	}

	std::optional<std::string> sum(const queryModel& query, const Rml::VariantList& args)
	{
		if (query.filtered.empty())
		{
			logger::logFor<queryLogSettings>().logLn("Sum: (No Cards).");
			return "0";
		}
		if (args.size() != 2)
		{
			logger::logFor<queryLogSettings>().flogLn("Sum: Expected 1 argument, got {}.", args.size() - 1);
			return std::nullopt;
		}
		auto key = args[1].Get<std::string>();
		int sum = 0;
		for (const auto& card : query.filtered)
		{
			auto value = card->customFields[key];
			if (value.empty())
			{
				//Assume empty values are 0
				continue;
			}

			int valueInt = 0;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), valueInt);
			if (ec != std::errc{})
			{
				logger::logFor<queryLogSettings>().flogLn("Sum: (Invalid value '{}').", value);
				return std::nullopt;
			}
			sum += valueInt;
		}
		logger::logFor<queryLogSettings>().flogLn("Sum of {}: ({}).", key, sum);
		return std::to_string(sum);
	}

	std::optional<std::string> mostRecent(const queryModel& query, const Rml::VariantList&)
	{
		if (query.filtered.empty())
		{
			logger::logFor<queryLogSettings>().logLn("Most recent: (No Cards).");
			return std::nullopt;
		}

		auto mostRecent = std::ranges::max_element(query.filtered, [](const auto& a, const auto& b)
			{
				return a->lastUpdate < b->lastUpdate;
			});
		auto lastUpdate = std::chrono::floor<std::chrono::seconds>((*mostRecent)->lastUpdate);
		auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), lastUpdate };
		auto result = std::format("{:%Y-%m-%d %H:%M:%S}", localTime.get_local_time());
		logger::logFor<queryLogSettings>().flogLn("Most recent: ({}).", result);
		return result;
	}

	std::optional<std::string> members(const queryModel& query, const Rml::VariantList& args)
	{
		std::optional<std::string> formatter;
		if (args.size() == 2)
		{
			formatter = args[1].Get<std::string>();
		}

		std::vector<const api::member*> members;
		for (const auto& card : query.filtered)
		{
			for (auto member : card->members)
			{
				if (!std::ranges::contains(members, member))
				{
					members.push_back(member);
				}
			}
		}

		if (members.empty())
		{
			logger::logFor<queryLogSettings>().flogLn("No members found.");
			return std::nullopt;
		}

		std::ranges::sort(members, [](auto a, auto b)
			{
				return a->fullName < b->fullName;
			});

		auto format = [formatter](const std::string& name)
			{
				if (!formatter)
					return name;

				auto div = name.find(' ');
				std::string_view first{ name.begin(), name.begin() + div };
				std::string_view last{ name.begin() + div + 1, name.end() };

				if (*formatter == "first")
					return std::string{ first };
				else if (*formatter == "last")
					return std::string{ last };
				else if (*formatter == "initialsDot")
					return std::string{ first[0], '.', last[0], '.' };
				else if (*formatter == "initials")
					return std::string{ first[0], last[0] };

				return name;
			};

		std::string result;
		for (const auto& member : members)
		{
			result += format(member->fullName) + ", ";
		}
		result.resize(result.size() - 2); //Remove the last comma and space
		logger::logFor<queryLogSettings>().flogLn("Members: ({}).", result);
		return result;
	}

	std::optional<std::string> singleProfilePictureId(const queryModel& query, const Rml::VariantList& args)
	{
		if (query.filtered.size() != 1)
		{
			logger::logFor<queryLogSettings>().flogLn("Picture: Query does not have a single card.");
			return std::nullopt;
		}
		auto firstCard = query.filtered[0];
		if (firstCard->members.size() != 1)
		{
			logger::logFor<queryLogSettings>().flogLn("Picture: Query does not have exactly 1 member.");
			return std::nullopt;
		}
		auto firstMember = firstCard->members[0];
		if (firstMember->profilePicturePath.empty())
		{
			logger::logFor<queryLogSettings>().flogLn("Picture: Member does not have a profile picture.");
			return std::nullopt;
		}
		if (args.size() != 2)
		{
			//Note that first argument is implied
			logger::logFor<queryLogSettings>().flogLn("Picture: Expected 1 argument, got {}.", args.size());
			return std::nullopt;
		}

		std::string path = firstMember->profilePicturePath;
		std::string elementId = args[1].Get<std::string>();

		cache().addPreRenderEvent([path, elementId](Rml::ElementDocument& document)
			{
				auto element = document.GetElementById(elementId);
				if (element)
				{
					element->SetAttribute("src", path.c_str());
				}
			});

		logger::logFor<queryLogSettings>().flogLn("Picture: Set profile picture for element '{}' to '{}'.", elementId, path);
		return std::nullopt;
	}

}

namespace meta
{
	Rml::Variant saveQuery(const Rml::VariantList& args)
	{
		if (args.size() != 2 || !cache().queryCached())
		{
			logger::logFor<queryLogSettings>().logLn("Unable to save query, expected 1 arg and a query in progress.");
			return Rml::Variant{};
		}

		auto name = args[1].Get<std::string>();
		if (!cache().saveQuery(name))
			logger::logFor<queryLogSettings>().flogLn("Unable to save query '{}'.", name);
		cache().clearCache();
		logger::logFor<queryLogSettings>().flogLn("Saved as '{}'.", name);
		//Note that we don't want to provide output after saving the query, as we can therefore save from content (aka, uniquely dont display "Unresolved Query")
		return Rml::Variant{ "" };
	}

	static Rml::Variant loadQuery(const Rml::VariantList& args)
	{
		//Don't allow loading if we have a query in progress, as we can't load into it
		if (cache().queryCached())
		{
			logger::logFor<queryLogSettings>().logLn("Unable to load query, query already in progress.");
			return Rml::Variant{};
		}

		if (args.size() != 2)
		{
			logger::logFor<queryLogSettings>().logLn("Unable to load query, expected 2 args.");
			return Rml::Variant{};
		}

		if (args[0].Get<std::string>("") != "Loaded Query")
		{
			logger::logFor<queryLogSettings>().logLn("Unable to load query, expected first arg to be the load expression.");
			return Rml::Variant{};
		}

		auto name = args[1].Get<std::string>();
		auto it = cache().getQuery(name);
		if (!it)
			return Rml::Variant{};
		cache().pushQuery(*it);
		logger::logFor<queryLogSettings>().flog("From '{}'.", name);
		return Rml::Variant{ "Unresolved Query" };
	}
}

void query::bind(Rml::DataModelConstructor& constructor)
{
	bindQueryFilter(constructor, "where", filters::where);
	bindQueryFilter(constructor, "whereAny", filters::whereAny);
	bindQueryFilter(constructor, "today", filters::today);
	bindQueryFilter(constructor, "withLabel", filters::withLabel);
	bindQueryFilter(constructor, "withoutLabel", filters::withoutLabel);

	bindQueryTransform(constructor, "count", transforms::count);
	bindQueryTransform(constructor, "sum", transforms::sum);
	bindQueryTransform(constructor, "mostRecent", transforms::mostRecent);
	bindQueryTransform(constructor, "members", transforms::members);
	bindQueryTransform(constructor, "singleProfilePicture", transforms::singleProfilePictureId);

	constructor.RegisterTransformFunc("as", meta::saveQuery);
	constructor.RegisterTransformFunc("from", meta::loadQuery);
}

void query::setCards(keyValueLookup<std::string, std::vector<api::card>>&& cards)
{
	cache().setCards(std::move(cards));
}

void query::preRender(Rml::ElementDocument& document)
{
	cache().executePreRender(document);
}