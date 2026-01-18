namespace HmxLabs.TechTest.Models
{
    public class BondTrade : BaseTrade
    {
        private readonly string _tradeType;

        public BondTrade(string tradeId, string tradeType)
        {
            if (string.IsNullOrWhiteSpace(tradeId))
            {
                throw new ArgumentException("A valid non null, non empty trade ID must be provided");
            }

            // Implemented as a switch to maintain string semanics/compatibility with external code
            // If this is not a core requirement, I would prefer to use an enum so the type checker can help verify correctness,
            // make dictionary lookup faster, lower memory requirements, etc.
            switch (tradeType)
            {
                case GovBondTradeType:
                    _tradeType = GovBondTradeType;
                    break;
                case CorpBondTradeType:
                    _tradeType = CorpBondTradeType;
                    break;
                case SupraBondTradeType:
                    _tradeType = SupraBondTradeType;
                    break;
                default:
                    throw new ArgumentException($"A valid trade type must be used, found {tradeType}");
            }
            
            TradeId = tradeId;
        }

        public const string GovBondTradeType = "GovBond";
        public const string CorpBondTradeType = "CorpBond";
        public const string SupraBondTradeType = "SupraBond";

        public override string TradeType { get { return _tradeType; } }
    }
}
