using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace CardControlService.Models
{
    public class EmbeddedCardRequest
    {
        public decimal IdBooking { get; set; }
        public string Plate { get; set; }
        public int Tip { get; set; }
        public short KulId { get; set; }
    }
}