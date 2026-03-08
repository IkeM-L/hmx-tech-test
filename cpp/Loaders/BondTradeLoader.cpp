#include "BondTradeLoader.h"

#include <fstream>
#include <stdexcept>

#include "Utils/TradeParsingUtils.h"

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

    BondTrade* trade = new BondTrade(tradeId, tradeType);

    try
    {
        trade->setTradeDate(TradeParsingUtils::parseDate(items[1]));
        trade->setInstrument(items[2]);
        trade->setCounterparty(items[3]);
        trade->setNotional(std::stod(items[4]));
        trade->setRate(std::stod(items[5]));
    }
    catch (...)
    {
        delete trade;
        throw;
    }

    return trade;
}

void BondTradeLoader::loadTradesFromFile(const std::string& filename, BondTradeList& tradeList)
{
    TradeParsingUtils::validateFileNotEmpty(filename);

    std::ifstream stream(filename);
    if (!stream.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
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

            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF)
            {
                line = line.substr(3);
            }

            continue;
        }

        tradeList.add(createTradeFromLine(line));
    }
}

std::vector<ITrade*> BondTradeLoader::loadTrades()
{
    BondTradeList tradeList;
    loadTradesFromFile(dataFile_, tradeList);

    std::vector<ITrade*> result;
    for (std::size_t i = 0; i < tradeList.size(); ++i)
    {
        result.push_back(tradeList[i]);
    }

    return result;
}

std::string BondTradeLoader::getDataFile() const
{
    return dataFile_;
}

void BondTradeLoader::setDataFile(const std::string& file)
{
    dataFile_ = file;
}