using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    public interface ICardPersonalizationRepository
    {
        ApiResponse AddPersonalizationRequest(CardPersonalizationCreateRequest request);

        CardPersonalizationQueryResponse GetPersonalizationRequestByCardUid(string cardUid);

        ApiResponse UpdatePersonalizationStatus(CardPersonalizationStatusUpdateRequest request);
    }
}