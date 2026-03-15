#ifndef PARALLELPRICER_H
#define PARALLELPRICER_H

#include "../Models/ITrade.h"
#include "../Models/IScalarResultReceiver.h"

#include <memory>
#include <vector>

class ParallelPricer {
private:
    struct Impl;

public:
    class Session {
    public:
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;
        Session(Session&& other) noexcept = default;
        Session& operator=(Session&& other) noexcept = default;
        ~Session();

        /// Submits a streamed trade through an active pricing session.
        void submit(std::unique_ptr<ITrade> trade) const;

        /// Waits for all queued trades to finish processing and closes the session.
        void finish();

    private:
        friend class ParallelPricer;

        explicit Session(std::unique_ptr<Impl> impl);
        void cleanup() noexcept;

        std::unique_ptr<Impl> impl_;
    };

    ParallelPricer() = delete;
    ParallelPricer(const ParallelPricer&) = delete;
    ParallelPricer& operator=(const ParallelPricer&) = delete;
    ParallelPricer(ParallelPricer&&) = delete;
    ParallelPricer& operator=(ParallelPricer&&) = delete;
    ~ParallelPricer() = delete;

    /// Starts a move-only session that owns the entire lifecycle of a pricing run.
    [[nodiscard]] static Session start(IScalarResultReceiver& resultReceiver);

    /// Prices owned trade containers in parallel without taking ownership from the caller.
    static void price(const std::vector<std::vector<std::unique_ptr<ITrade>>>& tradeContainers,
                      IScalarResultReceiver& resultReceiver);
};

#endif // PARALLELPRICER_H
