#include "SerialPricer.h"
#include "PricingEngineFactory.h"

#include <stdexcept>

SerialPricer::~SerialPricer() {
    for (auto& entry : pricers_) {
        delete entry.second;
    }
    pricers_.clear();
}

void SerialPricer::loadPricers() {
    if (!pricers_.empty()) {
        return;
    }

    pricers_ = PricingEngineFactory::createPricersFromConfigFile("./PricingConfig/PricingEngines.xml");
}

void SerialPricer::price(const std::vector<std::vector<ITrade*>>& tradeContainers,
                         IScalarResultReceiver* resultReceiver) {
    loadPricers();

    for (const auto& tradeContainer : tradeContainers) {
        for (ITrade* trade : tradeContainer) {
            std::string tradeType = trade->getTradeType();
            if (pricers_.find(tradeType) == pricers_.end()) {
                resultReceiver->addError(trade->getTradeId(), "No Pricing Engines available for this trade type");
                continue;
            }

            IPricingEngine* pricer = pricers_[tradeType];
            pricer->price(trade, resultReceiver);
        }
    }
}