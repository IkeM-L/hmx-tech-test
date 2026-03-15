#include "ParallelPricer.h"

#include "PricingConfigLoader.h"
#include "PricingEngineConfig.h"
#include "PricingEngineFactory.h"

#include <condition_variable>
#include <deque>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct ParallelPricer::Impl {
    class LockedScalarResultReceiver : public IScalarResultReceiver {
    public:
        explicit LockedScalarResultReceiver(IScalarResultReceiver& inner)
            : inner_(inner)
        {
        }

        void addResult(const std::string& tradeId, double result) override
        {
            std::lock_guard lock(mutex_);
            inner_.addResult(tradeId, result);
        }

        void addError(const std::string& tradeId, const std::string& error) override
        {
            std::lock_guard lock(mutex_);
            inner_.addError(tradeId, error);
        }

    private:
        IScalarResultReceiver& inner_;
        std::mutex mutex_;
    };

    struct QueuedTrade {
        std::unique_ptr<ITrade> ownedTrade;
        ITrade* borrowedTrade = nullptr;

        [[nodiscard]] ITrade* get() const
        {
            return ownedTrade ? ownedTrade.get() : borrowedTrade;
        }
    };

    PricingEngineConfig pricerConfig_;
    std::vector<PricingEngineFactory::PricingEngineMap> workerPricers_;
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

    ~Impl()
    {
        shutdown(false);
        clearWorkerPricers();
    }

    void loadPricers(const std::size_t workerCount)
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

    void clearWorkerPricers()
    {
        workerPricers_.clear();
    }

    void start(IScalarResultReceiver& resultReceiver)
    {
        if (acceptingTrades_ || !workerThreads_.empty())
        {
            throw std::logic_error("ParallelPricer is already running");
        }

        lockedReceiver_ = std::make_unique<LockedScalarResultReceiver>(resultReceiver);
        firstFatalError_ = nullptr;

        {
            std::lock_guard lock(queueMutex_);
            queue_.clear();
        }

        // Used in exception paths
        // ReSharper disable once CppDFAUnusedValue
        auto rollbackFailedStart = [this]() noexcept
        {
            {
                std::lock_guard lock(queueMutex_);
                acceptingTrades_ = false;
                queue_.clear();
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

            lockedReceiver_.reset();
            firstFatalError_ = nullptr;
        };

        try
        {
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
                workerThreads_.emplace_back(&Impl::workerLoop, this, i);
            }
        }
        catch (...)
        {
            rollbackFailedStart();
            throw;
        }
    }

    void submit(std::unique_ptr<ITrade> trade)
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

    void enqueueBorrowedTrade(ITrade& trade)
    {
        std::unique_lock lock(queueMutex_);
        queueNotFull_.wait(lock, [this]()
        {
            return !acceptingTrades_ || queue_.size() < queueCapacity_;
        });

        if (!acceptingTrades_)
        {
            throw std::logic_error("ParallelPricer is not running");
        }

        queue_.push_back(QueuedTrade{nullptr, &trade});

        lock.unlock();
        queueNotEmpty_.notify_one();
    }

    void workerLoop(const std::size_t workerIndex)
    {
        // Used in exception paths
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

                queuedTrade = std::move(queue_.front());
                queue_.pop_front();
            }

            queueNotFull_.notify_one();

            ITrade* trade = queuedTrade.get();

            if (trade == nullptr)
            {
                continue;
            }

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

    void finish()
    {
        shutdown(true);
    }

    void shutdown(const bool rethrowFatalError)
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
};

ParallelPricer::Session::Session(std::unique_ptr<Impl> impl)
    : impl_(std::move(impl))
{
}

ParallelPricer::Session::~Session()
{
    cleanup();
}

void ParallelPricer::Session::submit(std::unique_ptr<ITrade> trade) const
{
    if (impl_ == nullptr)
    {
        throw std::logic_error("ParallelPricer session is not active");
    }

    impl_->submit(std::move(trade));
}

void ParallelPricer::Session::finish()
{
    if (impl_ == nullptr)
    {
        throw std::logic_error("ParallelPricer session is not active");
    }

    std::unique_ptr<Impl> impl = std::move(impl_);
    impl->finish();
}

void ParallelPricer::Session::cleanup() noexcept
{
    impl_.reset();
}

ParallelPricer::Session ParallelPricer::start(IScalarResultReceiver& resultReceiver)
{
    auto impl = std::make_unique<Impl>();
    impl->start(resultReceiver);
    return Session(std::move(impl));
}

void ParallelPricer::price(const std::vector<std::vector<std::unique_ptr<ITrade>>>& tradeContainers,
                           IScalarResultReceiver& resultReceiver)
{
    Session session = start(resultReceiver);

    for (const auto& tradeContainer : tradeContainers)
    {
        for (const auto& trade : tradeContainer)
        {
            session.impl_->enqueueBorrowedTrade(*trade);
        }
    }

    session.finish();
}
