using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.Data;
using System.Linq;
using System.Web;
using CardControlService.Models;

namespace CardControlService.Repositories
{
    public class EmbeddedCardRepository : IEmbeddedCardRepository
    {
        private readonly string _connectionString;

        public EmbeddedCardRepository(string connectionString)
        {
            _connectionString = connectionString;
        }

        public EmbeddedCardResponse Add(EmbeddedCardRequest request)
        {
            try
            {
                const string checkQuery = @"
                    SELECT COUNT(1)
                    FROM BOOKINGPLAKA
                    WHERE IDBOOKING = @IDBOOKING
                      AND PLAKA = @PLAKA
                      AND TIP = @TIP";

                const string insertQuery = @"
                    INSERT INTO BOOKINGPLAKA (IDBOOKING, PLAKA, TIP, KULID, TARSAAT)
                    VALUES (@IDBOOKING, @PLAKA, @TIP, @KULID, CURRENT_TIMESTAMP)";

                using (SqlConnection connection = new SqlConnection(_connectionString))
                {
                    connection.Open();

                    using (SqlTransaction transaction = connection.BeginTransaction())
                    {
                        try
                        {
                            using (SqlCommand checkCommand = new SqlCommand(checkQuery, connection, transaction))
                            {
                                checkCommand.Parameters.Add("@IDBOOKING", SqlDbType.Decimal).Value = request.IdBooking;
                                checkCommand.Parameters.Add("@PLAKA", SqlDbType.VarChar).Value = request.Plate.Trim();
                                checkCommand.Parameters.Add("@TIP", SqlDbType.Int).Value = request.Tip;

                                int count = Convert.ToInt32(checkCommand.ExecuteScalar());

                                if (count > 0)
                                {
                                    transaction.Commit();

                                    return new EmbeddedCardResponse
                                    {
                                        Success = false,
                                        Message = "Bu kayıt zaten mevcut."
                                    };
                                }
                            }

                            using (SqlCommand insertCommand = new SqlCommand(insertQuery, connection, transaction))
                            {
                                insertCommand.Parameters.Add("@IDBOOKING", SqlDbType.Decimal).Value = request.IdBooking;
                                insertCommand.Parameters.Add("@PLAKA", SqlDbType.VarChar).Value = request.Plate.Trim();
                                insertCommand.Parameters.Add("@TIP", SqlDbType.Int).Value = request.Tip;
                                insertCommand.Parameters.Add("@KULID", SqlDbType.SmallInt).Value = request.KulId;

                                insertCommand.ExecuteNonQuery();
                            }

                            transaction.Commit();

                            return new EmbeddedCardResponse
                            {
                                Success = true,
                                Message = "Kayıt başarılı."
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
                return new EmbeddedCardResponse
                {
                    Success = false,
                    Message = ex.Message
                };
            }
        }
    }
}