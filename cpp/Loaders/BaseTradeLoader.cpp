#include "BaseTradeLoader.h"

#include "Utils/TradeParsingUtils.h"

#include <stdexcept>

const std::string& BaseTradeLoader::dataFile() const
{
    return dataFile_;
}

std::ifstream BaseTradeLoader::openInputFile() const
{
    TradeParsingUtils::validateFileNotEmpty(dataFile_);

    std::ifstream stream(dataFile_);
    if (!stream.is_open())
    {
        throw std::runtime_error("Cannot open file: " + dataFile_);
    }

    return stream;
}

void BaseTradeLoader::skipBom(std::string& line)
{
    if (line.size() >= 3 &&
        static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF)
    {
        line = line.substr(3);
    }
}

std::vector<ITrade*> BaseTradeLoader::loadTrades()
{
    std::vector<ITrade*> result;

    forEachTrade([&result](std::unique_ptr<ITrade> trade)
    {
        result.push_back(trade.release());
    });

    return result;
}

void BaseTradeLoader::streamTrades(const TradeHandler& tradeHandler)
{
    forEachTrade(tradeHandler);
}

std::string BaseTradeLoader::getDataFile() const
{
    return dataFile_;
}

void BaseTradeLoader::setDataFile(const std::string& file)
{
    dataFile_ = file;
}
