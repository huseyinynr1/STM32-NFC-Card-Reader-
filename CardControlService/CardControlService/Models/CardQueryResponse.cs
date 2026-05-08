using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace CardControlService.Models
{
    public class CardQueryResponse
    {
        public bool Success { get; set; }
        public string Message { get; set; }

        public string CardUid { get; set; }
        public string MagicNumber { get; set; }
        public int Version { get; set; }
        public string CardType { get; set; }
        public string ExpiryDate { get; set; }
        public string VisaDate { get; set; }
        public int CurrentBalanceKurus { get; set; }
        public int MaxAllowedBalance { get; set; }
        public int ProcessOperationCounter { get; set; }
        public int BalanceOperationCounter { get; set; }
        public string LastTransactionDate { get; set; }
        public bool IsActive { get; set; }
    }
}