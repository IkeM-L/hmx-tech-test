#include "FxTradeLoader.h"

#include <fstream>
#include <stdexcept>

#include "Utils/TradeParsingUtils.h"

std::vector<ITrade*> FxTradeLoader::loadTrades()
{
    TradeParsingUtils::validateFileNotEmpty(dataFile_);

    std::ifstream stream(dataFile_);
    if (!stream.is_open())
    {
        throw std::runtime_error("Cannot open file: " + dataFile_);
    }

    std::vector<ITrade*> result;
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

        if (lineNumber == 1)
        {
            continue; // file header
        }

        if (lineNumber == 2)
        {
            continue; // column header
        }

        if (line.rfind("END", 0) == 0)
        {
            break; // trailer
        }

        // Important: separator is UTF-8 "¬", not a single char.
        const std::vector<std::string> items = TradeParsingUtils::splitLine(line, u8"¬");

        if (items.size() != 9)
        {
            throw std::runtime_error(
                "Invalid FX trade line format at line " +
                std::to_string(lineNumber) +
                ": expected 9 fields, got " +
                std::to_string(items.size()));
        }

        // 0 Type
        // 1 TradeDate
        // 2 Ccy1
        // 3 Ccy2
        // 4 Amount
        // 5 Rate
        // 6 ValueDate
        // 7 Counterparty
        // 8 TradeId

        const std::string& tradeType = items[0];
        const std::string& tradeId = items[8];

        FxTrade* trade = new FxTrade(tradeId, tradeType);

        try
        {
            trade->setTradeDate(TradeParsingUtils::parseDate(items[1]));
            trade->setInstrument(items[2] + items[3]); // concatenation, per spec
            trade->setNotional(std::stod(items[4]));
            trade->setRate(std::stod(items[5]));
            trade->setValueDate(TradeParsingUtils::parseDate(items[6]));
            trade->setCounterparty(items[7]);
        }
        catch (...)
        {
            delete trade;
            throw;
        }

        result.push_back(trade);
    }

    return result;
}

std::string FxTradeLoader::getDataFile() const
{
    return dataFile_;
}

void FxTradeLoader::setDataFile(const std::string& file)
{
    dataFile_ = file;
}