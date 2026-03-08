#ifndef PRICINGCONFIGLOADER_H
#define PRICINGCONFIGLOADER_H

#include "PricingEngineConfig.h"
#include <string>

class PricingConfigLoader {
private:
    std::string configFile_;
    static PricingEngineConfig parseXml(const std::string& content);
    
public:
    [[nodiscard]] std::string getConfigFile() const;
    void setConfigFile(const std::string& file);
    [[nodiscard]] PricingEngineConfig loadConfig() const;
};

#endif // PRICINGCONFIGLOADER_H
