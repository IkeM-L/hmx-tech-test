using System.Globalization;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.Loaders
{
    public sealed class BondTradeLoader : ITradeLoader
    {
        private const char Separator = ',';

        public string? DataFile { get; set; }

        public IEnumerable<ITrade> LoadTrades()
        {
            if (string.IsNullOrWhiteSpace(DataFile))
                throw new InvalidOperationException("DataFile must be set before calling LoadTrades().");

            using var stream = new StreamReader(DataFile);
            // Discard header
            _ = stream.ReadLine();

            while (stream.ReadLine() is { } line)
            {
                if (string.IsNullOrWhiteSpace(line))
                    continue;

                yield return CreateTradeFromLine(line);
            }
        }

        internal static BondTrade CreateTradeFromLine(string line)
        {
            var items = line.Split(Separator);
            if (items.Length < 7)
                throw new FormatException($"Invalid trade line (expected 7 columns, got {items.Length}): {line}");

            string tradeType;
            switch (items[0].Trim())
            {
                case "GovBond":
                    tradeType = BondTrade.GovBondTradeType;
                    break;
                case "CorpBond":
                    tradeType = BondTrade.CorpBondTradeType;
                    break;
                case "Supra":
                    tradeType = BondTrade.SupraBondTradeType;
                    break;
                default:
                    throw new NotSupportedException("Found an unsupported trade type");
            }

            // Columns:
            // 0 Type
            // 1 TradeDate (yyyy-MM-dd)
            // 2 Instrument
            // 3 Counterparty
            // 4 Notional
            // 5 Rate
            // 6 TradeId
            var trade = new BondTrade(items[6].Trim(), tradeType)
            {
                TradeDate = DateTime.ParseExact(items[1].Trim(), "yyyy-MM-dd", CultureInfo.InvariantCulture),
                Instrument = items[2].Trim(),
                Counterparty = items[3].Trim(),
                Notional = double.Parse(items[4].Trim(), NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture),
                Rate = double.Parse(items[5].Trim(), NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture),
            };

            return trade;
        }

        // We could avoid allocating the string[] items and the substrings therein with span, but this logic is substancially harder to understand/maintain.
        // I would only do so if there was a need to reduce memory usage/GC in this part of the application and with 
        // microbenchmarks to prove it is actually performs better on the target hardware. For example on my hardware:
        // | Method          | N       | Mean     | Error   | StdDev  | Ratio | Gen0      | Allocated | Alloc Ratio |
        // |---------------- |-------- |---------:|--------:|--------:|------:|----------:|----------:|------------:|
        // | Split_Parse_All | 1000000 | 278.5 ms | 3.06 ms | 2.86 ms |  1.00 | 9500.0000 | 462.83 MB |        1.00 |
        // | Span_Parse_All  | 1000000 | 271.2 ms | 1.74 ms | 1.55 ms |  0.97 | 4000.0000 | 198.29 MB |        0.43 |
        // So while we do allocate substancially less with Span, this has a negligable impact on performance.
        // NOTE: Maintaining both with equivalent behaviour is the worst option, this is present to allow a (hypothetical) decision to be made about the best route to take
        internal static BondTrade CreateTradeFromLineWithSpan(string line)
        {
            ReadOnlySpan<char> input = line.AsSpan();

            Span<int> starts = stackalloc int[7];
            Span<int> lens = stackalloc int[7];

            var field = 0;
            var start = 0;

            while (field < 6)
            {
                var relativeIndex = input[start..].IndexOf(Separator);
                if (relativeIndex < 0)
                    break;

                var seperatorIndex = start + relativeIndex;
                starts[field] = start;
                lens[field] = seperatorIndex - start;

                field++;
                start = seperatorIndex + 1;
            }

            starts[field] = start;
            lens[field] = input.Length - start;
            field++;

            if (field < 7)
                throw new FormatException($"Invalid trade line (expected 7 columns, got {field}): {line}");

            var typeSpan = input.Slice(starts[0], lens[0]).Trim();

            var tradeType = typeSpan switch
            {
                var s when s.SequenceEqual("GovBond".AsSpan())  => BondTrade.GovBondTradeType,
                var s when s.SequenceEqual("CorpBond".AsSpan()) => BondTrade.CorpBondTradeType,
                var s when s.SequenceEqual("Supra".AsSpan())    => BondTrade.SupraBondTradeType,
                _ => throw new NotSupportedException("Found an unsupported trade type")
            };

            var trade = new BondTrade(input.Slice(starts[6], lens[6]).Trim().ToString(), tradeType)
            {
                TradeDate = DateTime.ParseExact(
                    input.Slice(starts[1], lens[1]).Trim(),
                    "yyyy-MM-dd".AsSpan(),
                    CultureInfo.InvariantCulture),

                Instrument = input.Slice(starts[2], lens[2]).Trim().ToString(),

                Counterparty = input.Slice(starts[3], lens[3]).Trim().ToString(),

                Notional = double.Parse(
                    input.Slice(starts[4], lens[4]).Trim(),
                    NumberStyles.Float | NumberStyles.AllowThousands,
                    CultureInfo.InvariantCulture),

                Rate = double.Parse(
                    input.Slice(starts[5], lens[5]).Trim(),
                    NumberStyles.Float | NumberStyles.AllowThousands,
                    CultureInfo.InvariantCulture),
            };

            return trade;
        }
    }
}
