#ifndef TRADEPARSINGUTILS_H
#define TRADEPARSINGUTILS_H

#include <chrono>
#include <string>
#include <vector>

namespace TradeParsingUtils {
    std::string trim(std::string s);
    std::chrono::system_clock::time_point parseDate(const std::string& value);
    std::vector<std::string> splitLine(const std::string& line, char separator);
    std::vector<std::string> splitLine(const std::string& line, const std::string& separator);
    void validateFileNotEmpty(const std::string& filename);
}

#endif // TRADEPARSINGUTILS_H