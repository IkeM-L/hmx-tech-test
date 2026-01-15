using System.Xml.Linq;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem
{
    public class PricingConfigLoader
    {
        public string? ConfigFile { get; set; }

        public PricingEngineConfig LoadConfig()
        {
            if (string.IsNullOrWhiteSpace(ConfigFile))
                throw new InvalidOperationException("ConfigFile is not set.");

            if (!File.Exists(ConfigFile))
                throw new FileNotFoundException("Config file not found.", ConfigFile);

            var doc = XDocument.Load(ConfigFile);

            var config = new PricingEngineConfig();

            var root = doc.Root;
            if (root == null || !string.Equals(root.Name.LocalName, "PricingEngines", StringComparison.Ordinal))
                return config;

            foreach (var e in root.Elements("Engine"))
            {
                var tradeTypeRaw = (string?)e.Attribute("tradeType");
                var assembly = (string?)e.Attribute("assembly");
                var typeName = (string?)e.Attribute("pricingEngine");

                var mappedTradeType = MapTradeType(tradeTypeRaw);

                config.Add(new PricingEngineConfigItem
                {
                    TradeType = mappedTradeType,
                    Assembly = assembly,
                    TypeName = typeName
                });
            }

            return config;
        }

        private static string? MapTradeType(string? tradeType)
        {
            if (string.IsNullOrWhiteSpace(tradeType))
                return tradeType;

            // Map known XML values to the model constants.
            switch (tradeType)
            {
                case "GovBond":
                    return BondTrade.GovBondTradeType;
                case "CorpBond":
                    return BondTrade.CorpBondTradeType;
                case "FxSpot":
                    return FxTrade.FxSpotTradeType;
                case "FxFwd":
                    return FxTrade.FxForwardTradeType;
                default:
                    return tradeType;
            }
        }
    }
}