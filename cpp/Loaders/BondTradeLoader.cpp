#include "BondTradeLoader.h"
#include "Utils/TradeParsingUtils.h"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace
{
    void skipBom(std::string& line)
    {
        if (line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF)
        {
            line = line.substr(3);
        }
    }
}

BondTrade* BondTradeLoader::createTradeFromLine(const std::string& line)
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

    return trade.release();
}

void BondTradeLoader::forEachTrade(const std::function<void(ITrade*)>& tradeHandler) const
{
    TradeParsingUtils::validateFileNotEmpty(dataFile_);

    std::ifstream stream(dataFile_);
    if (!stream.is_open())
    {
        throw std::runtime_error("Cannot open file: " + dataFile_);
    }

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

std::vector<ITrade*> BondTradeLoader::loadTrades()
{
    std::vector<ITrade*> result;

    forEachTrade([&result](ITrade* trade)
    {
        result.push_back(trade);
    });

    return result;
}

void BondTradeLoader::streamTrades(const std::function<void(ITrade*)>& tradeHandler)
{
    forEachTrade(tradeHandler);
}

std::string BondTradeLoader::getDataFile() const
{
    return dataFile_;
}

void BondTradeLoader::setDataFile(const std::string& file)
{
    dataFile_ = file;
}
