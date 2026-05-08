#ifndef HELPER_FUNCTION_H_
#define HELPER_FUNCTION_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "rtc_ds3231.h"

// GET ve POST endpoint.
#define GET_NEW_CARD_INFO_URL  					  "https://seq-taylor-hockey-participated.trycloudflare.com/api/personalization/getbyuid"
#define GET_TOPUP_REQUEST_INFO_URL  			  "https://seq-taylor-hockey-participated.trycloudflare.com/api/topup/getbyuid"
#define POST_PERSONALIZATION_REQUEST_RESPONSE_URL "https://seq-taylor-hockey-participated.trycloudflare.com/api/personalization/update-status"
#define POST_TOPUP_REQUEST_RESPONSE_URL 		  "https://seq-taylor-hockey-participated.trycloudflare.com/api/topup/update-status"

// HTTP işlem durumları.
typedef enum
{
    HTTP_REQ_NEW_CARD_INFO = 0,
    HTTP_REQ_TOPUP_CHECK,
	HTTP_REQ_PERSONALIZATION_RESULT_POST,
	HTTP_REQ_TOPUP_RESULT_POST
}http_request_type;

// Kart işlemleri sonuç durumu.
typedef enum
{
	Pending,
	Completed,
	Failed
}http_status_value_type;

// Kart bilgileri için struct nesnesi.
typedef struct{
	uint16_t magic_number;
	uint8_t version;
	uint8_t card_type;
	uint8_t uid[4];
	uint32_t operation_counter;
	uint16_t expiry_date;
	uint32_t balance;
	uint32_t operation_counter_balance;
	uint16_t visa_date;
	uint16_t current_date;
	uint32_t current_time;
}card_info_helper;

// Kart ücret bilgileri için struct nesnesi.
typedef struct
{
	uint16_t fare;
	uint32_t balance;
	uint32_t loaded_amount;
	uint8_t status;
}card_balance_info_helper;

// POST işlem sonucu durumları.
typedef enum{
	Post_Result_Start_Value = 0,
	Post_Result_Successfull,
	Post_Result_Invalid_Json,
	Post_Result_DB_Error,
	Post_Result_Server_Error,
	Post_Result_Unknown_Error,
}http_post_result;

// HTTP isteği bilgileri için struct nesnesi.
typedef struct
{
	uint8_t uid[4];
	uint32_t request_id;
	http_request_type request_type;
	http_status_value_type status_type;
}http_request_helper;

// Yeni kart bilgilerini tutucak struct nesnesi.
typedef struct
{
	uint32_t request_id;
	uint8_t magic_number[2];
	uint8_t version;
	uint8_t card_type;
	uint16_t expiry_date;
	uint32_t balance;
	uint32_t max_allowed_balance;
	uint16_t visa_date;
}new_card_response_helper;

// Kart bakiye yükleme isteği bilgilerini tutucak struct nesnesi.
typedef struct
{
	bool http_process_status;
	bool has_topup;
	uint32_t amount;
	uint32_t request_id;
	http_status_value_type status;
}topup_response_helper;

// Post işlemi sonucu durumu için struct nesnesi.
typedef struct{
	bool http_process_status;
	http_post_result result;
}post_response_helper;

// Yeni kart bilgilerini kartın bloklarına yazma işlemi sonuç durumu db'e göndermek için json dönüşümü.
void Helper_Conver_to_Json_Request_Result(http_request_helper* result,char* arr, size_t size);

// API'den gelen işlemlerin ilerleyiş durumunu int'den string'e çevirme.
const char* Helper_Get_HTTP_Status_Value_to_String(uint8_t type);

// GET işlemi ile gelen yeni kart bilgileriresponse'u değerlere ayrıştırma.
void Helper_Json_Convert_to_New_Card_Info(char* arr, new_card_response_helper* new_card);

// GET işlemi ile gelen yeni bakiye bilgileri response'u değerlere ayrıştırma.
void Helper_Json_Convert_to_New_Balance(char* arr, topup_response_helper* response);

const char* Helper_Get_Card_Type_Name(uint8_t type);

// Fonksiyona gelen dizideki string'e göre int sonucu dön.
uint8_t Helper_Get_Card_Type_Value(const char* arr);

// Fonksiyona gelen dizideki string'e göre int sonucu dön.
http_status_value_type Helper_Get_Progress_Status_Value(const char* arr);

//Fonksiyona gelecek dizideki tarih değerini ayrıştır,
uint16_t Helper_Get_Date_Value(char* arr);

// Decimal tipinde kart id'yi string'e çevirme.
void Helper_Get_Card_ID_String(char* arr, size_t size, uint8_t* id, uint8_t id_len);

// Kart tarih bilgilerini string'e çevirme.
void Helper_Get_Card_Date_To_String(uint16_t date, char* buffer, uint8_t size);

// Kart zaman bilgilerini string'e çevirme.
void Helper_Get_Card_Time_To_String(uint32_t time, char* buffer, uint8_t size);

// Tarih değerini binary değere çevirme.
uint16_t Helper_Date_Packing(uint8_t year, uint8_t month, uint8_t day);

// Zaman değerini binary değere çevirme.
uint32_t Helper_Time_Packing(uint8_t hour, uint8_t minute, uint8_t second);

// Kart bloğundaki verilerin crc değerinin hesaplanması.
uint16_t Helper_Calculate_Crc16(uint8_t* data, uint8_t len);

// Buffer ile gelen değerlerin 0x00'mı kontrolü.
bool Helper_IsAllZero(uint8_t* buffer, uint8_t buffer_len);

// Buffer ile gelen değerlerin 0xFF'mi kontrolü.
bool Helper_IsAllFF(uint8_t* buffer, uint8_t buffer_len);

// GET isteği yapmadan önce, GET isteğinin neye göre yapılacağına dair dinamik url oluşturma.
void Helper_Build_Get_Url(char* url, size_t size_url, const char* base_url, const char* key, const char* value);

// Bu fonksiyona gelecek olan required time, şimdiki zaman ve gelecek son zaman arasındaki farkı bulup required time çıkarsa true döner.
bool Helper_GetElapsedTime(TickType_t last_time, uint8_t required_time);

// POST cevabından gelen mesajı ilgili değerlere atama.
http_post_result Helper_Post_Response_Code(char* buff);

// Zamanı string'e çevirme.
void Helper_Convert_Time_To_String(char* buffer, uint8_t size, RTC_DS321_Time* time);

// Tarihi string'e çevirme.
void Helper_Convert_Date_To_String(char* buffer, uint8_t size, RTC_DS321_Time* time);

// Bakiyeyi string'e çevirme.
void Helper_Convert_Balance_To_String(char* buffer, uint8_t size, uint32_t balance);

// Ücret tarifesini string'e çevirme.
void Helper_Convert_Fare_To_String(char* buffer, uint8_t size, uint32_t fare);

// Yüklenecek bakiye değerini string'e çevirme.
void Helper_Convert_Loaded_Amount_To_String(char* buffer, uint8_t size, uint32_t amount);

#endif /* HELPER_FUNCTION_H_ */
