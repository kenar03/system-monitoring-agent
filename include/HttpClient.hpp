#ifndef SYSTEM_MONITORING_AGENT_HTTP_CLIENT_HPP
#define SYSTEM_MONITORING_AGENT_HTTP_CLIENT_HPP

#include <chrono> // for std::chrono::seconds
#include <string> // for std::string

namespace monitoring
{

    class HttpClient
    {
    public:
        bool PostJson(const std::string& endpoint, const std::string& jsonBody) const;

    private:
        static constexpr std::chrono::seconds kTimeout{5};
    };

} // namespace monitoring

#endif // SYSTEM_MONITORING_AGENT_HTTP_CLIENT_HPP