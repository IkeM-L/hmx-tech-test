#include "FxTradeLoader.h"
#include "Utils/TradeParsingUtils.h"

#include <memory>
#include <stdexcept>
#include <vector>

std::unique_ptr<FxTrade> FxTradeLoader::createTradeFromLine(
    const std::string& line,
    const int lineNumber)
{
    const std::vector<std::string> items = TradeParsingUtils::splitLine(line, "¬");

    if (items.size() != 9)
    {
        throw std::runtime_error(
            "Invalid FX trade line format at line " +
            std::to_string(lineNumber) +
            ": expected 9 fields, got " +
            std::to_string(items.size()));
    }

    const std::string& tradeType = items[0];
    const std::string& tradeId = items[8];

    auto trade = std::make_unique<FxTrade>(tradeId, tradeType);

    trade->setTradeDate(TradeParsingUtils::parseDate(items[1]));
    trade->setInstrument(items[2] + items[3]);
    trade->setNotional(std::stod(items[4]));
    trade->setRate(std::stod(items[5]));
    trade->setValueDate(TradeParsingUtils::parseDate(items[6]));
    trade->setCounterparty(items[7]);

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

        if (nonEmptyLineNumber == 1)
        {
            skipBom(line);
        }

        if (nonEmptyLineNumber <= 2)
        {
            continue;
        }

        if (line.rfind("END", 0) == 0)
        {
            break;
        }

        tradeHandler(createTradeFromLine(line, lineNumber));
    }
}
