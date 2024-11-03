#pragma once
#include <string>
#include <string_view>
#include "Request.h"

namespace api
{
    struct data;
}

namespace resolutions
{
    size_t getTicketsResolvedToday(const requestHandler &api, const api::data &data);
}