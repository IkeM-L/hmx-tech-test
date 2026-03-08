#ifndef BONDTRADELOADER_H
#define BONDTRADELOADER_H

#include "ITradeLoader.h"
#include "../Models/BondTrade.h"

#include <functional>
#include <string>
#include <vector>

class BondTradeLoader : public ITradeLoader
{
private:
    static constexpr char separator = ',';
    std::string dataFile_;

    static BondTrade* createTradeFromLine(const std::string& line);
    void forEachTrade(const std::function<void(ITrade*)>& tradeHandler) const;

public:
    std::vector<ITrade*> loadTrades() override;
    void streamTrades(const std::function<void(ITrade*)>& tradeHandler) override;
    [[nodiscard]] std::string getDataFile() const override;
    void setDataFile(const std::string& file) override;
};

#endif // BONDTRADELOADER_H