using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.Pricers
{
    public class FxPricingEngine : BasePricingEngine
    {
        public FxPricingEngine()
        {
            Delay = 2000;
            AddSupportedTradeType(TradeType.FxSpot);
            AddSupportedTradeType(TradeType.FxFwd);
        }
    }
}