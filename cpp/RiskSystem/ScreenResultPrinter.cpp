#include "ScreenResultPrinter.h"
#include <iostream>

void ScreenResultPrinter::printResults(const ScalarResults& results) {
    for (const auto& result : results) {
        const std::string& tradeId = result.getTradeId();
        const auto& price = result.getResult();
        const auto& error = result.getError();

        std::cout << tradeId;

        if (price.has_value()) {
            std::cout << " : " << price.value();
        }

        if (error.has_value()) {
            std::cout << " : " << error.value();
        }

        std::cout << std::endl;
    }
}