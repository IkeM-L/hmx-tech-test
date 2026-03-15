#include "BondTradeLoader.h"
#include "Utils/TradeParsingUtils.h"

#include <memory>
#include <stdexcept>
#include <vector>

namespace {
    constexpr std::size_t ExpectedBondFieldCount = 7;
    constexpr std::size_t TradeTypeIndex = 0;
    constexpr std::size_t TradeDateIndex = 1;
    constexpr std::size_t InstrumentIndex = 2;
    constexpr std::size_t CounterpartyIndex = 3;
    constexpr std::size_t NotionalIndex = 4;
    constexpr std::size_t RateIndex = 5;
    constexpr std::size_t TradeIdIndex = 6;
}

std::unique_ptr<BondTrade> BondTradeLoader::createTradeFromLine(const std::string& line)
{
    const std::vector<std::string> items = TradeParsingUtils::splitLine(line, separator);

    if (items.size() != ExpectedBondFieldCount)
    {
        throw std::runtime_error(
            "Invalid bond trade line format: expected " +
            std::to_string(ExpectedBondFieldCount) +
            " fields, got " +
            std::to_string(items.size()));
    }

    const std::string& tradeType = items[TradeTypeIndex];
    const std::string& tradeId = items[TradeIdIndex];

    auto trade = std::make_unique<BondTrade>(tradeId, tradeType);

    trade->setTradeDate(TradeParsingUtils::parseDate(items[TradeDateIndex]));
    trade->setInstrument(items[InstrumentIndex]);
    trade->setCounterparty(items[CounterpartyIndex]);
    trade->setNotional(std::stod(items[NotionalIndex]));
    trade->setRate(std::stod(items[RateIndex]));

    return trade;
}

void BondTradeLoader::forEachTrade(const TradeHandler& tradeHandler) const
{
    std::ifstream stream = openInputFile();

    std::string line;
    bool firstLine = true;

    while (std::getline(stream, line))
    {
        line = TradeParsingUtils::trim(line);

        if (line.empty())
        {
            continue;
        }

        if (firstLine)
        {
            firstLine = false;
            skipBom(line);
            continue; // skip first non-empty line as header
        }

        tradeHandler(createTradeFromLine(line));
    }
}
