#ifndef TRADELIST_H
#define TRADELIST_H

#include "ITrade.h"
#include "ITradeReceiver.h"
#include <vector>

class TradeList : public ITradeReceiver {
public:
    TradeList() = default;
    
    /// Legacy test helper that stores borrowed raw pointers from loader tests.
    /// Once the tests are updated, this should either own `std::unique_ptr`
    /// trades or be removed entirely.
    void add(ITrade* trade) override {
        trades_.push_back(trade);
    }
    
    [[nodiscard]] size_t size() const { return trades_.size(); }
    ITrade* operator[](size_t index) const { return trades_[index]; }
    
    using iterator = std::vector<ITrade*>::iterator;
    using const_iterator = std::vector<ITrade*>::const_iterator;
    
    iterator begin() { return trades_.begin(); }
    iterator end() { return trades_.end(); }
    [[nodiscard]] const_iterator begin() const { return trades_.begin(); }
    [[nodiscard]] const_iterator end() const { return trades_.end(); }
    
private:
    std::vector<ITrade*> trades_;
};

#endif // TRADELIST_H
