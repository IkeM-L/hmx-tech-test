#include "PricingEngineFactory.h"

#include "../Pricers/GovBondPricingEngine.h"
#include "../Pricers/CorpBondPricingEngine.h"
#include "../Pricers/FxPricingEngine.h"
#include "PricingConfigLoader.h"

#include <memory>
#include <stdexcept>

std::unique_ptr<IPricingEngine> PricingEngineFactory::createPricingEngine(const std::string& typeName)
{
    if (typeName == "HmxLabs.TechTest.Pricers.GovBondPricingEngine")
    {
        return std::make_unique<GovBondPricingEngine>();
    }

    if (typeName == "HmxLabs.TechTest.Pricers.CorpBondPricingEngine")
    {
        return std::make_unique<CorpBondPricingEngine>();
    }

    if (typeName == "HmxLabs.TechTest.Pricers.FxPricingEngine")
    {
        return std::make_unique<FxPricingEngine>();
    }

    throw std::runtime_error("Unknown pricing engine type: " + typeName);
}

PricingEngineFactory::PricingEngineMap PricingEngineFactory::createPricersFromConfig(
    const PricingEngineConfig& config)
{
    PricingEngineMap pricers;
    for (const auto& configItem : config)
    {
        const std::string& tradeType = configItem.getTradeType();
        const std::string& typeName = configItem.getTypeName();
        pricers[tradeType] = createPricingEngine(typeName);
    }
    return pricers;
}

PricingEngineFactory::PricingEngineMap PricingEngineFactory::createPricersFromConfigFile(
    const std::string& configFile)
{
    PricingConfigLoader pricingConfigLoader;
    pricingConfigLoader.setConfigFile(configFile);
    const PricingEngineConfig config = pricingConfigLoader.loadConfig();

    return createPricersFromConfig(config);
}
