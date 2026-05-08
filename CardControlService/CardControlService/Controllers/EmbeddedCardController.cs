using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using CardControlService.Models;
using CardControlService.Repositories;
using Newtonsoft.Json.Linq;

namespace CardControlService.Controllers
{
    [RoutePrefix("api/embeddedcard")]
    public class EmbeddedCardController : ApiController
    {
        private readonly IEmbeddedCardRepository _repository;

        public EmbeddedCardController()
        {
            string connectionString = @"Server=DESKTOP-M1MR0GD\MSSQLSERVER2022;Database=cardControlDb;Trusted_Connection=True;";

            _repository = new EmbeddedCardRepository(connectionString);
        }

        [HttpPost]
        [Route("add")]
        public IHttpActionResult AddCard([FromBody] EmbeddedCardRequest request)
        {
            if (request == null)
                return BadRequest("İstek verisi boş.");

            if (string.IsNullOrWhiteSpace(request.Plate))
                return BadRequest("Plate alanı zorunludur.");

            var result = _repository.Add(request);

            if (result.Success)
                return Ok(result);

            return BadRequest(result.Message);
        }
    }
}
