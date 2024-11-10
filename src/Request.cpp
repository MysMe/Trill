#include "Request.h"
#include <format>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>

nlohmann::json requestHandler::get(std::string_view url) const
{
	//Check if the URL already has parameters, don't double-add a '?'
    bool hasParams = url.find('?') != std::string::npos;
    std::string fullUrl = std::format("{}{}key={}&token={}", url, hasParams ? '&' : '?', accessKeys.apiKey, accessKeys.userToken);

    curlpp::Easy request;
    request.setOpt<curlpp::options::Url>(fullUrl);

    std::ostringstream response;
    request.setOpt<curlpp::options::WriteStream>(&response);
    request.perform();
	//Assume the response is JSON
    return nlohmann::json::parse(response.str());
}
