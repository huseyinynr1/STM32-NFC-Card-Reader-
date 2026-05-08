using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.Data;
using System.Linq;
using System.Web;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    /// <summary>
    /// https://localhost:44395/api/topup/add
    /// Body
    /// {
    //  "requestId": 17,
    //  "cardUid": "AABBCCDD",
    //  "amountKurus": 5000,
    //  "status": 0
    //}
    /// </summary>
    public class CardTopupRepository : ICardTopupRepository
    {
        private readonly string _connectionString;

        private const string STATUS_PENDING = "Pending";
        private const string STATUS_COMPLETED = "Completed";
        private const string STATUS_FAILED = "Failed";

        public CardTopupRepository(string connectionString)
        {
            _connectionString = connectionString;
        }

        public ApiResponse AddTopupRequest(CardTopupRequest request)
        {
            try
            {
                const string getCardQuery = @"
            SELECT
                CurrentBalanceKurus,
                MaxAllowedBalance
            FROM dbo.Cards
            WHERE LTRIM(RTRIM(CardUid)) = @CardUid";

                const string insertQuery = @"
            INSERT INTO dbo.CardTopupRequests
            (
                RequestId,
                CardUid,
                AmountKurus,
                Status,
                CreatedAt
            )
            VALUES
            (
                @RequestId,
                @CardUid,
                @AmountKurus,
                @Status,
                GETDATE()
            )";

                const string checkRequestQuery = @"
            SELECT COUNT(1)
            FROM dbo.CardTopupRequests
            WHERE RequestId = @RequestId";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    string trimmedCardUid = request.CardUid.Trim();

                    int currentBalanceKurus;
                    int maxAllowedBalance;

                    using (SqlCommand cardCommand = new SqlCommand(getCardQuery, connection))
                    {
                        cardCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = trimmedCardUid;

                        using (SqlDataReader reader = cardCommand.ExecuteReader())
                        {
                            if (!reader.Read())
                            {
                                return new ApiResponse
                                {
                                    Success = false,
                                    Message = "Kart bulunamadı."
                                };
                            }

                            currentBalanceKurus = Convert.ToInt32(reader["CurrentBalanceKurus"]);
                            maxAllowedBalance = Convert.ToInt32(reader["MaxAllowedBalance"]);
                        }
                    }

                    if (request.AmountKurus <= 0)
                    {
                        return new ApiResponse
                        {
                            Success = false,
                            Message = "Yüklenecek tutar sıfırdan büyük olmalıdır."
                        };
                    }

                    if (currentBalanceKurus + request.AmountKurus > maxAllowedBalance)
                    {
                        return new ApiResponse
                        {
                            Success = false,
                            Message = "Bakiye yükleme işlemi maksimum bakiye limitini aşıyor."
                        };
                    }

                    using (SqlCommand checkCommand = new SqlCommand(checkRequestQuery, connection))
                    {
                        checkCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;

                        int requestCount = Convert.ToInt32(checkCommand.ExecuteScalar());

                        if (requestCount > 0)
                        {
                            return new ApiResponse
                            {
                                Success = false,
                                Message = "Bu RequestId ile bakiye yükleme isteği zaten mevcut."
                            };
                        }
                    }

                    using (SqlCommand insertCommand = new SqlCommand(insertQuery, connection))
                    {
                        insertCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;
                        insertCommand.Parameters.Add("@CardUid", SqlDbType.VarChar, 50).Value = trimmedCardUid;
                        insertCommand.Parameters.Add("@AmountKurus", SqlDbType.Int).Value = request.AmountKurus;
                        insertCommand.Parameters.Add("@Status", SqlDbType.VarChar, 20).Value = STATUS_PENDING;

                        insertCommand.ExecuteNonQuery();
                    }

                    return new ApiResponse
                    {
                        Success = true,
                        Message = "Bakiye yükleme isteği başarıyla oluşturuldu. Status: Pending"
                    };
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

        public CardTopupQueryResponse GetTopupRequestByCardUid(string cardUid)
        {
            try
            {
                const string query = @"
                    SELECT TOP 1
                        RequestId,
                        CardUid,
                        AmountKurus,
                        Status
                    FROM dbo.CardTopupRequests
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
                                return new CardTopupQueryResponse
                                {
                                    Success = false,
                                    Message = "Bu kart ID'sine ait bekleyen bakiye yükleme isteği bulunamadı."
                                };
                            }

                            return new CardTopupQueryResponse
                            {
                                Success = true,
                                Message = "Bekleyen bakiye yükleme isteği bulundu.",
                                RequestId = Convert.ToInt64(reader["RequestId"]),
                                CardUid = reader["CardUid"].ToString(),
                                AmountKurus = Convert.ToInt32(reader["AmountKurus"]),
                                Status = reader["Status"].ToString()
                            };
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                return new CardTopupQueryResponse
                {
                    Success = false,
                    Message = ex.ToString()
                };
            }
        }

        public ApiResponse UpdateTopupStatus(CardTopupStatusUpdateRequest request)
        {
            try
            {
                if (!IsValidStatus(request.Status))
                {
                    return new ApiResponse
                    {
                        Success = false,
                        Message = "Geçersiz status değeri. Sadece Pending, Completed, Failed kullanılabilir."
                    };
                }

                const string checkRequestQuery = @"
                    SELECT COUNT(1)
                    FROM dbo.CardTopupRequests
                    WHERE RequestId = @RequestId";

                const string updateQuery = @"
                    UPDATE dbo.CardTopupRequests
                    SET
                        Status = @Status,
                        CompletedAt = CASE
                            WHEN @Status = 'Completed' THEN GETDATE()
                            ELSE CompletedAt
                        END,
                        ErrorMessage = CASE
                            WHEN @Status = 'Failed' THEN 'STM32 tarafında bakiye yükleme işlemi başarısız oldu.'
                            ELSE NULL
                        END
                    WHERE RequestId = @RequestId";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    using (SqlCommand checkCommand = new SqlCommand(checkRequestQuery, connection))
                    {
                        checkCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;

                        int requestCount = Convert.ToInt32(checkCommand.ExecuteScalar());

                        if (requestCount == 0)
                        {
                            return new ApiResponse
                            {
                                Success = false,
                                Message = "Bu RequestId değerine ait bakiye yükleme isteği bulunamadı."
                            };
                        }
                    }

                    using (SqlCommand updateCommand = new SqlCommand(updateQuery, connection))
                    {
                        updateCommand.Parameters.Add("@RequestId", SqlDbType.BigInt).Value = request.RequestId;
                        updateCommand.Parameters.Add("@Status", SqlDbType.VarChar, 20).Value = request.Status.Trim();

                        int affectedRows = updateCommand.ExecuteNonQuery();

                        if (affectedRows == 0)
                        {
                            return new ApiResponse
                            {
                                Success = false,
                                Message = "Status güncellenemedi."
                            };
                        }

                        return new ApiResponse
                        {
                            Success = true,
                            Message = "Bakiye yükleme status değeri başarıyla güncellendi."
                        };
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
