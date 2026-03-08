#include "FxTradeLoader.h"
#include "Utils/TradeParsingUtils.h"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

std::unique_ptr<FxTrade> FxTradeLoader::createTradeFromLine(
    const std::string& line,
    const int lineNumber)
{
    const std::vector<std::string> items = TradeParsingUtils::splitLine(line, u8"¬");

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

void FxTradeLoader::forEachTrade(
    const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler) const
{
    TradeParsingUtils::validateFileNotEmpty(dataFile_);

    std::ifstream stream(dataFile_);
    if (!stream.is_open())
    {
        throw std::runtime_error("Cannot open file: " + dataFile_);
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(stream, line))
    {
        ++lineNumber;
        line = TradeParsingUtils::trim(line);

        if (line.empty())
        {
            continue;
        }

        if (lineNumber == 1 || lineNumber == 2)
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

std::vector<ITrade*> FxTradeLoader::loadTrades()
{
    std::vector<ITrade*> result;

    forEachTrade([&result](std::unique_ptr<ITrade> trade)
    {
        result.push_back(trade.release());
    });

    return result;
}

void FxTradeLoader::streamTrades(
    const std::function<void(std::unique_ptr<ITrade>)>& tradeHandler)
{
    forEachTrade(tradeHandler);
}

std::string FxTradeLoader::getDataFile() const
{
    return dataFile_;
}

void FxTradeLoader::setDataFile(const std::string& file)
{
    dataFile_ = file;
}
