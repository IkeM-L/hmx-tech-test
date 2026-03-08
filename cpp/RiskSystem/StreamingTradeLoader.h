#ifndef STREAMINGTRADELOADER_H
#define STREAMINGTRADELOADER_H

#include "../Loaders/ITradeLoader.h"
#include <functional>
#include <memory>
#include <vector>

class StreamingTradeLoader {
private:
    static std::vector<std::unique_ptr<ITradeLoader>> getTradeLoaders();
    
public:
    /// Streams trades from all configured sources, transferring ownership to the callback.
    static void streamTrades(const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler);
};

#endif // STREAMINGTRADELOADER_H
