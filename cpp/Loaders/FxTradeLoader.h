#ifndef FXTRADELOADER_H
#define FXTRADELOADER_H

#include "BaseTradeLoader.h"
#include "../Models/FxTrade.h"

#include <string>

class FxTradeLoader : public BaseTradeLoader
{
private:
    static std::unique_ptr<FxTrade> createTradeFromLine(const std::string& line, int lineNumber);
    void forEachTrade(const TradeHandler& tradeHandler) const override;

public:
    using BaseTradeLoader::loadTrades;
    using BaseTradeLoader::streamTrades;
    using BaseTradeLoader::getDataFile;
    using BaseTradeLoader::setDataFile;
};

#endif // FXTRADELOADER_H
