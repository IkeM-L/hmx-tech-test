#ifndef SERIALPRICER_H
#define SERIALPRICER_H

#include "../Models/IPricingEngine.h"
#include "../Models/ITrade.h"
#include "../Models/IScalarResultReceiver.h"
#include "PricingConfigLoader.h"
#include <map>
#include <vector>
#include <string>

class SerialPricer {
private:
    std::map<std::string, IPricingEngine*> pricers_;
    void loadPricers();
    
public:
    /// Destroys the pricer and releases owned pricing engines.
    ~SerialPricer();
    /// Prices each trade sequentially using the configured pricing engines.
    void price(const std::vector<std::vector<ITrade*>>& tradeContainers, 
               IScalarResultReceiver* resultReceiver);
};

#endif // SERIALPRICER_H
