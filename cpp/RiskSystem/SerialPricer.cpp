#include "SerialPricer.h"

void SerialPricer::loadPricers() {
    if (!pricers_.empty()) {
        return;
    }

    pricers_ = PricingEngineFactory::createPricersFromConfigFile("./PricingConfig/PricingEngines.xml");
}

void SerialPricer::priceTrade(ITrade& trade, IScalarResultReceiver& resultReceiver) {
    const std::string tradeType = trade.getTradeType();
    if (pricers_.find(tradeType) == pricers_.end()) {
        resultReceiver.addError(trade.getTradeId(), "No Pricing Engines available for this trade type");
        return;
    }

    pricers_.at(tradeType)->price(&trade, &resultReceiver);
}

void SerialPricer::price(ITrade& trade, IScalarResultReceiver& resultReceiver) {
    loadPricers();
    priceTrade(trade, resultReceiver);
}

void SerialPricer::price(const std::vector<std::vector<std::unique_ptr<ITrade>>>& tradeContainers,
                         IScalarResultReceiver& resultReceiver) {
    loadPricers();

    for (const auto& tradeContainer : tradeContainers) {
        for (const auto& trade : tradeContainer) {
            priceTrade(*trade, resultReceiver);
        }
    }
}
