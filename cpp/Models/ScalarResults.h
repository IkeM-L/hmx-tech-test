#ifndef SCALARRESULTS_H
#define SCALARRESULTS_H

#include "IScalarResultReceiver.h"
#include "ScalarResult.h"
#include <map>
#include <vector>
#include <optional>
#include <string>
#include <iterator>

class ScalarResults : public IScalarResultReceiver {
public:
    ~ScalarResults() override;
    std::optional<ScalarResult> operator[](const std::string& tradeId) const;

    bool containsTrade(const std::string& tradeId) const;

    void addResult(const std::string& tradeId, double result) override;

    void addError(const std::string& tradeId, const std::string& error) override;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ScalarResult;
        using difference_type = std::ptrdiff_t;
        using pointer = ScalarResult*;
        using reference = ScalarResult&;

        Iterator() = default;
        Iterator(const ScalarResults* parent,
                 std::vector<std::string> tradeIds,
                 std::size_t index);

        Iterator& operator++();
        ScalarResult operator*() const;
        bool operator!=(const Iterator& other) const;

    private:
        const ScalarResults* parent_ = nullptr;
        std::vector<std::string> tradeIds_;
        std::size_t index_ = 0;
    };

    [[nodiscard]] Iterator begin() const;
    [[nodiscard]] Iterator end() const;

private:
    std::map<std::string, double> results_;
    std::map<std::string, std::string> errors_;
};

#endif // SCALARRESULTS_H