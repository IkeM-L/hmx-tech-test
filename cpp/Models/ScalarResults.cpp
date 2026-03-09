#include "ScalarResults.h"

#include <map>
#include <stdexcept>

ScalarResults::~ScalarResults() = default;

std::optional<ScalarResult> ScalarResults::operator[](const std::string& tradeId) const {
    if (!containsTrade(tradeId)) {
        return std::nullopt;
    }

    std::optional<double> priceResult = std::nullopt;
    std::optional<std::string> error = std::nullopt;

    if (const auto resultIt = results_.find(tradeId); resultIt != results_.end()) {
        priceResult = resultIt->second;
    }

    if (const auto errorIt = errors_.find(tradeId); errorIt != errors_.end()) {
        error = errorIt->second;
    }

    return ScalarResult(tradeId, priceResult, error);
}

bool ScalarResults::containsTrade(const std::string& tradeId) const {
    return results_.find(tradeId) != results_.end() || errors_.find(tradeId) != errors_.end();
}

void ScalarResults::addResult(const std::string& tradeId, double result) {
    results_[tradeId] = result;
}

void ScalarResults::addError(const std::string& tradeId, const std::string& error) {
    errors_[tradeId] = error;
}

ScalarResults::Iterator::Iterator(
    const ScalarResults* parent,
    std::shared_ptr<const std::vector<ScalarResult>> snapshot,
    const std::size_t index)
    : parent_(parent), snapshot_(std::move(snapshot)), index_(index) {
}

ScalarResults::Iterator& ScalarResults::Iterator::operator++() {
    if (snapshot_ != nullptr && index_ < snapshot_->size()) {
        ++index_;
    }
    return *this;
}

ScalarResults::Iterator::reference ScalarResults::Iterator::operator*() const {
    if (parent_ == nullptr || snapshot_ == nullptr || index_ >= snapshot_->size()) {
        throw std::out_of_range("Iterator cannot be dereferenced");
    }

    return (*snapshot_)[index_];
}

ScalarResults::Iterator::pointer ScalarResults::Iterator::operator->() const {
    if (parent_ == nullptr || snapshot_ == nullptr || index_ >= snapshot_->size()) {
        throw std::out_of_range("Iterator cannot be dereferenced");
    }

    return &(*snapshot_)[index_];
}

bool ScalarResults::Iterator::operator==(const Iterator& other) const {
    const bool thisIsEnd = snapshot_ == nullptr || index_ >= snapshot_->size();

    if (const bool otherIsEnd = other.snapshot_ == nullptr || other.index_ >= other.snapshot_->size(); thisIsEnd || otherIsEnd) {
        return thisIsEnd == otherIsEnd;
    }

    return parent_ == other.parent_ && index_ == other.index_;
}

bool ScalarResults::Iterator::operator!=(const Iterator& other) const {
    return !(*this == other);
}

std::shared_ptr<const std::vector<ScalarResult>> ScalarResults::buildSnapshot() const {
    std::map<std::string, std::pair<std::optional<double>, std::optional<std::string>>> mergedResults;

    for (const auto& [fst, snd] : results_) {
        mergedResults[fst].first = snd;
    }

    for (const auto& [fst, snd] : errors_) {
        mergedResults[fst].second = snd;
    }

    std::vector<ScalarResult> snapshot;
    snapshot.reserve(mergedResults.size());

    for (const auto& [tradeId, resultData] : mergedResults) {
        snapshot.emplace_back(tradeId, resultData.first, resultData.second);
    }

    return std::make_shared<const std::vector<ScalarResult>>(std::move(snapshot));
}

ScalarResults::Iterator ScalarResults::begin() const {
    // The iterator walks an immutable snapshot so copied iterators remain valid
    // and comparisons are based on logical position rather than snapshot identity.
    return Iterator(this, buildSnapshot(), 0);
}

ScalarResults::Iterator ScalarResults::end()
{
    return Iterator();
}
