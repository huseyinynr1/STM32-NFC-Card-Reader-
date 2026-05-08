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
    
    [RoutePrefix("api/cards")]
    public class CardsController : ApiController    
    {

        private readonly ICardRepository _repository;

        public CardsController()
        {
            string connectionString = @"Server=DESKTOP-M1MR0GD\MSSQLSERVER2022;Database=cardControlDb;Trusted_Connection=True;";
            _repository = new CardRepository(connectionString);
        }

        /// <summary>
        /// https://localhost:44395/api/cards/add
        //   {
        //  "cardUid": "AABBCCDDEEE",
        //  "magicNumber": "123456",
        //  "version": 1,
        //  "cardType": "Tam Kart",
        //  "expiryDate": "01/01/2027",
        //  "visaDate": "01/01/2027",
        //  "currentBalanceKurus": 0,
        //  "maxAllowedBalance" : 30000, 
        //  "processOperationCounter": 0,
        //  "balanceOperationCounter": 0
        //}
        /// </summary>
        [HttpPost]
        [Route("add")]
        public IHttpActionResult Add([FromBody] CardCreateRequest request)
        {
            if (request == null)
                return BadRequest("İstek verisi boş.");

            if (string.IsNullOrWhiteSpace(request.CardUid))
                return BadRequest("CardUid zorunludur.");

            var result = _repository.AddCard(request);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }

        /// <summary>
        /// GET https://localhost:44395/api/cards/getbyuid?cardUid=AABBCCDD
        /// </summary>
        /// <param name="cardUid"></param>
        /// <returns></returns>
        [HttpGet]
        [Route("getbyuid")]
        public IHttpActionResult GetByUid(string cardUid)
        {
            if (string.IsNullOrWhiteSpace(cardUid))
                return BadRequest("cardUid zorunludur.");

            var result = _repository.GetCardByUid(cardUid);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }

    }


}
