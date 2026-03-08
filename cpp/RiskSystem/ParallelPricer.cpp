#include "ParallelPricer.h"
#include "PricingEngineFactory.h"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{
    class LockedScalarResultReceiver : public IScalarResultReceiver
    {
    public:
        explicit LockedScalarResultReceiver(IScalarResultReceiver* inner)
            : inner_(inner)
        {
            if (inner_ == nullptr)
            {
                throw std::invalid_argument("resultReceiver");
            }
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
}

ParallelPricer::~ParallelPricer()
{
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

void ParallelPricer::price(const std::vector<std::vector<ITrade*>>& tradeContainers,
                           IScalarResultReceiver* resultReceiver)
{
    if (resultReceiver == nullptr)
    {
        throw std::invalid_argument("resultReceiver");
    }

    loadPricers();

    LockedScalarResultReceiver lockedReceiver(resultReceiver);

    const unsigned int hardwareThreads = std::thread::hardware_concurrency();
    const std::size_t numThreads = hardwareThreads > 0 ? hardwareThreads : 4;
    const std::size_t queueCapacity = numThreads * 2;

    std::deque<ITrade*> queue;
    std::mutex queueMutex;
    std::condition_variable queueNotEmpty;
    std::condition_variable queueNotFull;
    bool producerDone = false;

    std::exception_ptr firstFatalError = nullptr;
    std::mutex fatalErrorMutex;

    auto recordFatalError = [&](const std::exception_ptr& exPtr)
    {
        std::lock_guard lock(fatalErrorMutex);
        if (firstFatalError == nullptr)
        {
            firstFatalError = exPtr;
        }
    };

    auto producer = [&]
    {
        try
        {
            for (const auto& tradeContainer : tradeContainers)
            {
                for (ITrade* trade : tradeContainer)
                {
                    std::unique_lock lock(queueMutex);
                    queueNotFull.wait(lock, [&]()
                    {
                        return queue.size() < queueCapacity;
                    });

                    queue.push_back(trade);

                    lock.unlock();
                    queueNotEmpty.notify_one();
                }
            }
        }
        catch (...)
        {
            recordFatalError(std::current_exception());
        }

        {
            std::lock_guard lock(queueMutex);
            producerDone = true;
        }
        queueNotEmpty.notify_all();
    };

    auto consumer = [&]()
    {
        while (true)
        {
            ITrade* trade = nullptr;

            {
                std::unique_lock lock(queueMutex);
                queueNotEmpty.wait(lock, [&]()
                {
                    return !queue.empty() || producerDone;
                });

                if (queue.empty())
                {
                    if (producerDone)
                    {
                        return;
                    }
                    continue;
                }

                trade = queue.front();
                queue.pop_front();
            }

            queueNotFull.notify_one();

            if (trade == nullptr)
            {
                continue;
            }

            try
            {
                const std::string tradeType = trade->getTradeType();
                auto it = pricers_.find(tradeType);

                if (it == pricers_.end())
                {
                    lockedReceiver.addError(
                        trade->getTradeId(),
                        "No Pricing Engines available for this trade type");
                    continue;
                }

                it->second->price(trade, &lockedReceiver);
            }
            catch (const std::exception& ex)
            {
                lockedReceiver.addError(trade->getTradeId(), ex.what());
            }
            catch (...)
            {
                lockedReceiver.addError(trade->getTradeId(), "Unknown pricing error");
            }
        }
    };

    std::thread producerThread(producer);

    std::vector<std::thread> consumerThreads;
    consumerThreads.reserve(numThreads);

    for (std::size_t i = 0; i < numThreads; ++i)
    {
        consumerThreads.emplace_back(consumer);
    }

    producerThread.join();

    for (auto& thread : consumerThreads)
    {
        thread.join();
    }

    if (firstFatalError != nullptr)
    {
        std::rethrow_exception(firstFatalError);
    }
}