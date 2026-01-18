using System.Threading.Channels;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem;

public class ParallelPricer : PricerBase
{
    /// <summary>
    /// Price in parallel using numThreads threads, or Environment.ProcessorCount if not provided
    /// Returns true if all trades were processed, false if a fatal error occured and results are partial
    /// </summary>
    /// <param name="tradeContainers"></param>
    /// <param name="resultReceiver"></param>
    /// <param name="assemblyPath"></param>
    /// <param name="numThreads"></param>
    public bool Price(IEnumerable<IEnumerable<ITrade>> tradeContainers, IScalarResultReceiver resultReceiver, string assemblyPath, int numThreads = -1)
    {
        LoadPricers(assemblyPath);
        
        if (numThreads <= 0)
            numThreads = Environment.ProcessorCount;
        
        var lockedReceiver = new LockedScalarResultReceiver(resultReceiver);

        // Bounded so we do not materialise entire input
        // This needs tuning based on real-world behavior/memory constraints/etc., 2 should be appropriate if 
        // all pricers take the same oom of time - we might want less if memory is highly 
        // restricted or more if the processing can be bursty (e.g. if one type of 
        // pricer is much faster)
        var capacity = numThreads*2;
        var channel = Channel.CreateBounded<ITrade>(new BoundedChannelOptions(capacity)
        {
            SingleWriter = true,   // only the enumerating producer writes
            SingleReader = false,  // many consumers read
            FullMode = BoundedChannelFullMode.Wait // Provide backpressure 
        });
        
        return Run(channel, tradeContainers, lockedReceiver, numThreads).GetAwaiter().GetResult();
    }

    private async Task<bool> Run(Channel<ITrade> channel, IEnumerable<IEnumerable<ITrade>> tradeContainers, IScalarResultReceiver lockedReceiver, int numThreads)
    {
        var writer = channel.Writer;
        var reader = channel.Reader;


        // WARNING: this is currently accessed in a thread safe way from multiple threads,
        // if modifying this code take care to maintain that or add a lock
        Exception? firstError = null;

        void MarkIncomplete(Exception ex)
        {
            Interlocked.CompareExchange(ref firstError, ex, null);
        }

        // Producer: Enumerate the input on a single thread as we cannot guarantee the enumerator is thread safe
        var producer = Task.Run(async () =>
        {
            try
            {
                foreach (var container in tradeContainers)
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
                MarkIncomplete(ex);
                writer.TryComplete(ex);
            }
        });

        // Consumers: ChannelReader is safe for concurrent readers, so we can do numThreads tasks (saving one for the producer)
        var consumers = new Task[numThreads - 1];
        for (var i = 0; i < numThreads - 1; i++)
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
                    // Mark incomplete instead of bubbling up to avoid progress/data loss during long-running task
                    MarkIncomplete(ex);
                }
            });
        }
        
        await Task.WhenAll(consumers.Append(producer)).ConfigureAwait(false);

        var err = firstError; // atomic
        if (err != null)
        {
            // Note that Console.WriteLine should not be used in prod as console IO is slow and not inspectable
            // and a failure here should be very apparent to the end user
            Console.WriteLine($"CRITICAL: Pricing did not complete successfully. Partial results returned. Reason: {err.Message}");
            return false;
        }
        return true;
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
