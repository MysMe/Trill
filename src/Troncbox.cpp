#include "Troncbox.h"
#include <format>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <nlohmann/json.hpp>

troncboxHandler::troncboxResponse troncboxHandler::get() const
{
    troncboxHandler::troncboxResponse result;

    std::string empTBUrl = std::format("https://troncbox-api.azurewebsites.net/api/Employee/Incomplete?seedKey={}", accessKeys.troncboxKey);
    std::string empPrimaryUrl = std::format("https://troncbox-api.azurewebsites.net/api/Employee/MissingPrimary?seedKey={}", accessKeys.troncboxKey);

    curlpp::Easy request;
    request.setOpt<curlpp::options::Url>(empTBUrl);

    std::ostringstream response;
    request.setOpt<curlpp::options::WriteStream>(&response);
    request.perform();
	//Assume the response is JSON
    auto tbResponse = nlohmann::json::parse(response.str());
    result.employeesWithoutEmployeeTB = tbResponse["data"];

    request.setOpt<curlpp::options::Url>(empPrimaryUrl);
    response = {};
    request.setOpt<curlpp::options::WriteStream>(&response);
    request.perform();

    auto primaryResponse = nlohmann::json::parse(response.str());
    result.employeesWithoutPrimaryRole = primaryResponse["data"];

    return result;
}
