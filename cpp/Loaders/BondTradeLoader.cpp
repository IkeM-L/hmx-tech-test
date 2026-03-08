#include "BondTradeLoader.h"
#include "Utils/TradeParsingUtils.h"

#include <memory>
#include <stdexcept>
#include <vector>

std::unique_ptr<BondTrade> BondTradeLoader::createTradeFromLine(const std::string& line)
{
    const std::vector<std::string> items = TradeParsingUtils::splitLine(line, separator);

    if (items.size() != 7)
    {
        throw std::runtime_error(
            "Invalid bond trade line format: expected 7 fields, got " +
            std::to_string(items.size()));
    }

    const std::string& tradeType = items[0];
    const std::string& tradeId = items[6];

    auto trade = std::make_unique<BondTrade>(tradeId, tradeType);

    trade->setTradeDate(TradeParsingUtils::parseDate(items[1]));
    trade->setInstrument(items[2]);
    trade->setCounterparty(items[3]);
    trade->setNotional(std::stod(items[4]));
    trade->setRate(std::stod(items[5]));

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
