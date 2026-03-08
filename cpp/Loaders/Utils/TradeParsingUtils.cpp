#include "TradeParsingUtils.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace TradeParsingUtils {

    std::string trim(std::string s)
    {
        while (!s.empty() &&
               (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
        {
            s.pop_back();
        }

        std::size_t start = 0;
        while (start < s.size() && (s[start] == ' ' || s[start] == '\t'))
        {
            ++start;
        }

        return s.substr(start);
    }

    std::chrono::system_clock::time_point parseDate(const std::string& value)
    {
        std::tm tm = {};
        std::istringstream ss(value);
        ss >> std::get_time(&tm, "%Y-%m-%d");

        if (ss.fail())
        {
            throw std::runtime_error("Invalid date: " + value);
        }

        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }

    std::vector<std::string> splitLine(const std::string& line, char separator)
    {
        std::vector<std::string> items;
        std::stringstream ss(line);
        std::string item;

        while (std::getline(ss, item, separator))
        {
            items.push_back(trim(item));
        }

        return items;
    }

    std::vector<std::string> splitLine(const std::string& line, const std::string& separator)
    {
        std::vector<std::string> items;

        if (separator.empty())
        {
            items.push_back(trim(line));
            return items;
        }

        std::size_t start = 0;
        while (true)
        {
            std::size_t pos = line.find(separator, start);
            if (pos == std::string::npos)
            {
                items.push_back(trim(line.substr(start)));
                break;
            }

            items.push_back(trim(line.substr(start, pos - start)));
            start = pos + separator.size();
        }

        return items;
    }

    void validateFileNotEmpty(const std::string& filename)
    {
        if (filename.empty())
        {
            throw std::invalid_argument("Filename cannot be empty");
        }
    }

} // namespace TradeParsingUtils