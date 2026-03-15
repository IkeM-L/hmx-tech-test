#ifndef BASETRADELOADER_H
#define BASETRADELOADER_H

#include "ITradeLoader.h"

#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class BaseTradeLoader : public ITradeLoader
{
protected:
    using TradeHandler = std::function<void(std::unique_ptr<ITrade>)>;

    [[nodiscard]] const std::string& dataFile() const;
    [[nodiscard]] std::ifstream openInputFile() const;
    // Sample trade files may start with a UTF-8 BOM; strip it so header checks and parsing see the real text.
    static void skipBom(std::string& line);

    virtual void forEachTrade(const TradeHandler& tradeHandler) const = 0;

public:
    std::vector<ITrade*> loadTrades() override;
    void streamTrades(const TradeHandler& tradeHandler) override;
    [[nodiscard]] std::string getDataFile() const override;
    void setDataFile(const std::string& file) override;

private:
    std::string dataFile_;
};

#endif // BASETRADELOADER_H
