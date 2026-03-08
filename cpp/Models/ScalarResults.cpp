#include "ScalarResults.h"

#include <set>
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

ScalarResults::Iterator::Iterator(const ScalarResults* parent, std::shared_ptr<const std::vector<std::string>> tradeIds, const std::size_t index)
    : parent_(parent), tradeIds_(std::move(tradeIds)), index_(index) {
}

ScalarResults::Iterator& ScalarResults::Iterator::operator++() {
    if (tradeIds_ != nullptr && index_ < tradeIds_->size()) {
        ++index_;
    }
    return *this;
}

ScalarResult ScalarResults::Iterator::operator*() const {
    if (parent_ == nullptr || tradeIds_ == nullptr || index_ >= tradeIds_->size()) {
        throw std::out_of_range("Iterator cannot be dereferenced");
    }

    const std::string& tradeId = (*tradeIds_)[index_];
    std::optional<ScalarResult> result = (*parent_)[tradeId];

    if (!result.has_value()) {
        throw std::runtime_error("Iterator points to invalid trade ID");
    }

    return *result;
}

bool ScalarResults::Iterator::operator!=(const Iterator& other) const {
    if (other.parent_ == nullptr && other.tradeIds_ == nullptr) {
        return parent_ != nullptr && tradeIds_ != nullptr && index_ < tradeIds_->size();
    }

    return parent_ != other.parent_ ||
           index_ != other.index_ ||
           tradeIds_ != other.tradeIds_;
}

std::shared_ptr<const std::vector<std::string>> ScalarResults::buildTradeIdSnapshot() const {
    std::set<std::string> uniqueTradeIds;

    for (const auto& [fst, snd] : results_) {
        uniqueTradeIds.insert(fst);
    }

    for (const auto& [fst, snd] : errors_) {
        uniqueTradeIds.insert(fst);
    }

    return std::make_shared<const std::vector<std::string>>(uniqueTradeIds.begin(), uniqueTradeIds.end());
}

ScalarResults::Iterator ScalarResults::begin() const {
    // The iterator walks a stable snapshot so range-based for loops are not tied
    // to whichever backing map currently contains a given trade ID.
    return Iterator(this, buildTradeIdSnapshot(), 0);
}

ScalarResults::Iterator ScalarResults::end() const {
    return Iterator();
}
