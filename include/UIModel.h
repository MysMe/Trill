#pragma once
#include <string>
#include <cstdint>
#include <RmlUi/Core.h>

namespace UI
{
	struct model
	{
		size_t hotfixes = 0;
		size_t hotfixesUnseen = 0;
		std::string hotfixesLast;

		size_t medium = 0;
		size_t mediumUnseen = 0;
		std::string mediumLast;

		size_t low = 0;
		size_t lowUnseen = 0;
		std::string lowLast;

		size_t veryLow = 0;
		size_t veryLowUnseen = 0;
		std::string veryLowLast;

		size_t resolved = 0;
		std::string syncLast;	

		model() = default;
		//The UI holds a reference to the model, so we need to make sure it's not copied or moved.
		model(const model&) = delete;
		model& operator=(const model&) = delete;
		model(model&&) = delete;
		model& operator=(model&&) = delete;
		~model() = default;

		void set(const keyValueLookup<fields::priority, issues::issueSummary>& issues, size_t resolved)
		{
			auto timeToString = [](const std::optional<std::chrono::system_clock::time_point>& time) -> std::string
				{
					if (!time.has_value())
						return "";
					auto time_t = std::chrono::system_clock::to_time_t(time.value());
					std::tm tm = *std::localtime(&time_t);
					std::string result(20, '\0');
					std::strftime(result.data(), result.size(), "%F %T", &tm);
					return result;
				};

			const auto& hotfixSummary = issues.getOr(fields::priority::hotfix, {});
			hotfixes = hotfixSummary.count;
			hotfixesUnseen = hotfixSummary.unseen;
			hotfixesLast = timeToString(hotfixSummary.lastUpdate);

			const auto& mediumSummary = issues.getOr(fields::priority::regular, {});
			medium = mediumSummary.count;
			mediumUnseen = mediumSummary.unseen;
			mediumLast = timeToString(mediumSummary.lastUpdate);

			const auto& lowSummary = issues.getOr(fields::priority::low, {});
			low = lowSummary.count;
			lowUnseen = lowSummary.unseen;
			lowLast = timeToString(lowSummary.lastUpdate);

			const auto& veryLowSummary = issues.getOr(fields::priority::veryLow, {});
			veryLow = veryLowSummary.count;
			veryLowUnseen = veryLowSummary.unseen;
			veryLowLast = timeToString(veryLowSummary.lastUpdate);

			resolved = resolved;
			syncLast = timeToString(std::chrono::system_clock::now());
		}

		void bind(Rml::Context* context, Rml::DataModelHandle& model)
		{
			Rml::DataModelConstructor constructor = context->CreateDataModel("model");
			if (!constructor)
				return;
			constructor.Bind("hotfixes", &hotfixes);
			constructor.Bind("hotfixesUnseen", &hotfixesUnseen);
			constructor.Bind("hotfixesLast", &hotfixesLast);
			constructor.Bind("medium", &medium);
			constructor.Bind("mediumUnseen", &mediumUnseen);
			constructor.Bind("mediumLast", &mediumLast);
			constructor.Bind("low", &low);
			constructor.Bind("lowUnseen", &lowUnseen);
			constructor.Bind("lowLast", &lowLast);
			constructor.Bind("veryLow", &veryLow);
			constructor.Bind("veryLowUnseen", &veryLowUnseen);
			constructor.Bind("veryLowLast", &veryLowLast);
			constructor.Bind("resolved", &resolved);
			constructor.Bind("syncLast", &syncLast);
			model = constructor.GetModelHandle();
		}
	};
}