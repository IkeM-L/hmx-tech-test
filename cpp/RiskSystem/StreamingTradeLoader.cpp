#include "StreamingTradeLoader.h"

#include "../Loaders/BondTradeLoader.h"
#include "../Loaders/FxTradeLoader.h"
#include "PricingEngineFactory.h"

#include <memory>
#include <stdexcept>

std::vector<std::unique_ptr<ITradeLoader>> StreamingTradeLoader::getTradeLoaders() {
    std::vector<std::unique_ptr<ITradeLoader>> loaders;

    auto bondLoader = std::make_unique<BondTradeLoader>();
    bondLoader->setDataFile("TradeData/BondTrades.dat");
    loaders.push_back(std::move(bondLoader));

    auto fxLoader = std::make_unique<FxTradeLoader>();
    fxLoader->setDataFile("TradeData/FxTrades.dat");
    loaders.push_back(std::move(fxLoader));

    return loaders;
}

void StreamingTradeLoader::loadPricers() {
    if (!pricers_.empty()) {
        return;
    }

    pricers_ = PricingEngineFactory::createPricersFromConfigFile("./PricingConfig/PricingEngines.xml");
}

StreamingTradeLoader::~StreamingTradeLoader() {
    for (const auto& entry : pricers_) {
        delete entry.second;
    }
    pricers_.clear();
}

void StreamingTradeLoader::loadAndPrice(IScalarResultReceiver* resultReceiver) {
    if (resultReceiver == nullptr) {
        throw std::invalid_argument("resultReceiver");
    }

    loadPricers();
    auto loaders = getTradeLoaders();
    for (const auto& loader : loaders) {
        loader->streamTrades([this, resultReceiver](ITrade* trade) {
            std::unique_ptr<ITrade> tradeOwner(trade);
            const std::string tradeType = tradeOwner->getTradeType();

            if (const auto it = pricers_.find(tradeType); it == pricers_.end()) {
                resultReceiver->addError(
                    tradeOwner->getTradeId(),
                    "No Pricing Engines available for this trade type");
            } else {
                it->second->price(tradeOwner.get(), resultReceiver);
            }
        });
    }
}
