using System.Reflection;
using System.Runtime.CompilerServices;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem;

public abstract class PricerBase
{
    private readonly Dictionary<TradeType, IPricingEngine> _pricers = new Dictionary<TradeType, IPricingEngine>();

    protected void LoadPricers(string assemblyPath)
    {
        var pricingConfigLoader = new PricingConfigLoader { ConfigFile = @"PricingConfig/PricingEngines.xml" };
        var pricerConfig = pricingConfigLoader.LoadConfig();
        
        if(assemblyPath == null)
            throw new ArgumentNullException(nameof(assemblyPath));
            
        // Dynamic loading these like this will work, but the requirement for no compile-time dependency is strange and I would want to know why it
        // exists/if there is an alternative solution. This is very fragile.
        // This may throw other exceptions, but if it does this is a fatal issue for the application so we do not catch
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
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    protected bool TryGetPricer(TradeType tradeType, out IPricingEngine? engine)
    {
        return _pricers.TryGetValue(tradeType, out engine);
    }
}