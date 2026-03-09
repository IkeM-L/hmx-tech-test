#ifndef TRADELIST_H
#define TRADELIST_H

#include "ITrade.h"
#include "ITradeReceiver.h"
#include <memory>
#include <vector>

class TradeList : public ITradeReceiver {
public:
    TradeList() = default;
    ~TradeList() override = default;

    TradeList(const TradeList&) = delete;
    TradeList& operator=(const TradeList&) = delete;
    TradeList(TradeList&&) = default;
    TradeList& operator=(TradeList&&) = default;
    
    /// Legacy test helper that takes ownership of raw pointers from loader tests.
    /// Once the tests are updated, this should either own `std::unique_ptr`
    /// trades directly or be removed entirely.
    void add(ITrade* trade) override {
        trades_.push_back(std::unique_ptr<ITrade>(trade));
    }
    
    [[nodiscard]] size_t size() const { return trades_.size(); }
    ITrade* operator[](size_t index) const { return trades_[index].get(); }
    
    class iterator {
    public:
        using underlying_iterator = std::vector<std::unique_ptr<ITrade>>::iterator;
        using iterator_category = std::forward_iterator_tag;
        using value_type = ITrade*;
        using difference_type = std::ptrdiff_t;
        using pointer = ITrade**;
        using reference = ITrade*&;

        explicit iterator(const underlying_iterator current) : current_(current) {}

        iterator& operator++() {
            ++current_;
            return *this;
        }

        bool operator==(const iterator& other) const { return current_ == other.current_; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
        ITrade* operator*() const { return current_->get(); }

    private:
        underlying_iterator current_;
    };

    class const_iterator {
    public:
        using underlying_iterator = std::vector<std::unique_ptr<ITrade>>::const_iterator;
        using iterator_category = std::forward_iterator_tag;
        using value_type = const ITrade*;
        using difference_type = std::ptrdiff_t;
        using pointer = const ITrade**;
        using reference = const ITrade*&;

        explicit const_iterator(underlying_iterator current) : current_(current) {}

        const_iterator& operator++() {
            ++current_;
            return *this;
        }

        bool operator==(const const_iterator& other) const { return current_ == other.current_; }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }
        const ITrade* operator*() const { return current_->get(); }

    private:
        underlying_iterator current_;
    };
    
    iterator begin() { return iterator(trades_.begin()); }
    iterator end() { return iterator(trades_.end()); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(trades_.begin()); }
    [[nodiscard]] const_iterator end() const { return const_iterator(trades_.end()); }
    
private:
    std::vector<std::unique_ptr<ITrade>> trades_;
};

#endif // TRADELIST_H
