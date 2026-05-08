using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace CardControlService.Models
{
    public class CardPersonalizationStatusUpdateRequest
    {
        public long RequestId { get; set; }

        public string Status { get; set; }
    }
}