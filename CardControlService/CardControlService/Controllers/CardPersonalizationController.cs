using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using CardControlService.Models;
using CardControlService.Repositories;

namespace CardControlService.Controllers
{
    [RoutePrefix("api/personalization")]
    public class CardPersonalizationController : ApiController
    {
        private readonly ICardPersonalizationRepository _repository;

        public CardPersonalizationController()
        {
            string connectionString = @"Server=DESKTOP-M1MR0GD\MSSQLSERVER2022;Database=cardControlDb;Trusted_Connection=True;";
            _repository = new CardPersonalizationRepository(connectionString);
        }

        /*
            POST:
            https://localhost:44395/api/personalization/add

            Body:
                {
                    "RequestId": 1,
                    "CardUid": "AABBCCDD",
                    "MagicNumber": "HY",
                    "Version": 1,
                    "CardType": "Full Fare Card",
                    "ExpiryDate": "01/01/2027",
                    "InitialBalanceKurus": 0,
                    "MaxAllowedBalance": 30000,
                    "VisaDate": "01/01/2027"
                }
        */
        [HttpPost]
        [Route("add")]
        public IHttpActionResult Add([FromBody] CardPersonalizationCreateRequest request)
        {
            if (request == null)
                return BadRequest("İstek verisi boş.");

            if (string.IsNullOrWhiteSpace(request.MagicNumber))
                return BadRequest("MagicNumber zorunludur.");

            if (request.MaxAllowedBalance <= 0)
                return BadRequest("MaxAllowedBalance 0'dan büyük olmalıdır.");

            if (request.RequestId <= 0)
                return BadRequest("RequestId zorunludur.");

            if (string.IsNullOrWhiteSpace(request.CardUid))
                return BadRequest("CardUid zorunludur.");

            if (request.Version <= 0)
                return BadRequest("Version 0'dan büyük olmalıdır.");

            if (string.IsNullOrWhiteSpace(request.CardType))
                return BadRequest("CardType zorunludur.");

            if (string.IsNullOrWhiteSpace(request.ExpiryDate))
                return BadRequest("ExpiryDate zorunludur.");

            if (string.IsNullOrWhiteSpace(request.VisaDate))
                return BadRequest("VisaDate zorunludur.");

            if (request.InitialBalanceKurus < 0)
                return BadRequest("InitialBalanceKurus negatif olamaz.");

            var result = _repository.AddPersonalizationRequest(request);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }

        /*
            GET:
            https://localhost:44395/api/personalization/getbyuid?cardUid=AABBCCDD
        */
        [HttpGet]
        [Route("getbyuid")]
        public IHttpActionResult GetByUid(string cardUid)
        {
            if (string.IsNullOrWhiteSpace(cardUid))
                return BadRequest("CardUid zorunludur.");

            var result = _repository.GetPersonalizationRequestByCardUid(cardUid);

            return Ok(result);
        }

        /*
            POST:
            https://localhost:44395/api/personalization/update-status

            Body:
            {
                "RequestId": 1,
                "Status": "Completed"
            }
        */
        [HttpPost]
        [Route("update-status")]
        public IHttpActionResult UpdateStatus([FromBody] CardPersonalizationStatusUpdateRequest request)
        {
            if (request == null)
                return BadRequest("İstek verisi boş.");

            if (request.RequestId <= 0)
                return BadRequest("RequestId zorunludur.");

            if (string.IsNullOrWhiteSpace(request.Status))
                return BadRequest("Status zorunludur.");

            var result = _repository.UpdatePersonalizationStatus(request);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }
    }
}