using System.Reflection;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem
{
    public class SerialPricer : PricerBase
    {
        public void Price(IEnumerable<IEnumerable<ITrade>> tradeContainters_, IScalarResultReceiver resultReceiver_, string assemblyPath)
        {
            LoadPricers(assemblyPath);

            foreach (var tradeContainter in tradeContainters_)
            {
                foreach (var trade in tradeContainter)
                {
                    if (!Pricers.TryGetValue(trade.TradeType, out var pricer))
                    {
                        resultReceiver_.AddError(trade.TradeId, "No Pricing Engines available for this trade type");
                        continue;
                    }

                    pricer.Price(trade, resultReceiver_);
                }
            }
        }
    }
}