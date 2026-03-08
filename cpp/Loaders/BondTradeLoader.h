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

    static std::unique_ptr<BondTrade> createTradeFromLine(const std::string& line);
    void forEachTrade(const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler) const;

public:
    /// Legacy batch API kept because the current bond-loader tests still use it.
    /// Once the tests are updated, this should either return `std::unique_ptr`
    /// ownership or be removed entirely.
    std::vector<ITrade*> loadTrades() override;

    /// Streams bond trades one at a time to the supplied callback.
    void streamTrades(const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler) override;

    /// Returns the configured bond trade file path.
    [[nodiscard]] std::string getDataFile() const override;

    /// Sets the bond trade file path.
    void setDataFile(const std::string& file) override;
};

#endif // BONDTRADELOADER_H
