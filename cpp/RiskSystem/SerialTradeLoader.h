#ifndef SERIALTRADELOADER_H
#define SERIALTRADELOADER_H

#include "../Loaders/ITradeLoader.h"
#include "../Models/ITrade.h"
#include <memory>
#include <vector>

class SerialTradeLoader {
private:
    static std::vector<std::unique_ptr<ITradeLoader>> getTradeLoaders();
    
public:
    static std::vector<std::vector<std::unique_ptr<ITrade>>> loadTrades();
};

#endif // SERIALTRADELOADER_H
