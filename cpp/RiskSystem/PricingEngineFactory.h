#ifndef PRICINGENGINEFACTORY_H
#define PRICINGENGINEFACTORY_H

#include "../Models/IPricingEngine.h"
#include "PricingEngineConfig.h"
#include <map>
#include <string>

class PricingEngineFactory {
public:
    static IPricingEngine* createPricingEngine(const std::string& typeName);
    static std::map<std::string, IPricingEngine*> createPricersFromConfig(const PricingEngineConfig& config);
    static std::map<std::string, IPricingEngine*> createPricersFromConfigFile(const std::string& configFile);
};

#endif // PRICINGENGINEFACTORY_H