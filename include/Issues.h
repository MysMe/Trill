#pragma once
#include <string>
#include <string_view>
#include "KeyValueLookup.h"
#include "Request.h"
#include "CustomFields.h"

namespace api
{
    struct data;
    struct card;
}

namespace issues
{
    std::vector<api::card> getCurrentCards(const requestHandler &api, const api::data &data);
}