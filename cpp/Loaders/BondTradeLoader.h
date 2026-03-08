#ifndef BONDTRADELOADER_H
#define BONDTRADELOADER_H

#include "BaseTradeLoader.h"
#include "../Models/BondTrade.h"

#include <string>

class BondTradeLoader : public BaseTradeLoader
{
private:
    static constexpr char separator = ',';

    static std::unique_ptr<BondTrade> createTradeFromLine(const std::string& line);
    void forEachTrade(const TradeHandler& tradeHandler) const override;

public:
    using BaseTradeLoader::loadTrades;
    using BaseTradeLoader::streamTrades;
    using BaseTradeLoader::getDataFile;
    using BaseTradeLoader::setDataFile;
};

#endif // BONDTRADELOADER_H
