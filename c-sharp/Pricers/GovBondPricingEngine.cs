using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.Pricers
{
    public class GovBondPricingEngine : BasePricingEngine
    {
        public GovBondPricingEngine()
        {
            Delay = 5000;
            AddSupportedTradeType(TradeType.GovBond);
        }
    }
}
