#ifndef ITRADERECEIVER_H
#define ITRADERECEIVER_H

#include "ITrade.h"

class ITradeReceiver {
public:
    virtual ~ITradeReceiver() = default;
    /// Legacy raw-pointer receiver retained for the current loader tests.
    /// Once the tests are updated, this should either move to `std::unique_ptr`
    /// ownership or be removed entirely.
    virtual void add(ITrade* trade) = 0;
};

#endif // ITRADERECEIVER_H
