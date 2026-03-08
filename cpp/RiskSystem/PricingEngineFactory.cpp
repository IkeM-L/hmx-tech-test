#include "PricingEngineFactory.h"

#include "../Pricers/GovBondPricingEngine.h"
#include "../Pricers/CorpBondPricingEngine.h"
#include "../Pricers/FxPricingEngine.h"
#include "PricingConfigLoader.h"

#include <memory>
#include <stdexcept>

IPricingEngine* PricingEngineFactory::createPricingEngine(const std::string& typeName)
{
    if (typeName == "HmxLabs.TechTest.Pricers.GovBondPricingEngine")
    {
        return new GovBondPricingEngine();
    }

    if (typeName == "HmxLabs.TechTest.Pricers.CorpBondPricingEngine")
    {
        return new CorpBondPricingEngine();
    }

    if (typeName == "HmxLabs.TechTest.Pricers.FxPricingEngine")
    {
        return new FxPricingEngine();
    }

    throw std::runtime_error("Unknown pricing engine type: " + typeName);
}

std::map<std::string, IPricingEngine*> PricingEngineFactory::createPricersFromConfig(const PricingEngineConfig& config)
{
    std::map<std::string, IPricingEngine*> pricers;

    try
    {
        for (const auto& configItem : config)
        {
            const std::string& tradeType = configItem.getTradeType();
            const std::string& typeName = configItem.getTypeName();
            auto replacement = std::unique_ptr<IPricingEngine>(createPricingEngine(typeName));

            auto existing = pricers.find(tradeType);
            if (existing != pricers.end())
            {
                delete existing->second;
                existing->second = replacement.release();
            }
            else
            {
                pricers.emplace(tradeType, replacement.release());
            }
        }
    }
    catch (...)
    {
        for (auto& [fst, snd] : pricers)
        {
            delete snd;
        }
        throw;
    }

    return pricers;
}

std::map<std::string, IPricingEngine*> PricingEngineFactory::createPricersFromConfigFile(const std::string& configFile)
{
    PricingConfigLoader pricingConfigLoader;
    pricingConfigLoader.setConfigFile(configFile);
    const PricingEngineConfig config = pricingConfigLoader.loadConfig();

    return createPricersFromConfig(config);
}
