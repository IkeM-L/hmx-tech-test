using System.Globalization;
using BenchmarkDotNet.Attributes;
using HmxLabs.TechTest.Loaders;

namespace Benchmarks;

[MemoryDiagnoser]
public class BondTradeLoaderBenchmarks
{
    private const char Separator = ',';

    private string[] _lines = Array.Empty<string>();

    [Params(1_000_000)]
    public int N;

    [GlobalSetup]
    public void Setup()
    {
        _lines = new string[N];
        
        for (int i = 0; i < N; i++)
        {
            var type = (i % 3) switch { 0 => "GovBond", 1 => "CorpBond", _ => "Supra" };
            var date = new DateTime(2025, 01, 01).AddDays(i % 365).ToString("yyyy-MM-dd", CultureInfo.InvariantCulture);
            var inst = $"GB{i % 1000:0000}";
            var cpty = $"CP{i % 200:000}";
            var notional = (1_000_000d + (i % 10_000) * 10d).ToString("N0", CultureInfo.InvariantCulture);
            var rate = (2.0 + (i % 500) * 0.001).ToString("0.###", CultureInfo.InvariantCulture);
            var tradeId = $"T{i:00000000}";

            _lines[i] = $"{type}{Separator}{date}{Separator}{inst}{Separator}{cpty}{Separator}{notional}{Separator}{rate}{Separator}{tradeId}";
        }
    }

    [Benchmark(Baseline = true)]
    public int Split_Parse_All()
    {
        var checksum = 0;
        for (var i = 0; i < _lines.Length; i++)
        {
            var t = BondTradeLoader.CreateTradeFromLine(_lines[i]);
            checksum ^= t.TradeId.GetHashCode(StringComparison.Ordinal);
        }
        return checksum;
    }

    [Benchmark]
    public int Span_Parse_All()
    {
        var checksum = 0;
        for (var i = 0; i < _lines.Length; i++)
        {
            var t = BondTradeLoader.CreateTradeFromLineWithSpan(_lines[i]);
            checksum ^= t.TradeId.GetHashCode(StringComparison.Ordinal);
        }
        return checksum;
    }
}