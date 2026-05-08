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
    /// <summary>
    /// https://localhost:44395/api/topup/add
    /// Body
    //{
        //"RequestId": 17,
        //"CardUid": "AABBCCDD",
        //"AmountKurus": 5000
    //}
/// </summary>
[RoutePrefix("api/topup")]
    public class CardTopupController : ApiController
    {
        private readonly ICardTopupRepository _repository;

        public CardTopupController()
        {
            string connectionString = @"Server=DESKTOP-M1MR0GD\MSSQLSERVER2022;Database=cardControlDb;Trusted_Connection=True;";
            _repository = new CardTopupRepository(connectionString);
        }

        [HttpPost]
        [Route("add")]
        public IHttpActionResult Add([FromBody] CardTopupRequest request)
        {
            if (request == null)
                return BadRequest("İstek verisi boş.");

            if (string.IsNullOrWhiteSpace(request.CardUid))
                return BadRequest("CardUid zorunludur.");

            if (request.AmountKurus <= 0)
                return BadRequest("AmountKurus 0'dan büyük olmalıdır.");

            var result = _repository.AddTopupRequest(request);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }

        /// https://localhost:44395/api/topup/getbyuid?
      

        [HttpGet]
        [Route("getbyuid")]
        public IHttpActionResult GetByUid(string cardUid)
        {
            if (string.IsNullOrWhiteSpace(cardUid))
                return BadRequest("CardUid zorunludur.");

            var result = _repository.GetTopupRequestByCardUid(cardUid);

            return Ok(result);
        }

        /// https://localhost:44395/api/topup/update-status
        /// Body
        // {
        //"RequestId": 17,
        //"Status": "Completed"
        // }


        [HttpPost]
        [Route("update-status")]
        public IHttpActionResult UpdateStatus([FromBody] CardTopupStatusUpdateRequest request)
        {
            if (request == null)
                return BadRequest("İstek verisi boş.");

            if (request.RequestId <= 0)
                return BadRequest("RequestId zorunludur.");

            if (string.IsNullOrWhiteSpace(request.Status))
                return BadRequest("Status zorunludur.");

            var result = _repository.UpdateTopupStatus(request);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }
    }
}
