#ifndef PARALLELPRICER_H
#define PARALLELPRICER_H

#include "../Models/IPricingEngine.h"
#include "../Models/ITrade.h"
#include "../Models/IScalarResultReceiver.h"
#include <condition_variable>
#include <deque>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class ParallelPricer {
private:
    class LockedScalarResultReceiver : public IScalarResultReceiver {
    public:
        explicit LockedScalarResultReceiver(IScalarResultReceiver* inner)
            : inner_(inner)
        {
        }

        void addResult(const std::string& tradeId, double result) override
        {
            std::lock_guard lock(mutex_);
            inner_->addResult(tradeId, result);
        }

        void addError(const std::string& tradeId, const std::string& error) override
        {
            std::lock_guard lock(mutex_);
            inner_->addError(tradeId, error);
        }

    private:
        IScalarResultReceiver* inner_;
        std::mutex mutex_;
    };

    struct QueuedTrade {
        ITrade* trade = nullptr;
        bool deleteAfterPricing = false;
    };

    std::map<std::string, IPricingEngine*> pricers_;
    std::unique_ptr<LockedScalarResultReceiver> lockedReceiver_;
    std::deque<QueuedTrade> queue_;
    std::vector<std::thread> workerThreads_;
    std::mutex queueMutex_;
    std::condition_variable queueNotEmpty_;
    std::condition_variable queueNotFull_;
    std::mutex fatalErrorMutex_;
    std::exception_ptr firstFatalError_;
    std::size_t queueCapacity_ = 0;
    bool acceptingTrades_ = false;
    
    void loadPricers();
    void workerLoop();
    void enqueueTrade(ITrade* trade, bool deleteAfterPricing);
    void shutdown(bool rethrowFatalError);
    
public:
    ~ParallelPricer();

    void start(IScalarResultReceiver* resultReceiver);
    void submit(ITrade* trade);
    void finish();
    void price(const std::vector<std::vector<ITrade*>>& tradeContainers,
               IScalarResultReceiver* resultReceiver);
};

#endif // PARALLELPRICER_H
