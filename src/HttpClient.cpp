#include "HttpClient.hpp" // for monitoring::HttpClient
#include <httplib.h>      // for httplib::Client

#include <string> // for std::string
#include <utility> // for std::pair

namespace monitoring
{
    namespace
    {
        std::pair<std::string, std::string> SplitEndpoint(const std::string& endpoint)
        {
            const auto schemeSeparator = endpoint.find("://");
            const auto pathSearchStart = schemeSeparator == std::string::npos ? 0 : schemeSeparator + 3;
            const auto pathStart = endpoint.find('/', pathSearchStart);

            if (pathStart == std::string::npos)
            {
                return {endpoint, "/"};
            }

            std::string baseUrl = endpoint.substr(0, pathStart);
            std::string path = endpoint.substr(pathStart);

            if (baseUrl.empty())
            {
                baseUrl = endpoint;
                path = "/";
            }

            return {std::move(baseUrl), std::move(path)};
        }
    } // namespace

    bool HttpClient::PostJson(const std::string& endpoint, const std::string& jsonBody) const
    {
        const auto [baseUrl, path] = SplitEndpoint(endpoint);

        httplib::Client client(baseUrl);
        client.set_max_timeout(kTimeout);

        auto response = client.Post(path, jsonBody, "application/json");

        return response && response->status >= 200 && response->status < 300;
    }

} // namespace monitoring
