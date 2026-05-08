using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace CardControlService.Models
{
    public class CardTopupRequest
    {
        public long RequestId { get; set; }
        public string CardUid { get; set; }
        public int AmountKurus { get; set; }
        public string Status { get; set; }
    }
}