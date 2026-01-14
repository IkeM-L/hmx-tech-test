using System.Globalization;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.Loaders
{
    public class FxTradeLoader : ITradeLoader
    {
        private const char Delimiter = '¬';

        public string? DataFile { get; set; }

        public IEnumerable<ITrade> LoadTrades()
        {
            if (string.IsNullOrWhiteSpace(DataFile))
                throw new InvalidOperationException("DataFile is not set.");

            if (!File.Exists(DataFile))
                throw new FileNotFoundException("Data file not found.", DataFile);

            using var reader = new StreamReader(DataFile);

            // Header lines:
            // 1) FxTrades¬yyyy-MM-dd
            // 2) Type¬TradeDate¬Ccy1¬Ccy2¬Amount¬Rate¬ValueDate¬Counterparty¬TradeId
            _ = reader.ReadLine();
            _ = reader.ReadLine();

            while (reader.ReadLine() is { } line)
            {
                if (string.IsNullOrWhiteSpace(line))
                    continue;

                if (line.StartsWith("END", StringComparison.OrdinalIgnoreCase))
                    yield break;

                var f = line.Split(Delimiter);
                if (f.Length != 9)
                    throw new FormatException($"Invalid trade row (expected 9 fields): '{line}'");

                var type = f[0];

                if (!string.Equals(type, FxTrade.FxSpotTradeType, StringComparison.OrdinalIgnoreCase) && !string.Equals(type, FxTrade.FxForwardTradeType, StringComparison.OrdinalIgnoreCase))
                {
                    throw new NotSupportedException($"Unsupported FX trade type: {type}");
                }

                // Type¬TradeDate¬Ccy1¬Ccy2¬Amount¬Rate¬ValueDate¬Counterparty¬TradeId
                var tradeDate = DateTime.ParseExact(f[1], "yyyy-MM-dd", CultureInfo.InvariantCulture);
                var ccy1 = f[2];
                var ccy2 = f[3];
                var notional = double.Parse(f[4], NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture);
                var rate = double.Parse(f[5], NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture);
                var valueDate = DateTime.ParseExact(f[6], "yyyy-MM-dd", CultureInfo.InvariantCulture);
                var counterparty = f[7];
                var tradeId = f[8];

                yield return new FxTrade(type, tradeId)
                {
                    TradeDate = tradeDate,
                    Instrument = ccy1 + ccy2, 
                    Notional = notional,
                    Rate = rate,
                    ValueDate = valueDate,
                    Counterparty = counterparty
                };
            }
        }
    }
}
