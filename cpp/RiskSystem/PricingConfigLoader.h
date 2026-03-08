#ifndef PRICINGCONFIGLOADER_H
#define PRICINGCONFIGLOADER_H

#include "PricingEngineConfig.h"
#include <string>

class PricingConfigLoader {
private:
    std::string configFile_;
    static PricingEngineConfig parseXml(const std::string& content);
    
public:
    /// Returns the configured pricing-engine XML file path.
    [[nodiscard]] std::string getConfigFile() const;
    /// Sets the pricing-engine XML file path.
    void setConfigFile(const std::string& file);
    /// Loads and parses the pricing-engine configuration file.
    [[nodiscard]] PricingEngineConfig loadConfig() const;
};

#endif // PRICINGCONFIGLOADER_H
