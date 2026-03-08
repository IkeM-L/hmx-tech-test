#include "PricingConfigLoader.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace
{
    std::string readFileContents(const std::string& filename)
    {
        if (filename.empty())
        {
            throw std::invalid_argument("Config filename cannot be empty");
        }

        std::ifstream stream(filename);
        if (!stream.is_open())
        {
            throw std::runtime_error("Cannot open config file: " + filename);
        }

        std::ostringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

    std::string extractAttribute(const std::string& element, const std::string& attributeName)
    {
        const std::regex attributeRegex(attributeName + "=\"([^\"]*)\"");
        std::smatch match;

        if (!std::regex_search(element, match, attributeRegex))
        {
            throw std::runtime_error("Missing XML attribute: " + attributeName);
        }

        return match[1].str();
    }
}

std::string PricingConfigLoader::getConfigFile() const
{
    return configFile_;
}

void PricingConfigLoader::setConfigFile(const std::string& file)
{
    configFile_ = file;
}

PricingEngineConfig PricingConfigLoader::parseXml(const std::string& content)
{
    PricingEngineConfig config;

    const std::regex engineRegex(R"(<Engine\s+[^>]*/>)");
    const auto begin = std::sregex_iterator(content.begin(), content.end(), engineRegex);
    const auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it)
    {
        const std::string element = it->str();

        PricingEngineConfigItem item;
        item.setTradeType(extractAttribute(element, "tradeType"));
        item.setAssembly(extractAttribute(element, "assembly"));
        item.setTypeName(extractAttribute(element, "pricingEngine"));

        config.push_back(item);
    }

    if (config.empty())
    {
        throw std::runtime_error("No pricing engine configuration entries found");
    }

    return config;
}

PricingEngineConfig PricingConfigLoader::loadConfig() const
{
    const std::string content = readFileContents(configFile_);
    return parseXml(content);
}
