using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace CardControlService.Models
{
    public class CardPersonalizationCreateRequest
    {
        public long RequestId { get; set; }

        public string CardUid { get; set; }

        public string MagicNumber { get; set; }

        public int Version { get; set; }

        public string CardType { get; set; }

        public string ExpiryDate { get; set; }

        public int InitialBalanceKurus { get; set; }

        public int MaxAllowedBalance { get; set; }

        public string VisaDate { get; set; }

        public string Status { get; set; }
    }
}