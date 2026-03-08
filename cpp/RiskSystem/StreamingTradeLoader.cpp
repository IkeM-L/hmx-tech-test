#include "StreamingTradeLoader.h"

#include "../Loaders/BondTradeLoader.h"
#include "../Loaders/FxTradeLoader.h"

#include <memory>

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

void StreamingTradeLoader::streamTrades(const std::function<void(ITrade*)>& tradeHandler) {
    const auto loaders = getTradeLoaders();
    for (const auto& loader : loaders) {
        loader->streamTrades(tradeHandler);
    }
}
