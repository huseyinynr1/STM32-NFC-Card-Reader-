using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Data;
using System.Data.SqlClient;
using System.Globalization;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    public class CardPersonalizationRepository : ICardPersonalizationRepository
    {
        private readonly string _connectionString;

        private const string STATUS_PENDING = "Pending";
        private const string STATUS_COMPLETED = "Completed";
        private const string STATUS_FAILED = "Failed";

        public CardPersonalizationRepository(string connectionString)
        {
            _connectionString = connectionString;
        }

        public ApiResponse AddPersonalizationRequest(CardPersonalizationCreateRequest request)
        {
            try
            {
                const string checkRequestQuery = @"
                    SELECT COUNT(1)
                    FROM dbo.CardPersonalizationRequests
                    WHERE RequestId = @RequestId";

                const string checkPendingCardQuery = @"
                    SELECT COUNT(1)
                    FROM dbo.CardPersonalizationRequests
                    WHERE LTRIM(RTRIM(CardUid)) = @CardUid
                      AND Status = @Status";

                const string insertQuery = @"
                    INSERT INTO dbo.CardPersonalizationRequests
                    (
                        RequestId,
                        CardUid,
                        MagicNumber,
                        Version,
                        CardType,
                        ExpiryDate,
                        InitialBalanceKurus,
                        MaxAllowedBalance,
                        VisaDate,
                        Status
                    )
                    VALUES
                    (
                        @RequestId,
                        @CardUid,
                        @MagicNumber,
                        @Version,
                        @CardType,
                        @ExpiryDate,
                        @InitialBalanceKurus,
                        @MaxAllowedBalance,
                        @VisaDate,
                        @Status
                    )";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    string trimmedCardUid = request.CardUid.Trim();
                    string trimmedMagicNumber = request.MagicNumber.Trim();

                    using (SqlCommand checkCommand = new SqlCommand(checkRequestQuery, connection))
                    {
                        checkCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;

                        int requestCount = Convert.ToInt32(checkCommand.ExecuteScalar());

                        if (requestCount > 0)
                        {
                            return new ApiResponse
                            {
                                Success = false,
                                Message = "Bu RequestId ile kart kişiselleştirme isteği zaten mevcut."
                            };
                        }
                    }

                    using (SqlCommand checkPendingCommand = new SqlCommand(checkPendingCardQuery, connection))
                    {
                        checkPendingCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = trimmedCardUid;
                        checkPendingCommand.Parameters.Add("@Status", SqlDbType.VarChar, 20).Value = STATUS_PENDING;

                        int pendingCount = Convert.ToInt32(checkPendingCommand.ExecuteScalar());

                        if (pendingCount > 0)
                        {
                            return new ApiResponse
                            {
                                Success = false,
                                Message = "Bu CardUid için zaten bekleyen kart kişiselleştirme isteği var."
                            };
                        }
                    }

                    using (SqlCommand insertCommand = new SqlCommand(insertQuery, connection))
                    {
                        insertCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;
                        insertCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = trimmedCardUid;
                        insertCommand.Parameters.Add("@MagicNumber", SqlDbType.NVarChar, 10).Value = trimmedMagicNumber;
                        insertCommand.Parameters.Add("@Version", SqlDbType.Int).Value = request.Version;
                        insertCommand.Parameters.Add("@CardType", SqlDbType.NVarChar, 30).Value = request.CardType.Trim();

                        insertCommand.Parameters.Add("@ExpiryDate", SqlDbType.Date).Value =
                            DateTime.ParseExact(request.ExpiryDate, "dd/MM/yyyy", CultureInfo.InvariantCulture);

                        insertCommand.Parameters.Add("@InitialBalanceKurus", SqlDbType.Int).Value = request.InitialBalanceKurus;
                        insertCommand.Parameters.Add("@MaxAllowedBalance", SqlDbType.Int).Value = request.MaxAllowedBalance;

                        insertCommand.Parameters.Add("@VisaDate", SqlDbType.Date).Value =
                            DateTime.ParseExact(request.VisaDate, "dd/MM/yyyy", CultureInfo.InvariantCulture);

                        insertCommand.Parameters.Add("@Status", SqlDbType.VarChar, 20).Value = STATUS_PENDING;

                        insertCommand.ExecuteNonQuery();
                    }

                    return new ApiResponse
                    {
                        Success = true,
                        Message = "Kart kişiselleştirme isteği başarıyla oluşturuldu. Status: Pending"
                    };
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
                    Message = ex.ToString()
                };
            }
        }

        public CardPersonalizationQueryResponse GetPersonalizationRequestByCardUid(string cardUid)
        {
            try
            {
                const string query = @"
                    SELECT TOP 1
                        Id,
                        RequestId,
                        CardUid,
                        MagicNumber,
                        Version,
                        CardType,
                        ExpiryDate,
                        InitialBalanceKurus,
                        MaxAllowedBalance,
                        VisaDate,
                        Status
                    FROM dbo.CardPersonalizationRequests
                    WHERE LTRIM(RTRIM(CardUid)) = @CardUid
                      AND Status = @Status
                    ORDER BY RequestId DESC";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    using (SqlCommand command = new SqlCommand(query, connection))
                    {
                        command.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = cardUid.Trim();
                        command.Parameters.Add("@Status", SqlDbType.VarChar, 20).Value = STATUS_PENDING;

                        using (SqlDataReader reader = command.ExecuteReader())
                        {
                            if (!reader.Read())
                            {
                                return new CardPersonalizationQueryResponse
                                {
                                    Success = false,
                                    Message = "Bu CardUid değerine ait bekleyen kart kişiselleştirme isteği bulunamadı."
                                };
                            }

                            int initialBalanceKurus = Convert.ToInt32(reader["InitialBalanceKurus"]);

                            return new CardPersonalizationQueryResponse
                            {
                                Success = true,
                                Message = "Bekleyen kart kişiselleştirme isteği bulundu.",

                                Id = Convert.ToInt64(reader["Id"]),
                                RequestId = Convert.ToInt64(reader["RequestId"]),
                                CardUid = reader["CardUid"].ToString(),

                                MagicNumber = reader["MagicNumber"].ToString(),
                                Version = Convert.ToInt32(reader["Version"]),
                                CardType = reader["CardType"].ToString(),

                                ExpiryDate = Convert.ToDateTime(reader["ExpiryDate"]).ToString("dd/MM/yyyy"),

                                InitialBalanceKurus = initialBalanceKurus,
                                CurrentBalanceKurus = initialBalanceKurus,
                                MaxAllowedBalance = Convert.ToInt32(reader["MaxAllowedBalance"]),

                                VisaDate = Convert.ToDateTime(reader["VisaDate"]).ToString("dd/MM/yyyy"),
                                Status = reader["Status"].ToString()
                            };
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                return new CardPersonalizationQueryResponse
                {
                    Success = false,
                    Message = ex.ToString()
                };
            }
        }

        public ApiResponse UpdatePersonalizationStatus(CardPersonalizationStatusUpdateRequest request)
        {
            try
            {
                string requestedStatus = request.Status.Trim();

                if (!IsValidStatus(requestedStatus))
                {
                    return new ApiResponse
                    {
                        Success = false,
                        Message = "Geçersiz status değeri. Sadece Pending, Completed, Failed kullanılabilir."
                    };
                }

                const string selectRequestQuery = @"
                    SELECT
                        RequestId,
                        CardUid,
                        MagicNumber,
                        Version,
                        CardType,
                        ExpiryDate,
                        InitialBalanceKurus,
                        MaxAllowedBalance,
                        VisaDate,
                        Status
                    FROM dbo.CardPersonalizationRequests
                    WHERE RequestId = @RequestId";

                const string updateStatusQuery = @"
                    UPDATE dbo.CardPersonalizationRequests
                    SET Status = @Status
                    WHERE RequestId = @RequestId";

                const string checkCardQuery = @"
                    SELECT COUNT(1)
                    FROM dbo.Cards
                    WHERE LTRIM(RTRIM(CardUid)) = @CardUid";

                const string insertCardQuery = @"
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
                        0,
                        0
                    )";

                const string updateCardQuery = @"
                    UPDATE dbo.Cards
                    SET
                        MagicNumber = @MagicNumber,
                        Version = @Version,
                        CardType = @CardType,
                        ExpiryDate = @ExpiryDate,
                        VisaDate = @VisaDate,
                        CurrentBalanceKurus = @CurrentBalanceKurus,
                        MaxAllowedBalance = @MaxAllowedBalance
                    WHERE LTRIM(RTRIM(CardUid)) = @CardUid";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    using (SqlTransaction transaction = connection.BeginTransaction())
                    {
                        try
                        {
                            string cardUid;
                            string magicNumber;
                            int version;
                            string cardType;
                            DateTime expiryDate;
                            int initialBalanceKurus;
                            int maxAllowedBalance;
                            DateTime visaDate;

                            using (SqlCommand selectCommand = new SqlCommand(selectRequestQuery, connection, transaction))
                            {
                                selectCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;

                                using (SqlDataReader reader = selectCommand.ExecuteReader())
                                {
                                    if (!reader.Read())
                                    {
                                        transaction.Commit();

                                        return new ApiResponse
                                        {
                                            Success = false,
                                            Message = "Bu RequestId değerine ait kart kişiselleştirme isteği bulunamadı."
                                        };
                                    }

                                    cardUid = reader["CardUid"].ToString().Trim();
                                    magicNumber = reader["MagicNumber"].ToString().Trim();
                                    version = Convert.ToInt32(reader["Version"]);
                                    cardType = reader["CardType"].ToString();
                                    expiryDate = Convert.ToDateTime(reader["ExpiryDate"]);
                                    initialBalanceKurus = Convert.ToInt32(reader["InitialBalanceKurus"]);
                                    maxAllowedBalance = Convert.ToInt32(reader["MaxAllowedBalance"]);
                                    visaDate = Convert.ToDateTime(reader["VisaDate"]);
                                }
                            }

                            using (SqlCommand updateStatusCommand = new SqlCommand(updateStatusQuery, connection, transaction))
                            {
                                updateStatusCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;
                                updateStatusCommand.Parameters.Add("@Status", SqlDbType.VarChar, 20).Value = requestedStatus;

                                int affectedRows = updateStatusCommand.ExecuteNonQuery();

                                if (affectedRows == 0)
                                {
                                    transaction.Rollback();

                                    return new ApiResponse
                                    {
                                        Success = false,
                                        Message = "Kart kişiselleştirme status değeri güncellenemedi."
                                    };
                                }
                            }

                            if (requestedStatus == STATUS_COMPLETED)
                            {
                                int cardCount;

                                using (SqlCommand checkCardCommand = new SqlCommand(checkCardQuery, connection, transaction))
                                {
                                    checkCardCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = cardUid;
                                    cardCount = Convert.ToInt32(checkCardCommand.ExecuteScalar());
                                }

                                string cardQuery = cardCount == 0 ? insertCardQuery : updateCardQuery;

                                using (SqlCommand cardCommand = new SqlCommand(cardQuery, connection, transaction))
                                {
                                    cardCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = cardUid;
                                    cardCommand.Parameters.Add("@MagicNumber", SqlDbType.NVarChar, 10).Value = magicNumber;
                                    cardCommand.Parameters.Add("@Version", SqlDbType.Int).Value = version;
                                    cardCommand.Parameters.Add("@CardType", SqlDbType.NVarChar, 30).Value = cardType;
                                    cardCommand.Parameters.Add("@ExpiryDate", SqlDbType.Date).Value = expiryDate;
                                    cardCommand.Parameters.Add("@VisaDate", SqlDbType.Date).Value = visaDate;
                                    cardCommand.Parameters.Add("@CurrentBalanceKurus", SqlDbType.Int).Value = initialBalanceKurus;
                                    cardCommand.Parameters.Add("@MaxAllowedBalance", SqlDbType.Int).Value = maxAllowedBalance;

                                    cardCommand.ExecuteNonQuery();
                                }
                            }

                            transaction.Commit();

                            return new ApiResponse
                            {
                                Success = true,
                                Message = requestedStatus == STATUS_COMPLETED
                                    ? "Kart kişiselleştirme status değeri güncellendi ve dbo.Cards kaydı oluşturuldu/güncellendi."
                                    : "Kart kişiselleştirme status değeri başarıyla güncellendi."
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
            catch (Exception ex)
            {
                return new ApiResponse
                {
                    Success = false,
                    Message = ex.ToString()
                };
            }
        }

        private bool IsValidStatus(string status)
        {
            if (string.IsNullOrWhiteSpace(status))
                return false;

            status = status.Trim();

            return status == STATUS_PENDING ||
                   status == STATUS_COMPLETED ||
                   status == STATUS_FAILED;
        }
    }
}