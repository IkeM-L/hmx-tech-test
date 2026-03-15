#include "SerialPricer.h"

void SerialPricer::loadPricers() {
    if (!pricers_.empty()) {
        return;
    }

    pricers_ = PricingEngineFactory::createPricersFromConfigFile("./PricingConfig/PricingEngines.xml");
}

void SerialPricer::price(const std::vector<std::vector<std::unique_ptr<ITrade>>>& tradeContainers,
                         IScalarResultReceiver& resultReceiver) {
    loadPricers();

    for (const auto& tradeContainer : tradeContainers) {
        for (const auto& trade : tradeContainer) {
            std::string tradeType = trade->getTradeType();
            if (pricers_.find(tradeType) == pricers_.end()) {
                resultReceiver.addError(trade->getTradeId(), "No Pricing Engines available for this trade type");
                continue;
            }

            pricers_.at(tradeType)->price(trade.get(), &resultReceiver);
        }
    }
}
