#include "TestFramework.h"

#include "BondTradeLoaderTests.cpp"
#include "FxTradeLoaderTests.cpp"
#include "PricingConfigLoaderTests.cpp"
#include "PricingEngineTests.cpp"
#include "ScalarResultsTests.cpp"

int main() {
    TestRunner::runAll();
    return 0;
}
