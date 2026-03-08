#ifndef BASETRADE_H
#define BASETRADE_H

#include "ITrade.h"
#include <chrono>
#include <string>

class BaseTrade : public ITrade {
public:
    /// Creates an empty trade shell for derived trade types.
    BaseTrade() = default;
    /// Destroys the trade instance.
    ~BaseTrade() override = default;
    
    /// Returns the trade date.
    [[nodiscard]] std::chrono::system_clock::time_point getTradeDate() const override { return tradeDate_; }
    /// Sets the trade date.
    void setTradeDate(const std::chrono::system_clock::time_point& date) override { tradeDate_ = date; }
    
    /// Returns the instrument identifier.
    [[nodiscard]] std::string getInstrument() const override { return instrument_; }
    /// Sets the instrument identifier.
    void setInstrument(const std::string& instrument) override { instrument_ = instrument; }
    
    /// Returns the trade counterparty.
    [[nodiscard]] std::string getCounterparty() const override { return counterparty_; }
    /// Sets the trade counterparty.
    void setCounterparty(const std::string& counterparty) override { counterparty_ = counterparty; }
    
    /// Returns the trade notional.
    [[nodiscard]] double getNotional() const override { return notional_; }
    /// Sets the trade notional.
    void setNotional(const double notional) override { notional_ = notional; }
    
    /// Returns the quoted trade rate or price.
    [[nodiscard]] double getRate() const override { return rate_; }
    /// Sets the quoted trade rate or price.
    void setRate(const double rate) override { rate_ = rate; }
    
    /// Returns the concrete trade type.
    [[nodiscard]] std::string getTradeType() const override = 0;
    /// Returns the trade identifier.
    [[nodiscard]] std::string getTradeId() const override { return tradeId_; }
    
protected:
    std::string tradeId_;
    
private:
    std::chrono::system_clock::time_point tradeDate_;
    std::string instrument_;
    std::string counterparty_;
    double notional_ = 0.0;
    double rate_ = 0.0;
};

#endif // BASETRADE_H
