#pragma once
#include <string>
#include <cstdint>
#include "Troncbox.h"
#include <RmlUi/Core.h>

namespace UI
{
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

		struct trelloSummary
		{
			size_t count;
			size_t unseen;
			size_t blocked;
			std::string lastUpdate;

			void from(const issues::issueSummary& summary)
			{
				count = summary.count;
				unseen = summary.unseen;
				blocked = summary.blocked;
				lastUpdate = timeToString(summary.lastUpdate);
			}

			void add(const issues::issueSummary& summary)
			{
				count += summary.count;
				unseen += summary.unseen;
				blocked += summary.blocked;
			}
		};

		struct troncboxSummary
		{
			size_t employeesWithoutEmployeeTB;
			size_t employeesWithoutPrimaryRole;
		};

		std::size_t resolved;
		std::string syncLast;	

		std::array<trelloSummary, 4> trelloSummaries;
		trelloSummary trelloAll;
		troncboxSummary troncbox;

		model() = default;
		//The UI holds a reference to the model, so we need to make sure it's not copied or moved.
		model(const model&) = delete;
		model& operator=(const model&) = delete;
		model(model&&) = delete;
		model& operator=(model&&) = delete;
		~model() = default;

		void set(const keyValueLookup<fields::priority, issues::issueSummary>& issues, size_t resolvedCount, troncboxHandler::troncboxResponse troncboxData)
		{
			trelloAll = {};
			const auto priorities = { fields::priority::hotfix, fields::priority::regular, fields::priority::low, fields::priority::veryLow };
			std::optional<std::chrono::system_clock::time_point> minTime;

			for (auto priority : priorities)
			{
				const issues::issueSummary defaultSummary{};
				const auto& summary = issues.getOr(priority, defaultSummary);
				auto& trelloSummary = trelloSummaries[static_cast<size_t>(priority)];
				
				trelloSummary.from(summary);
				trelloAll.add(summary);
				if (!minTime)
					minTime = summary.lastUpdate;
				else if (summary.lastUpdate && summary.lastUpdate < minTime)
					minTime = summary.lastUpdate;
			}
			trelloAll.lastUpdate = timeToString(minTime);
			syncLast = timeToString(std::chrono::system_clock::now());

			troncbox.employeesWithoutEmployeeTB = troncboxData.employeesWithoutEmployeeTB;
			troncbox.employeesWithoutPrimaryRole = troncboxData.employeesWithoutPrimaryRole;
			resolved = resolvedCount;
		}

		void bind(Rml::Context* context, Rml::DataModelHandle& model)
		{
			Rml::DataModelConstructor constructor = context->CreateDataModel("model");
			if (!constructor)
				return;

			if (auto handle = constructor.RegisterStruct<trelloSummary>())
			{
				handle.RegisterMember("count", &trelloSummary::count);
				handle.RegisterMember("unseen", &trelloSummary::unseen);
				handle.RegisterMember("blocked", &trelloSummary::blocked);
				handle.RegisterMember("last", &trelloSummary::lastUpdate);
			}

			if (auto handle = constructor.RegisterStruct<troncboxSummary>())
			{
				handle.RegisterMember("missingTB", &troncboxSummary::employeesWithoutEmployeeTB);
				handle.RegisterMember("missingPrimary", &troncboxSummary::employeesWithoutPrimaryRole);
			}

			constructor.Bind("trelloAll", &trelloAll);
			constructor.Bind("hotfixes", &trelloSummaries[0]);
			constructor.Bind("medium", &trelloSummaries[1]);
			constructor.Bind("low", &trelloSummaries[2]);
			constructor.Bind("veryLow", &trelloSummaries[3]);
			constructor.Bind("troncbox", &troncbox);
			constructor.Bind("syncLast", &syncLast);
			constructor.Bind("resolved", &resolved);

			model = constructor.GetModelHandle();
		}
	};
}