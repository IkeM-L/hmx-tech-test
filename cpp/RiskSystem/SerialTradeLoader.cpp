#include "SerialTradeLoader.h"
#include "../Loaders/BondTradeLoader.h"
#include "../Loaders/FxTradeLoader.h"

std::vector<std::unique_ptr<ITradeLoader>> SerialTradeLoader::getTradeLoaders() {
    std::vector<std::unique_ptr<ITradeLoader>> loaders;
    
    auto bondLoader = std::make_unique<BondTradeLoader>();
    bondLoader->setDataFile("TradeData/BondTrades.dat");
    loaders.push_back(std::move(bondLoader));
    
    auto fxLoader = std::make_unique<FxTradeLoader>();
    fxLoader->setDataFile("TradeData/FxTrades.dat");
    loaders.push_back(std::move(fxLoader));
    
    return loaders;
}

std::vector<std::vector<std::unique_ptr<ITrade>>> SerialTradeLoader::loadTrades() {
    const auto loaders = getTradeLoaders();
    std::vector<std::vector<std::unique_ptr<ITrade>>> result;
    
    for (const auto& loader : loaders) {
        auto& tradeContainer = result.emplace_back();
        loader->streamTrades([&tradeContainer](std::unique_ptr<ITrade> trade) {
            tradeContainer.push_back(std::move(trade));
        });
    }
    
    return result;
}
