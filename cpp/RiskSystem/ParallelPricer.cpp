#include "ParallelPricer.h"

#include "PricingEngineFactory.h"
#include "PricingConfigLoader.h"

#include <stdexcept>

ParallelPricer::~ParallelPricer()
{
    shutdown(false);
    clearWorkerPricers();
}

void ParallelPricer::loadPricers(const std::size_t workerCount)
{
    if (pricerConfig_.empty())
    {
        PricingConfigLoader pricingConfigLoader;
        pricingConfigLoader.setConfigFile("./PricingConfig/PricingEngines.xml");
        pricerConfig_ = pricingConfigLoader.loadConfig();
    }

    if (workerPricers_.size() == workerCount)
    {
        return;
    }

    clearWorkerPricers();

    try
    {
        workerPricers_.reserve(workerCount);
        for (std::size_t i = 0; i < workerCount; ++i)
        {
            // Pricing engines are not designed as shared thread-safe objects, so
            // each worker gets its own set and can price independently.
            workerPricers_.push_back(PricingEngineFactory::createPricersFromConfig(pricerConfig_));
        }
    }
    catch (...)
    {
        clearWorkerPricers();
        throw;
    }
}

void ParallelPricer::clearWorkerPricers()
{
    for (auto& workerPricers : workerPricers_)
    {
        for (auto& entry : workerPricers)
        {
            delete entry.second;
        }
        workerPricers.clear();
    }
    workerPricers_.clear();
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

    lockedReceiver_ = std::make_unique<LockedScalarResultReceiver>(resultReceiver);
    firstFatalError_ = nullptr;
    queue_.clear();

    const unsigned int hardwareThreads = std::thread::hardware_concurrency();
    const std::size_t numThreads = hardwareThreads > 0 ? hardwareThreads : 4;
    loadPricers(numThreads);
    queueCapacity_ = numThreads * 2;

    {
        std::lock_guard lock(queueMutex_);
        acceptingTrades_ = true;
    }

    workerThreads_.reserve(numThreads);
    for (std::size_t i = 0; i < numThreads; ++i)
    {
        workerThreads_.emplace_back(&ParallelPricer::workerLoop, this, i);
    }
}

void ParallelPricer::submit(std::unique_ptr<ITrade> trade)
{
    enqueueTrade(std::move(trade));
}

void ParallelPricer::enqueueTrade(std::unique_ptr<ITrade> trade)
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

    queue_.push_back(QueuedTrade{std::move(trade), nullptr});

    lock.unlock();
    queueNotEmpty_.notify_one();
}

void ParallelPricer::enqueueTrade(ITrade* trade)
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

    queue_.push_back(QueuedTrade{nullptr, trade});

    lock.unlock();
    queueNotEmpty_.notify_one();
}

void ParallelPricer::workerLoop(const std::size_t workerIndex)
{
    // Used inside exception paths
    // ReSharper disable once CppDFAUnusedValue
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

            // Move the queue entry out under the lock, then do the expensive work
            // after releasing it so producers and other workers can keep progressing.
            queuedTrade = std::move(queue_.front());
            queue_.pop_front();
        }

        queueNotFull_.notify_one();

        ITrade* trade = queuedTrade.get();

        if (trade == nullptr)
        {
            continue;
        }

        // Each worker indexes into its own pricer map to avoid sharing engine state.
        auto& pricers = workerPricers_[workerIndex];

        try
        {
            const std::string tradeType = trade->getTradeType();
            const auto it = pricers.find(tradeType);

            if (it == pricers.end())
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

    {
        std::lock_guard lock(queueMutex_);

        // Draining the queue stays under the mutex so concurrent rejected
        // submitters cannot race with shutdown on the deque itself.
        queue_.clear();
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
                enqueueTrade(trade);
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
