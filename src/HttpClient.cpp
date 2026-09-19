#include "HttpClient.hpp" // for monitoring::HttpClient
#include <httplib.h>      // for httplib::Client

namespace monitoring
{

    bool HttpClient::PostJson(const std::string& endpoint, const std::string& jsonBody) const
    {
        httplib::Client client(endpoint);
        client.set_max_timeout(kTimeout);

        auto response = client.Post("/", jsonBody, "application/json");

        return response && response->status >= 200 && response->status < 300;
    }

} // namespace monitoring
