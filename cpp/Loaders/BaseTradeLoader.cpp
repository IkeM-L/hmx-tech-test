#include "BaseTradeLoader.h"

#include "Utils/TradeParsingUtils.h"

#include <stdexcept>

namespace {
    constexpr std::size_t Utf8BomLength = 3;
    constexpr unsigned char Utf8BomByte0 = 0xEF;
    constexpr unsigned char Utf8BomByte1 = 0xBB;
    constexpr unsigned char Utf8BomByte2 = 0xBF;
}

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
    // Some input files are UTF-8 encoded with a BOM on the first line. If we leave it in place,
    // the first token starts with BOM bytes and no longer matches the expected header/content.
    if (line.size() >= Utf8BomLength &&
        static_cast<unsigned char>(line[0]) == Utf8BomByte0 &&
        static_cast<unsigned char>(line[1]) == Utf8BomByte1 &&
        static_cast<unsigned char>(line[2]) == Utf8BomByte2)
    {
        line = line.substr(Utf8BomLength);
    }
}

std::vector<ITrade*> BaseTradeLoader::loadTrades()
{
    std::vector<std::unique_ptr<ITrade>> ownedTrades;

    forEachTrade([&ownedTrades](std::unique_ptr<ITrade> trade)
    {
        ownedTrades.push_back(std::move(trade));
    });

    std::vector<ITrade*> result;
    result.reserve(ownedTrades.size());
    for (auto& trade : ownedTrades)
    {
        result.push_back(trade.release());
    }
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
