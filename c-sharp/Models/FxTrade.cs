using System;

namespace HmxLabs.TechTest.Models
{
    public class FxTrade : BaseTrade
    {
        public const TradeType FxSpotTradeType = TradeType.FxSpot;
        public const TradeType FxForwardTradeType = TradeType.FxFwd;

        private readonly TradeType _tradeType;
        private DateTime _valueDate;

        public FxTrade(TradeType tradeType, string tradeId)
        {
            if (string.IsNullOrWhiteSpace(tradeId))
                throw new ArgumentException("tradeId is required.", nameof(tradeId));
            
            _tradeType = tradeType;
            TradeId = tradeId;
        }

        public override TradeType TradeType
        {
            get { return _tradeType; }
        }

        public DateTime ValueDate
        {
            get { return _valueDate; }
            set { _valueDate = value; }
        }
    }
}