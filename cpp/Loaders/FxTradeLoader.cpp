#include "FxTradeLoader.h"
#include "Utils/TradeParsingUtils.h"

#include <memory>
#include <stdexcept>
#include <vector>

namespace {
    constexpr std::size_t ExpectedFxFieldCount = 9;
    constexpr std::size_t TradeTypeIndex = 0;
    constexpr std::size_t TradeDateIndex = 1;
    constexpr std::size_t BaseCurrencyIndex = 2;
    constexpr std::size_t QuoteCurrencyIndex = 3;
    constexpr std::size_t NotionalIndex = 4;
    constexpr std::size_t RateIndex = 5;
    constexpr std::size_t ValueDateIndex = 6;
    constexpr std::size_t CounterpartyIndex = 7;
    constexpr std::size_t TradeIdIndex = 8;
    constexpr int FirstNonEmptyLineNumber = 1;
    constexpr int HeaderLineCount = 2;
    constexpr int StartsWithPosition = 0;
    constexpr char EndMarker[] = "END";
}

std::unique_ptr<FxTrade> FxTradeLoader::createTradeFromLine(
    const std::string& line,
    const int lineNumber)
{
    const std::vector<std::string> items = TradeParsingUtils::splitLine(line, "¬");

    if (items.size() != ExpectedFxFieldCount)
    {
        throw std::runtime_error(
            "Invalid FX trade line format at line " +
            std::to_string(lineNumber) +
            ": expected " +
            std::to_string(ExpectedFxFieldCount) +
            " fields, got " +
            std::to_string(items.size()));
    }

    const std::string& tradeType = items[TradeTypeIndex];
    const std::string& tradeId = items[TradeIdIndex];

    auto trade = std::make_unique<FxTrade>(tradeId, tradeType);

    trade->setTradeDate(TradeParsingUtils::parseDate(items[TradeDateIndex]));
    trade->setInstrument(items[BaseCurrencyIndex] + items[QuoteCurrencyIndex]);
    trade->setNotional(std::stod(items[NotionalIndex]));
    trade->setRate(std::stod(items[RateIndex]));
    trade->setValueDate(TradeParsingUtils::parseDate(items[ValueDateIndex]));
    trade->setCounterparty(items[CounterpartyIndex]);

    return trade;
}

void FxTradeLoader::forEachTrade(const TradeHandler& tradeHandler) const
{
    std::ifstream stream = openInputFile();

    std::string line;
    int lineNumber = 0;
    int nonEmptyLineNumber = 0;

    while (std::getline(stream, line))
    {
        ++lineNumber;
        line = TradeParsingUtils::trim(line);

        if (line.empty())
        {
            continue;
        }

        ++nonEmptyLineNumber;

        if (nonEmptyLineNumber == FirstNonEmptyLineNumber)
        {
            skipBom(line);
        }

        if (nonEmptyLineNumber <= HeaderLineCount)
        {
            continue;
        }

        if (line.rfind(EndMarker, StartsWithPosition) == StartsWithPosition)
        {
            break;
        }

        tradeHandler(createTradeFromLine(line, lineNumber));
    }
}
