#ifndef SCALARRESULTS_H
#define SCALARRESULTS_H

#include "IScalarResultReceiver.h"
#include "ScalarResult.h"
#include <map>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class ScalarResults : public IScalarResultReceiver {
public:
    /// Destroys the result container.
    ~ScalarResults() override;

    /// Looks up the result and/or error recorded for a trade ID.
    std::optional<ScalarResult> operator[](const std::string& tradeId) const;

    /// Returns true when either a result or an error exists for the trade.
    [[nodiscard]] bool containsTrade(const std::string& tradeId) const;

    /// Records a scalar pricing result for a trade.
    void addResult(const std::string& tradeId, double result) override;

    /// Records an error message for a trade.
    void addError(const std::string& tradeId, const std::string& error) override;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ScalarResult;
        using difference_type = std::ptrdiff_t;
        using pointer = const ScalarResult*;
        using reference = const ScalarResult&;

        /// Creates a default end iterator.
        Iterator() = default;

        /// Creates an iterator over an immutable snapshot of results.
        Iterator(const ScalarResults* parent,
                 std::shared_ptr<const std::vector<ScalarResult>> snapshot,
                 std::size_t index);

        /// Advances the iterator to the next result.
        Iterator& operator++();
        /// Returns the current scalar result.
        reference operator*() const;
        /// Returns the current scalar result pointer.
        pointer operator->() const;
        /// Compares two iterators for equality.
        bool operator==(const Iterator& other) const;
        /// Compares two iterators for inequality.
        bool operator!=(const Iterator& other) const;

    private:
        const ScalarResults* parent_ = nullptr;
        std::shared_ptr<const std::vector<ScalarResult>> snapshot_;
        std::size_t index_ = 0;
    };

    /// Returns an iterator to the first stored result.
    [[nodiscard]] Iterator begin() const;
    /// Returns an iterator one past the last stored result.
    [[nodiscard]] static Iterator end();

private:
    [[nodiscard]] std::shared_ptr<const std::vector<ScalarResult>> buildSnapshot() const;

    std::map<std::string, double> results_;
    std::map<std::string, std::string> errors_;
};

#endif // SCALARRESULTS_H
