using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    public interface ICardRepository
    {
        ApiResponse AddCard(CardCreateRequest request);

        CardQueryResponse GetCardByUid(string cardUid);
    }
}
