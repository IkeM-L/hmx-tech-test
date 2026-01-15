using System.Reflection;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem
{
    public class SerialPricer
    {
        public void Price(IEnumerable<IEnumerable<ITrade>> tradeContainters_, IScalarResultReceiver resultReceiver_)
        {
            LoadPricers();

            foreach (var tradeContainter in tradeContainters_)
            {
                foreach (var trade in tradeContainter)
                {
                    if (!_pricers.ContainsKey(trade.TradeType))
                    {
                        resultReceiver_.AddError(trade.TradeId, "No Pricing Engines available for this trade type");
                        continue;
                    }

                    var pricer = _pricers[trade.TradeType];
                    pricer.Price(trade, resultReceiver_);
                }
            }
        }

        private void LoadPricers()
        {
            var pricingConfigLoader = new PricingConfigLoader { ConfigFile = @"PricingConfig/PricingEngines.xml" };
            var pricerConfig = pricingConfigLoader.LoadConfig();
            
            // Dynamic loading these like this will work, but the requirement for no compile-time dependency is strange and I would want to know why it
            // exists/if there is an alternative solution. This is very fragile.
            string assemblyPath = @"../../../../Pricers/bin/Debug/net8.0/Pricers.dll";
            // This may throw, if it does this is a fatal issue for the program so we do not catch
            Assembly asm = Assembly.LoadFrom(assemblyPath);
            

            foreach (var configItem in pricerConfig)
            {
                try
                {
                    Type type = asm.GetType(configItem.TypeName, throwOnError: true);
                    var pricer = (IPricingEngine)Activator.CreateInstance(type);
                    if (configItem.TradeType != null && pricer != null) 
                        _pricers.Add(configItem.TradeType, pricer);
                    else
                        Console.WriteLine($"Error creating pricer for {configItem.TradeType}");
                }
                catch (Exception e)
                {
                    // We would use a proper logger not Console in production
                    Console.WriteLine($"Exception when creating pricer for {configItem.TradeType} {e.Message}");
                }
            }
        }

        private readonly Dictionary<string, IPricingEngine> _pricers = new Dictionary<string, IPricingEngine>();
    }
}