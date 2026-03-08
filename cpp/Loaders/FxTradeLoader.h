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

    static std::unique_ptr<FxTrade> createTradeFromLine(const std::string& line, int lineNumber);
    void forEachTrade(const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler) const;

public:
    /// Legacy batch API kept because the current FX-loader tests still use it.
    /// Once the tests are updated, this should either return `std::unique_ptr`
    /// ownership or be removed entirely.
    std::vector<ITrade*> loadTrades() override;

    /// Streams FX trades one at a time to the supplied callback.
    void streamTrades(const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler) override;

    /// Returns the configured FX trade file path.
    [[nodiscard]] std::string getDataFile() const override;

    /// Sets the FX trade file path.
    void setDataFile(const std::string& file) override;
};

#endif // FXTRADELOADER_H
