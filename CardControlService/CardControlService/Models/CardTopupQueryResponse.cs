using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace CardControlService.Models
{
    public class CardTopupQueryResponse
    {
        public bool Success { get; set; }
        public string Message { get; set; }

        public long RequestId { get; set; }
        public string CardUid { get; set; }
        public int AmountKurus { get; set; }
        public string Status { get; set; }
    }
}