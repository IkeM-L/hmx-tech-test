#ifndef PRICINGENGINEFACTORY_H
#define PRICINGENGINEFACTORY_H

#include "../Models/IPricingEngine.h"
#include "PricingEngineConfig.h"
#include <map>
#include <memory>
#include <string>

class PricingEngineFactory {
public:
    using PricingEngineMap = std::map<std::string, std::unique_ptr<IPricingEngine>>;

    /// Creates a pricing engine instance for the configured type name.
    static std::unique_ptr<IPricingEngine> createPricingEngine(const std::string& typeName);
    /// Builds a trade-type to pricing-engine map from configuration entries.
    static PricingEngineMap createPricersFromConfig(const PricingEngineConfig& config);
    /// Loads configuration from disk and builds the pricing-engine map.
    static PricingEngineMap createPricersFromConfigFile(const std::string& configFile);
};

#endif // PRICINGENGINEFACTORY_H
