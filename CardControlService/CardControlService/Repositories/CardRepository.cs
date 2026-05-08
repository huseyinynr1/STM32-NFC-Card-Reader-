using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.Data;
using System.Globalization;
using System.Linq;
using System.Web;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    /// <summary>
    /// CardUid zaten var mı kontrol ediyor
    //yoksa dbo.Cards tablosuna insert atıyor
    //tarihleri string’den DATE tipine çeviriyor
    //hata varsa mesaj döndürüyor
    /// </summary>
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
    //  "processOperationCounter": 0,
    //  "balanceOperationCounter": 0
    //}
    /// </summary>
    public class CardRepository : ICardRepository
    {
        private readonly string _connectionString;

        public CardRepository(string connectionString)
        {
            _connectionString = connectionString;
        }

        public ApiResponse AddCard(CardCreateRequest request)
        {
            try
            {
                const string checkQuery = @"
                    SELECT COUNT(1)
                    FROM dbo.Cards
                    WHERE CardUid = @CardUid";

                const string insertQuery = @"
                    INSERT INTO dbo.Cards
                    (
                        CardUid,
                        MagicNumber,
                        Version,
                        CardType,
                        ExpiryDate,
                        VisaDate,
                        CurrentBalanceKurus,
                        MaxAllowedBalance,
                        ProcessOperationCounter,
                        BalanceOperationCounter
                    )
                    VALUES
                    (
                        @CardUid,
                        @MagicNumber,
                        @Version,
                        @CardType,
                        @ExpiryDate,
                        @VisaDate,
                        @CurrentBalanceKurus,
                        @MaxAllowedBalance,
                        @ProcessOperationCounter,
                        @BalanceOperationCounter
                    )";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    using (SqlTransaction transaction = connection.BeginTransaction())
                    {
                        try
                        {
                            using (SqlCommand checkCommand = new SqlCommand(checkQuery, connection, transaction))
                            {
                                checkCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = request.CardUid.Trim();

                                int cardCount = Convert.ToInt32(checkCommand.ExecuteScalar());

                                if (cardCount > 0)
                                {
                                    transaction.Commit();

                                    return new ApiResponse
                                    {
                                        Success = false,
                                        Message = "Bu CardUid ile kayıt zaten mevcut."
                                    };
                                }
                            }

                            using (SqlCommand insertCommand = new SqlCommand(insertQuery, connection, transaction))
                            {
                                insertCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = request.CardUid.Trim();
                                insertCommand.Parameters.Add("@MagicNumber", SqlDbType.NVarChar, 2).Value =request.MagicNumber.Trim().ToUpperInvariant();
                                insertCommand.Parameters.Add("@Version", SqlDbType.Int).Value = request.Version;
                                insertCommand.Parameters.Add("@CardType", SqlDbType.NVarChar, 30).Value =request.CardType.Trim().ToUpperInvariant();
                                insertCommand.Parameters.Add("@ExpiryDate", SqlDbType.Date).Value =
                                    DateTime.ParseExact(request.ExpiryDate, "dd/MM/yyyy", CultureInfo.InvariantCulture);
                                insertCommand.Parameters.Add("@VisaDate", SqlDbType.Date).Value =
                                    DateTime.ParseExact(request.VisaDate, "dd/MM/yyyy", CultureInfo.InvariantCulture);
                                insertCommand.Parameters.Add("@CurrentBalanceKurus", SqlDbType.Int).Value = request.CurrentBalanceKurus;
                                insertCommand.Parameters.Add("@MaxAllowedBalance", SqlDbType.Int).Value = request.MaxAllowedBalance;
                                insertCommand.Parameters.Add("@ProcessOperationCounter", SqlDbType.Int).Value = request.ProcessOperationCounter;
                                insertCommand.Parameters.Add("@BalanceOperationCounter", SqlDbType.Int).Value = request.BalanceOperationCounter;

                                insertCommand.ExecuteNonQuery();
                            }

                            transaction.Commit();

                            return new ApiResponse
                            {
                                Success = true,
                                Message = "Kart kaydı başarıyla eklendi."
                            };
                        }
                        catch
                        {
                            transaction.Rollback();
                            throw;
                        }
                    }
                }
            }
            catch (FormatException)
            {
                return new ApiResponse
                {
                    Success = false,
                    Message = "Tarih formatı hatalı. Beklenen format: dd/MM/yyyy"
                };
            }
            catch (Exception ex)
            {
                return new ApiResponse
                {
                    Success = false,
                    Message = ex.Message
                };
            }
        }
        public CardQueryResponse GetCardByUid(string cardUid)
        {
            try
            {
                const string query = @"
                    SELECT
                        CardUid,
                        MagicNumber,
                        Version,
                        CardType,
                        ExpiryDate,
                        VisaDate,
                        CurrentBalanceKurus,
                        MaxAllowedBalance,
                        ProcessOperationCounter,
                        BalanceOperationCounter,
                        LastTransactionDate,
                        IsActive
                    FROM dbo.Cards
                    WHERE CardUid = @CardUid";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    using (SqlCommand command = new SqlCommand(query, connection))
                    {
                        command.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = cardUid.Trim();

                        using (SqlDataReader reader = command.ExecuteReader())
                        {
                            if (!reader.Read())
                            {
                                return new CardQueryResponse
                                {
                                    Success = false,
                                    Message = "Kart bulunamadı."
                                };
                            }

                            return new CardQueryResponse
                            {
                                Success = true,
                                Message = "Kart bulundu.",
                                CardUid = reader["CardUid"].ToString(),
                                MagicNumber = reader["MagicNumber"].ToString(),
                                Version = Convert.ToInt32(reader["Version"]),
                                CardType = reader["CardType"].ToString(),
                                ExpiryDate = Convert.ToDateTime(reader["ExpiryDate"]).ToString("dd/MM/yyyy"),
                                VisaDate = Convert.ToDateTime(reader["VisaDate"]).ToString("dd/MM/yyyy"),
                                CurrentBalanceKurus = Convert.ToInt32(reader["CurrentBalanceKurus"]),
                                MaxAllowedBalance = Convert.ToInt32(reader["MaxAllowedBalance"]),
                                ProcessOperationCounter = Convert.ToInt32(reader["ProcessOperationCounter"]),
                                BalanceOperationCounter = Convert.ToInt32(reader["BalanceOperationCounter"]),
                                LastTransactionDate = reader["LastTransactionDate"] == DBNull.Value
                                    ? null
                                    : Convert.ToDateTime(reader["LastTransactionDate"]).ToString("dd/MM/yyyy HH:mm:ss"),
                                IsActive = Convert.ToBoolean(reader["IsActive"])
                            };
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                return new CardQueryResponse
                {
                    Success = false,
                    Message = ex.ToString()
                };
            }
        }
    }
}