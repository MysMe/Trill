#include "Query.h"
#include "APIData.h"

using namespace query;

using postLoadEvent = std::function<void(Rml::ElementDocument&)>;

class queryCache
{
	std::optional<queryModel> queryCache;
	keyValueLookup<std::string, std::vector<api::card*>> queryStore;
	keyValueLookup<std::string, std::vector<api::card>> cardLists;

	std::vector<postLoadEvent> postLoadEvents;

public:
	queryModel* getOrCreateQuery(const Rml::VariantList& args)
	{
		//Query already in progress, so continue that one
		if (queryCache)
			return &queryCache.value();
		//No running query and we can't start one
		if (args.empty())
			return nullptr;
		std::string name;
		if (!args[0].GetInto(name))
			return nullptr;
		auto list = cardLists.tryGet(name);
		if (!list)
			return nullptr;
		queryCache.emplace(*list);
		return &queryCache.value();
	}

	void setCards(keyValueLookup<std::string, std::vector<api::card>>&& cards)
	{
		clearCache();
		clearStore();
		cardLists = std::move(cards);
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

	void addPostLoadEvent(postLoadEvent&& event)
	{
		postLoadEvents.push_back(std::move(event));
	}

	void executePostLoad(Rml::ElementDocument& document)
	{
		for (auto& event : postLoadEvents)
		{
			event(document);
		}
		postLoadEvents.clear();
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
		for (int i = 1; i < args.size(); i++)
		{
			auto t = UI::transform::parse(args[i].Get<std::string>());
			if (!t)
				return false;
			query.match(*t);
		}
		return true;
	}

	bool whereAny(queryModel& query, const Rml::VariantList& args)
	{
		std::vector<UI::transform> transforms;
		for (int i = 1; i < args.size(); i++)
		{
			auto t = UI::transform::parse(args[i].Get<std::string>());
			if (!t)
				return false;
			transforms.emplace_back(*t);
		}
		query.matchAny(transforms);
		return true;
	}


	bool today(queryModel& query, const Rml::VariantList& args)
	{
		auto now = std::chrono::system_clock::now();
		auto today = std::format("{:%Y-%m-%d}", now);

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
}

namespace transforms
{
	std::optional<std::string> count(const queryModel& query, const Rml::VariantList&)
	{
		int count = static_cast<int>(query.filtered.size());
		return std::to_string(count);
	}

	std::optional<std::string> mostRecent(const queryModel& query, const Rml::VariantList&)
	{
		if (query.filtered.empty())
		{
			return std::nullopt;
		}

		auto mostRecent = std::ranges::max_element(query.filtered, [](const auto& a, const auto& b)
			{
				return a->lastUpdate < b->lastUpdate;
			});
		auto lastUpdate = (*mostRecent)->lastUpdate;
		return std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(lastUpdate));
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
			return std::nullopt;

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
		return result;
	}

	std::optional<std::string> singleProfilePictureId(const queryModel& query, const Rml::VariantList& args)
	{
		if (query.filtered.size() != 1)
			return std::nullopt;
		auto firstCard = query.filtered[0];
		if (firstCard->members.size() != 1)
			return std::nullopt;
		auto firstMember = firstCard->members[0];
		if (firstMember->profilePicturePath.empty())
			return std::nullopt;
		if (args.size() != 2)
			return std::nullopt;

		std::string path = firstMember->profilePicturePath;
		std::string elementId = args[1].Get<std::string>();

		cache().addPostLoadEvent([path, elementId](Rml::ElementDocument& document)
			{
				auto element = document.GetElementById(elementId);
				if (element)
				{
					element->SetAttribute("src", path.c_str());
				}
			});

		return std::nullopt;
	}

}

namespace meta
{
	Rml::Variant saveQuery(const Rml::VariantList& args)
	{
		if (args.size() != 2 || !cache().queryCached())
			return Rml::Variant{};

		auto name = args[1].Get<std::string>();
		cache().saveQuery(name);
		cache().clearCache();
		//Note that we don't want to provide output after saving the query, as we can therefore save from content (aka, uniquely dont display "Unresolved Query")
		return Rml::Variant{ "" };
	}

	static Rml::Variant loadQuery(const Rml::VariantList& args)
	{
		//Don't allow loading if we have a query in progress, as we can't load into it
		if (cache().queryCached())
			return Rml::Variant{};

		if (args.size() != 1)
			return Rml::Variant{};

		auto name = args[0].Get<std::string>();
		auto it = cache().getQuery(name);
		if (!it)
			return Rml::Variant{};
		cache().pushQuery(*it);
		return Rml::Variant{ "Unresolved Query" };
	}
}

void query::bind(Rml::DataModelConstructor& constructor)
{
	bindQueryFilter(constructor, "where", filters::where);
	bindQueryFilter(constructor, "whereAny", filters::whereAny);
	bindQueryFilter(constructor, "today", filters::today);

	bindQueryTransform(constructor, "count", transforms::count);
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

void query::postLoad(Rml::ElementDocument& document)
{
	cache().executePostLoad(document);
}