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

            var tradeList = new BondTradeList();
            LoadTradesFromFile(DataFile, tradeList);
            return tradeList;
        }

        private static BondTrade CreateTradeFromLine(string line)
        {
            var items = line.Split(Separator);
            if (items.Length < 7)
                throw new FormatException($"Invalid trade line (expected 7 columns, got {items.Length}): {line}");

            string tradeType;
            if (items[0] == "GovBond")
                tradeType = BondTrade.GovBondTradeType;
            else if (items[0] == "CorpBond")
                tradeType = BondTrade.CorpBondTradeType;
            else if (items[0] == "Supra")
                tradeType = BondTrade.SupraBondTradeType;
            else
                throw new NotSupportedException("Found an unsupported trade type");

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

        private static void LoadTradesFromFile(string filename, BondTradeList tradeList)
        {
            if (tradeList is null) throw new ArgumentNullException(nameof(tradeList));

            using var stream = new StreamReader(filename);

            // Read and discard header (or treat empty file as no trades).
            _ = stream.ReadLine();

            while (stream.ReadLine() is { } line)
            {
                if (string.IsNullOrWhiteSpace(line))
                    continue;

                tradeList.Add(CreateTradeFromLine(line));
            }
        }
    }
}
