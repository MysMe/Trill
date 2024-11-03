#include "Resolutions.h"
#include "Urls.h"
#include <chrono>
#include <format>
#include "APIData.h"

namespace resolutions
{
    size_t getTicketsResolvedToday(const requestHandler &api, const api::data &data)
    {
        auto cards = api.get(urls::getCardsInList(data.resolvedListId));
        size_t resolved = 0;

        //Get today in the form of "YYYY-MM-DD"
        auto now = std::chrono::system_clock::now();
        auto today = std::format("{:%Y-%m-%d}", now);

        for (auto &card : cards)
        {
            for (auto &fieldItem : card["customFieldItems"])
            {
                if (fieldItem["idCustomField"].is_null() || fieldItem["idCustomField"] != data.customFields.rKeys.fieldId)
                {
                    continue;
                }
                auto resolvedAtDate = fieldItem["value"]["date"];
                if (resolvedAtDate.is_null())
                {
                    continue;
                }
                auto resolvedAt = resolvedAtDate.get<std::string>();
                // Check if the card was resolved today, regardless of the time
                //(CBA to deal with timezones, don't resolve things at midnight)
                if (resolvedAt.starts_with(today))
                {
                    resolved++;
                }
            }
        }

        return resolved;
    }
}