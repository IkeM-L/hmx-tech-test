#ifndef PRICINGENGINEFACTORY_H
#define PRICINGENGINEFACTORY_H

#include "../Models/IPricingEngine.h"
#include "PricingEngineConfig.h"
#include <map>
#include <string>

class PricingEngineFactory {
public:
    /// Creates a pricing engine instance for the configured type name.
    static IPricingEngine* createPricingEngine(const std::string& typeName);
    /// Builds a trade-type to pricing-engine map from configuration entries.
    static std::map<std::string, IPricingEngine*> createPricersFromConfig(const PricingEngineConfig& config);
    /// Loads configuration from disk and builds the pricing-engine map.
    static std::map<std::string, IPricingEngine*> createPricersFromConfigFile(const std::string& configFile);
};

#endif // PRICINGENGINEFACTORY_H
