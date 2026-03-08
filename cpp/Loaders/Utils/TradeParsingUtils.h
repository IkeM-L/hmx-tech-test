#ifndef TRADEPARSINGUTILS_H
#define TRADEPARSINGUTILS_H

#include <chrono>
#include <string>
#include <vector>

namespace TradeParsingUtils {
    /// Trims leading and trailing whitespace, including trailing CR/LF characters.
    std::string trim(std::string s);
    /// Parses a date in `YYYY-MM-DD` format.
    std::chrono::system_clock::time_point parseDate(const std::string& value);
    /// Splits a delimited line using a single-character separator.
    std::vector<std::string> splitLine(const std::string& line, char separator);
    /// Splits a delimited line using a string separator.
    std::vector<std::string> splitLine(const std::string& line, const std::string& separator);
    /// Throws if the provided filename is empty.
    void validateFileNotEmpty(const std::string& filename);
}

#endif // TRADEPARSINGUTILS_H
