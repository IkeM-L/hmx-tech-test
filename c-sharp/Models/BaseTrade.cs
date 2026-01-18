using System;

namespace HmxLabs.TechTest.Models
{
    public abstract class BaseTrade : ITrade
    {
        public DateTime TradeDate { get; set; }

        public string? Counterparty { get; set; }

        public string? Instrument { get; set; }

        public double Notional { get; set; }
            
        public double Rate { get; set; }

        public abstract TradeType TradeType { get; }

        public string TradeId { get; protected set; }
    }
}
