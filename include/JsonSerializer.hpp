#ifndef SYSTEM_MONITORING_AGENT_JSON_SERIALIZER_HPP
#define SYSTEM_MONITORING_AGENT_JSON_SERIALIZER_HPP

#include "Metric.hpp" // for monitoring::Metric

#include <string> // for std::string
#include <vector> // for std::vector

namespace monitoring
{

class JsonSerializer
{
public:
    std::string Serialize(
        const std::string& agentId,
        const std::vector<Metric>& metrics) const;
};

} // namespace monitoring

#endif // SYSTEM_MONITORING_AGENT_JSON_SERIALIZER_HPP