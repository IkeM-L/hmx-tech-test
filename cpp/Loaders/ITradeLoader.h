#ifndef ITRADELOADER_H
#define ITRADELOADER_H

#include "../Models/ITrade.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class ITradeLoader {
public:
    /// Destroys the loader instance.
    virtual ~ITradeLoader() = default;

    /// Loads all trades into memory and returns raw pointers owned by the caller.
    virtual std::vector<ITrade*> loadTrades() = 0;

    /// Streams trades one at a time, transferring ownership to the callback.
    virtual void streamTrades(const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler) = 0;

    /// Returns the configured input file path.
    [[nodiscard]] virtual std::string getDataFile() const = 0;

    /// Sets the input file path used by the loader.
    virtual void setDataFile(const std::string& file) = 0;
};

#endif // ITRADELOADER_H
