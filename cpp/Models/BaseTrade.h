#ifndef BASETRADE_H
#define BASETRADE_H

#include "ITrade.h"
#include <chrono>
#include <string>

class BaseTrade : public ITrade {
public:
    BaseTrade() = default;
    ~BaseTrade() override = default;
    
    [[nodiscard]] std::chrono::system_clock::time_point getTradeDate() const override { return tradeDate_; }
    void setTradeDate(const std::chrono::system_clock::time_point& date) override { tradeDate_ = date; }
    
    [[nodiscard]] std::string getInstrument() const override { return instrument_; }
    void setInstrument(const std::string& instrument) override { instrument_ = instrument; }
    
    [[nodiscard]] std::string getCounterparty() const override { return counterparty_; }
    void setCounterparty(const std::string& counterparty) override { counterparty_ = counterparty; }
    
    [[nodiscard]] double getNotional() const override { return notional_; }
    void setNotional(const double notional) override { notional_ = notional; }
    
    [[nodiscard]] double getRate() const override { return rate_; }
    void setRate(const double rate) override { rate_ = rate; }
    
    [[nodiscard]] std::string getTradeType() const override = 0;
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

