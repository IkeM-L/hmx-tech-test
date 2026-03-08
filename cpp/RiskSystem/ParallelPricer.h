#ifndef PARALLELPRICER_H
#define PARALLELPRICER_H

#include "../Models/IPricingEngine.h"
#include "../Models/ITrade.h"
#include "../Models/IScalarResultReceiver.h"
#include "PricingEngineConfig.h"
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
        // A queue entry can either own a streamed trade or borrow one from the
        // batch pricing path, which keeps the queue logic shared across both modes.
        std::unique_ptr<ITrade> ownedTrade;
        ITrade* borrowedTrade = nullptr;

        ITrade* get() const
        {
            return ownedTrade ? ownedTrade.get() : borrowedTrade;
        }
    };

    PricingEngineConfig pricerConfig_;
    std::vector<std::map<std::string, IPricingEngine*>> workerPricers_;
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
    
    void loadPricers(std::size_t workerCount);
    void clearWorkerPricers();
    void workerLoop(std::size_t workerIndex);
    void enqueueTrade(std::unique_ptr<ITrade> trade);
    void enqueueBorrowedTrade(ITrade& trade);
    void shutdown(bool rethrowFatalError);
    
public:
    /// Stops worker threads and releases any owned pricing engines.
    ~ParallelPricer();

    /// Starts worker threads and prepares the pricer to receive streamed trades.
    void start(IScalarResultReceiver* resultReceiver);

    /// Submits a streamed trade and transfers ownership to the pricer.
    void submit(std::unique_ptr<ITrade> trade);

    /// Waits for all queued trades to finish processing.
    void finish();

    /// Prices owned trade containers in parallel without taking ownership from the caller.
    void price(const std::vector<std::vector<std::unique_ptr<ITrade>>>& tradeContainers,
               IScalarResultReceiver* resultReceiver);
};

#endif // PARALLELPRICER_H
