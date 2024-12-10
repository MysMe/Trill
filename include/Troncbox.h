#pragma once
#include <string_view>
#include "Keys.h"

class troncboxHandler
{
    keys accessKeys;
public:
    troncboxHandler(keys accessKeys) : accessKeys(std::move(accessKeys)) {}

    struct troncboxResponse
    {
        std::size_t employeesWithoutEmployeeTB = 0;
        std::size_t employeesWithoutPrimaryRole = 0;
    };

    troncboxResponse get() const;
};