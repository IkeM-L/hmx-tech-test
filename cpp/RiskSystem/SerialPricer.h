#ifndef SERIALPRICER_H
#define SERIALPRICER_H

#include "../Models/ITrade.h"
#include "../Models/IScalarResultReceiver.h"
#include "PricingEngineFactory.h"
#include <map>
#include <memory>
#include <vector>

class SerialPricer {
private:
    PricingEngineFactory::PricingEngineMap pricers_;
    void loadPricers();
    
public:
    /// Destroys the pricer and releases owned pricing engines.
    ~SerialPricer() = default;
    /// Prices owned trade containers sequentially without taking ownership from the caller.
    void price(const std::vector<std::vector<std::unique_ptr<ITrade>>>& tradeContainers,
               IScalarResultReceiver& resultReceiver);
};

#endif // SERIALPRICER_H
