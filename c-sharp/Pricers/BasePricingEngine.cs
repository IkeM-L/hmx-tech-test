using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.Pricers
{
    public abstract class BasePricingEngine : IPricingEngine
    {
        public void Price(ITrade trade_, IScalarResultReceiver resultReceiver_)
        {
            if (null == resultReceiver_)
            {
                throw new ArgumentNullException("resultReceiver_");
            }

            if (null == trade_)
            {
                throw new ArgumentNullException("trade_");
            }

            PriceTrade(trade_, resultReceiver_);
        }

        public bool IsTradeTypeSupported(TradeType tradeType_)
        {
            return _supportedTypes.ContainsKey(tradeType_);
        }

        protected BasePricingEngine()
        {
            Delay = 5000;
        }

        protected  void AddSupportedTradeType(TradeType tradeType_)
        {
            _supportedTypes.Add(tradeType_, 0);
        }

        protected int Delay { get; set; }

        protected virtual void PriceTrade(ITrade trade, IScalarResultReceiver resultReceiver_)
        {
            if (!IsTradeTypeSupported(trade.TradeType))
            {
                if (null == trade.TradeId)
                    throw new ArgumentNullException(nameof(trade));
                
                resultReceiver_.AddError(trade.TradeId, "Trade type not supported");
                return;
            }

            Console.WriteLine("Started pricing trade: " + trade.TradeId);
            Thread.Sleep(Delay);
            var result = CalculateResult();

            if (TradesToError.ContainsKey(trade.TradeId!))
            {
                resultReceiver_.AddError(trade.TradeId!, TradesToError[trade.TradeId]);
            }
            else
            {
                resultReceiver_.AddResult(trade.TradeId, result);
                if (TradesToWarn.ContainsKey(trade.TradeId))
                {
                    resultReceiver_.AddError(trade.TradeId, TradesToWarn[trade.TradeId]);
                }
            }

            Console.WriteLine("Completed pricing trade: " + trade.TradeId);
        }

        protected virtual double CalculateResult()
        {
            return _random.NextDouble()*100;
        }

        private readonly Dictionary<TradeType, uint> _supportedTypes = new Dictionary<TradeType, uint>();
        private readonly Random _random = new Random();
        private static readonly Dictionary<string, string> TradesToError = new Dictionary<string, string>();
        private static readonly Dictionary<string, string> TradesToWarn = new Dictionary<string, string>();

        static BasePricingEngine()
        {
            TradesToError.Add("GOV006", "Undefined error in pricing");
            TradesToWarn.Add("FWD001", "Unable to calibrate model to value date");
        }
    }
}