#ifndef FXTRADELOADER_H
#define FXTRADELOADER_H

#include "ITradeLoader.h"
#include "../Models/FxTrade.h"

#include <functional>
#include <string>
#include <vector>

class FxTradeLoader : public ITradeLoader
{
private:
    std::string dataFile_;

    static FxTrade* createTradeFromLine(const std::string& line, int lineNumber);
    void forEachTrade(const std::function<void(ITrade*)>& tradeHandler) const;

public:
    std::vector<ITrade*> loadTrades() override;
    void streamTrades(const std::function<void(ITrade*)>& tradeHandler) override;
    [[nodiscard]] std::string getDataFile() const override;
    void setDataFile(const std::string& file) override;
};

#endif // FXTRADELOADER_H