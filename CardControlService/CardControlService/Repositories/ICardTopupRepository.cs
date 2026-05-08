using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    public interface ICardTopupRepository
    {
        ApiResponse AddTopupRequest(CardTopupRequest request);
        CardTopupQueryResponse GetTopupRequestByCardUid(string cardUid);
        ApiResponse UpdateTopupStatus(CardTopupStatusUpdateRequest request);

    }
}
