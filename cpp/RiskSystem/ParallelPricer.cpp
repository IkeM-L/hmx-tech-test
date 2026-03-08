#include "ParallelPricer.h"

#include "PricingEngineFactory.h"

#include <stdexcept>

ParallelPricer::~ParallelPricer()
{
    shutdown(false);

    for (auto& entry : pricers_)
    {
        delete entry.second;
    }
    pricers_.clear();
}

void ParallelPricer::loadPricers()
{
    if (!pricers_.empty())
    {
        return;
    }

    pricers_ = PricingEngineFactory::createPricersFromConfigFile("./PricingConfig/PricingEngines.xml");
}

void ParallelPricer::start(IScalarResultReceiver* resultReceiver)
{
    if (resultReceiver == nullptr)
    {
        throw std::invalid_argument("resultReceiver");
    }

    if (acceptingTrades_ || !workerThreads_.empty())
    {
        throw std::logic_error("ParallelPricer is already running");
    }

    loadPricers();

    lockedReceiver_ = std::make_unique<LockedScalarResultReceiver>(resultReceiver);
    firstFatalError_ = nullptr;
    queue_.clear();

    const unsigned int hardwareThreads = std::thread::hardware_concurrency();
    const std::size_t numThreads = hardwareThreads > 0 ? hardwareThreads : 4;
    queueCapacity_ = numThreads * 2;

    {
        std::lock_guard lock(queueMutex_);
        acceptingTrades_ = true;
    }

    workerThreads_.reserve(numThreads);
    for (std::size_t i = 0; i < numThreads; ++i)
    {
        workerThreads_.emplace_back(&ParallelPricer::workerLoop, this);
    }
}

void ParallelPricer::submit(ITrade* trade)
{
    enqueueTrade(trade, true);
}

void ParallelPricer::enqueueTrade(ITrade* trade, const bool deleteAfterPricing)
{
    if (trade == nullptr)
    {
        throw std::invalid_argument("trade");
    }

    std::unique_lock lock(queueMutex_);
    queueNotFull_.wait(lock, [this]()
    {
        return !acceptingTrades_ || queue_.size() < queueCapacity_;
    });

    if (!acceptingTrades_)
    {
        throw std::logic_error("ParallelPricer is not running");
    }

    queue_.push_back(QueuedTrade{trade, deleteAfterPricing});

    lock.unlock();
    queueNotEmpty_.notify_one();
}

void ParallelPricer::workerLoop()
{
    auto recordFatalError = [this](const std::exception_ptr& exPtr)
    {
        std::lock_guard lock(fatalErrorMutex_);
        if (firstFatalError_ == nullptr)
        {
            firstFatalError_ = exPtr;
        }
    };

    while (true)
    {
        QueuedTrade queuedTrade;

        {
            std::unique_lock lock(queueMutex_);
            queueNotEmpty_.wait(lock, [this]()
            {
                return !queue_.empty() || !acceptingTrades_;
            });

            if (queue_.empty())
            {
                if (!acceptingTrades_)
                {
                    return;
                }
                continue;
            }

            queuedTrade = queue_.front();
            queue_.pop_front();
        }

        queueNotFull_.notify_one();

        std::unique_ptr<ITrade> ownedTrade;
        ITrade* trade = queuedTrade.trade;
        if (queuedTrade.deleteAfterPricing)
        {
            ownedTrade.reset(trade);
        }

        if (trade == nullptr)
        {
            continue;
        }

        try
        {
            const std::string tradeType = trade->getTradeType();
            const auto it = pricers_.find(tradeType);

            if (it == pricers_.end())
            {
                lockedReceiver_->addError(
                    trade->getTradeId(),
                    "No Pricing Engines available for this trade type");
                continue;
            }

            it->second->price(trade, lockedReceiver_.get());
        }
        catch (const std::exception& ex)
        {
            try
            {
                lockedReceiver_->addError(trade->getTradeId(), ex.what());
            }
            catch (...)
            {
                recordFatalError(std::current_exception());
            }
        }
        catch (...)
        {
            try
            {
                lockedReceiver_->addError(trade->getTradeId(), "Unknown pricing error");
            }
            catch (...)
            {
                recordFatalError(std::current_exception());
            }
        }
    }
}

void ParallelPricer::finish()
{
    shutdown(true);
}

void ParallelPricer::shutdown(const bool rethrowFatalError)
{
    {
        std::lock_guard lock(queueMutex_);
        acceptingTrades_ = false;
    }

    queueNotEmpty_.notify_all();
    queueNotFull_.notify_all();

    for (auto& workerThread : workerThreads_)
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }
    workerThreads_.clear();

    while (!queue_.empty())
    {
        auto queuedTrade = queue_.front();
        queue_.pop_front();
        if (queuedTrade.deleteAfterPricing)
        {
            delete queuedTrade.trade;
        }
    }

    lockedReceiver_.reset();

    const std::exception_ptr fatalError = firstFatalError_;
    firstFatalError_ = nullptr;

    if (rethrowFatalError && fatalError != nullptr)
    {
        std::rethrow_exception(fatalError);
    }
}

void ParallelPricer::price(const std::vector<std::vector<ITrade*>>& tradeContainers,
                           IScalarResultReceiver* resultReceiver)
{
    start(resultReceiver);

    try
    {
        for (const auto& tradeContainer : tradeContainers)
        {
            for (ITrade* trade : tradeContainer)
            {
                enqueueTrade(trade, false);
            }
        }
    }
    catch (...)
    {
        shutdown(false);
        throw;
    }

    finish();
}
