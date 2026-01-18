using System.Threading.Channels;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem;

public class ParallelPricer : PricerBase
{
    /// <summary>
    /// Price in parallel using numThreads threads, or Environment.ProcessorCount if not provided
    /// </summary>
    /// <param name="tradeContainers"></param>
    /// <param name="resultReceiver"></param>
    /// <param name="assemblyPath"></param>
    /// <param name="numProcessors"></param>
    public void Price(IEnumerable<IEnumerable<ITrade>> tradeContainers, IScalarResultReceiver resultReceiver, string assemblyPath, int numProcessors = -1)
    {
        LoadPricers(assemblyPath);

        if (numProcessors <= 0)
            numProcessors = Environment.ProcessorCount;
        
        var lockedReceiver = new LockedScalarResultReceiver(resultReceiver);

        // Bounded so we do not materialise entire input
        var capacity = numProcessors*2;
        var channel = Channel.CreateBounded<ITrade>(new BoundedChannelOptions(capacity)
        {
            SingleWriter = true,   // only the enumerating producer writes
            SingleReader = false,  // many consumers read
            FullMode = BoundedChannelFullMode.Wait
        });

        // Run producer + consumers and wait.
        Run(channel, tradeContainers, lockedReceiver, numProcessors).GetAwaiter().GetResult();
    }

    private async Task Run(Channel<ITrade> channel, IEnumerable<IEnumerable<ITrade>> tradeContainters_, IScalarResultReceiver lockedReceiver, int numThreads)
    {
        var writer = channel.Writer;
        var reader = channel.Reader;

        // Producer: Enumerate the input on a single thread as we cannot guarantee the enumerator is thread safe
        var producer = Task.Run(async () =>
        {
            try
            {
                foreach (var container in tradeContainters_)
                {
                    foreach (var trade in container)
                    {
                        await writer.WriteAsync(trade).ConfigureAwait(false);
                    }
                }

                writer.TryComplete();
            }
            catch (Exception ex)
            {
                writer.TryComplete(ex);
            }
        });

        // Consumers: ChannelReader is safe for concurrent readers, so we can do numThreads tasks
        var consumers = new Task[numThreads];
        for (var i = 0; i < numThreads; i++)
        {
            consumers[i] = Task.Run(async () =>
            {
                try
                {
                    await foreach (var trade in reader.ReadAllAsync().ConfigureAwait(false))
                    {
                        try
                        {
                            if (!TryGetPricer(trade.TradeType, out var pricer))
                            {
                                lockedReceiver.AddError(trade.TradeId, "No Pricing Engines available for this trade type");
                                continue;
                            }

                            pricer.Price(trade, lockedReceiver);
                        }
                        catch (Exception ex)
                        {
                            lockedReceiver.AddError(trade.TradeId, ex.Message);
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Log exceptions instead of bubbling up to avoid progress loss during long running task
                    // Note that Console.Log should not be used in prod as console IO is slow and it is not inspectable
                    Console.WriteLine($"Error processing trade {ex.Message}");
                }
            });
        }
        
        await Task.WhenAll(consumers).ConfigureAwait(false);
        await producer.ConfigureAwait(false);
    }

    private sealed class LockedScalarResultReceiver : IScalarResultReceiver
    {
        private readonly IScalarResultReceiver _inner;
        private readonly object _lock = new();

        public LockedScalarResultReceiver(IScalarResultReceiver inner)
        {
            _inner = inner;
        }

        public void AddResult(string tradeId_, double result_)
        {
            lock (_lock)
            {
                _inner.AddResult(tradeId_, result_);
            }
        }

        public void AddError(string tradeId, string message)
        {
            lock (_lock)
            {
                _inner.AddError(tradeId, message);
            }
        }
    }
}
