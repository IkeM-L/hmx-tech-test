using System.Globalization;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.Loaders
{
    public class FxTradeLoader : ITradeLoader
    {
        private const char Delimiter = '¬';

        public string? DataFile { get; set; }

        /// <summary>
        /// Loads all trades in the data file
        /// </summary>
        /// <returns></returns>
        /// <exception cref="InvalidOperationException"></exception>
        /// <exception cref="FileNotFoundException"></exception>
        /// <exception cref="FormatException"></exception>
        /// <exception cref="NotSupportedException"></exception>
        // Note that span-based parsing could also be used here, but would 
        // look very similar to the alternate logic in BondTradeLoader
        public IEnumerable<ITrade> LoadTrades()
        {
            if (string.IsNullOrWhiteSpace(DataFile))
                throw new InvalidOperationException("DataFile is not set.");

            if (!File.Exists(DataFile))
                throw new FileNotFoundException("Data file not found.", DataFile);

            using var reader = new StreamReader(DataFile);

            // Discard header lines:
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

                var fxTradeArray = line.Split(Delimiter, StringSplitOptions.TrimEntries);
                if (fxTradeArray.Length != 9)
                    throw new FormatException($"Invalid trade row (expected 9 fields): '{line}'");

                TradeType type;
                
                switch (fxTradeArray[0])
                {
                    case nameof(TradeType.FxSpot):
                        type = FxTrade.FxSpotTradeType;
                        break;
                    case nameof(TradeType.FxFwd):
                        type = FxTrade.FxForwardTradeType;
                        break;
                    default:
                        throw new NotSupportedException($"Unsupported FX trade type {fxTradeArray[0]}");
                }

                // Type¬TradeDate¬Ccy1¬Ccy2¬Amount¬Rate¬ValueDate¬Counterparty¬TradeId
                var tradeDate = DateTime.ParseExact(fxTradeArray[1], "yyyy-MM-dd", CultureInfo.InvariantCulture); // It would be good to standardise on a time zone or store the time zone, but the data is not available
                var ccy1 = fxTradeArray[2];
                var ccy2 = fxTradeArray[3];
                var notional = double.Parse(fxTradeArray[4], NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture);
                var rate = double.Parse(fxTradeArray[5], NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture);
                var valueDate = DateTime.ParseExact(fxTradeArray[6], "yyyy-MM-dd", CultureInfo.InvariantCulture);
                var counterparty = fxTradeArray[7];
                var tradeId = fxTradeArray[8];

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
