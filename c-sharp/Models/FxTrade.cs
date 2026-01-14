using System;

namespace HmxLabs.TechTest.Models
{
    public class FxTrade : BaseTrade
    {
        public const string FxSpotTradeType = "FxSpot";
        public const string FxForwardTradeType = "FxFwd";

        private readonly string _tradeType;

        public FxTrade(string tradeType, string tradeId)
        {
            if (string.IsNullOrWhiteSpace(tradeType))
                throw new ArgumentException("tradeType is required.", nameof(tradeType));

            if (string.IsNullOrWhiteSpace(tradeId))
                throw new ArgumentException("tradeId is required.", nameof(tradeId));

            _tradeType = tradeType;
            TradeId = tradeId;
        }

        public override string TradeType
        {
            get { return _tradeType; }
        }

        public DateTime ValueDate { get; set; }
    }
}