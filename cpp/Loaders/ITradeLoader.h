#ifndef ITRADELOADER_H
#define ITRADELOADER_H

#include "../Models/ITrade.h"
#include <functional>
#include <string>
#include <vector>

class ITradeLoader {
public:
    virtual ~ITradeLoader() = default;

    virtual std::vector<ITrade*> loadTrades() = 0;
    virtual void streamTrades(const std::function<void(ITrade*)>& tradeHandler) = 0;
    [[nodiscard]] virtual std::string getDataFile() const = 0;
    virtual void setDataFile(const std::string& file) = 0;
};

#endif // ITRADELOADER_H