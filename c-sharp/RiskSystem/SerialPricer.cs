using System.Reflection;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem
{
    public class SerialPricer : PricerBase
    {
        public void Price(IEnumerable<IEnumerable<ITrade>> tradeContainters, IScalarResultReceiver resultReceiver, string assemblyPath)
        {
            LoadPricers(assemblyPath);

            foreach (var tradeContainter in tradeContainters)
            {
                foreach (var trade in tradeContainter)
                {
                    if (!TryGetPricer(trade.TradeType, out var pricer))
                    {
                        resultReceiver.AddError(trade.TradeId, "No Pricing Engines available for this trade type");
                        continue;
                    }

                    pricer.Price(trade, resultReceiver);
                }
            }
        }
    }
}