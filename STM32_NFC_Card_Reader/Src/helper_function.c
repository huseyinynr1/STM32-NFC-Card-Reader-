#include "helper_function.h"

// Yeni kart bilgilerini kartın bloklarına yazma işlemi sonuç durumu db'e göndermek için json dönüşümü.
void Helper_Conver_to_Json_Request_Result(http_request_helper* result, char* arr, size_t size)
{
	char type_arr[11] = {0};
	memcpy(type_arr, Helper_Get_HTTP_Status_Value_to_String(result->status_type), sizeof(type_arr));
	snprintf(arr, size,  "{\"RequestId\":%lu,\"Status\":\"%s\"}", result->request_id, type_arr);
}

// API'den gelen işlemlerin ilerleyiş durumunu int'den string'e çevirme.
const char* Helper_Get_HTTP_Status_Value_to_String(uint8_t type)
{
	switch (type) {
		case 0:  return "Pending";
		case 1:  return "Completed";
		case 2:  return "Failed";
		default: return "Failed";
	}
}

// Get ile gelen response'u değerlere ayrıştırma.
void Helper_Json_Convert_to_New_Card_Info(char* arr, new_card_response_helper* new_card)
{
	char* p;
	char type        [20] = {0};
	char expiry_date [11] = {0};
	char visa_date   [11] = {0};


	if(arr == NULL || new_card == NULL) return; // Dizi veya struct boş ise döngüden çık.

	memset(new_card, 0, sizeof(*new_card)); // struct'ı temizle.

	p = strstr(arr, "\"MagicNumber\":\"");

	if(p != NULL)
	{
		p = p + strlen("\"MagicNumber\":\"");
		char* end = strchr(p, '"');
		if(end != NULL)
		{
			size_t len = end - p;
			if(len == sizeof(new_card->magic_number))
			{
				memcpy(new_card->magic_number, p, len);
			}

		}
	}

	// p ile response'da arananın başlangıç adresini al , :'a kadar p'yi arttır ve değeri al
	p = strstr(arr, "\"Version\":");
	if(p != NULL)
	{
		p = p + strlen("\"Version\":");
		new_card->version = atoi(p);
	}

	// Dizide ilgili kısım varmı kontrol et.
	p = strstr(arr, "\"RequestId\":");
	// Varsa ilgili kısımdaki değeri struct yapısındaki değişkene ata.
	if(p != NULL)
	{
		p = p + strlen("\"RequestId\":");
		new_card->request_id = atoi(p);
	}

	// kart tipi'ni alma.
	p = strstr(arr,"\"CardType\":\"");
	if(p != NULL)
	{
		p = p + strlen("\"CardType\":\""); // Aranan cevap kısmının başlangıç adresine git.
		char* end = strchr(p, '"');        // İkinci çift tırnak adres değeri al.
		if(end != NULL)
		{
			size_t len = end - p;         // İki çift tırnak arasındakini al
			if(len < sizeof(type))
			{
				memcpy(type, p, len);     // geçici diziye kopyala
				type[len] = '\0';
				new_card->card_type = Helper_Get_Card_Type_Value(type);  // kart tipinin karşılık gelen int değerini al.
			}
		}
	}

	// Son kullanma tarihi değeri alma. p ile başlangıç çift tırnağına end ile ikinci çift tırnağa gidip aradaki değeri al.
	p = strstr(arr, "\"ExpiryDate\":\"");
	if(p != NULL)
	{
		p = p + strlen("\"ExpiryDate\":\"");
		char* end = strchr(p, '"');

		if(end != NULL)
		{
			size_t len = end - p;
			if(len < sizeof(expiry_date))
			{
				memcpy(expiry_date, p, len);
				expiry_date[len] = '\0';
				new_card->expiry_date = Helper_Get_Date_Value(expiry_date); // Kart bloğunda kullanılacak formata çevir.
			}
		}
	}

	// Vize tarihi değeri alma. p ile başlangıç çift tırnağına end ile ikinci çift tırnağa gidip aradaki değeri al.
	p = strstr(arr, "\"VisaDate\":\"");
	if(p != NULL)
	{
		p = p + strlen("\"VisaDate\":\"");
		char* end = strchr(p, '"');

		if(end != NULL)
		{
			size_t len = end - p;
			if(len < sizeof(visa_date))
			{
				memcpy(visa_date, p, len);
				visa_date[len] = '\0';
				new_card->visa_date = Helper_Get_Date_Value(visa_date); // Kart bloğunda kullanılacak formata çevir.
			}
		}
	}

	// Bakiye değerini al.
	p = strstr(arr, "\"CurrentBalanceKurus\":");
	if(p != NULL)
	{
		p = p + strlen("\"CurrentBalanceKurus\":");
		new_card->balance = atoi(p);
	}

	// Maksimum Bakiye değerini al.
	p = strstr(arr, "\"MaxAllowedBalance\":");
	if(p != NULL)
	{
		p = p + strlen("\"MaxAllowedBalance\":");
		new_card->max_allowed_balance = atoi(p);
	}
}

// GET işlemi ile gelen yeni bakiye bilgileri response'u değerlere ayrıştırma.
void Helper_Json_Convert_to_New_Balance(char* arr, topup_response_helper* response)
{
	if(arr == NULL || response == NULL) return;

	memset(response, 0, sizeof(*response));

	char* p;
	char status [10] = {0};

	response->http_process_status = true;

	// Dizide ilgili kısım varmı kontrol et.
	p = strstr(arr, "\"Success\":");

	// Varsa ilgili kısımdaki değeri struct yapısındaki değişkene ata.
	if(p != NULL)
	{
		p = p + strlen("\"Success\":");  // İlgili kısımda : sonrasına gel

		// Değer hangi iki şarttan biriyse onu ata.
		if(strncmp(p, "true", 4) == 0)
		{
			response->has_topup = true;
		}

		else if(strncmp(p, "false", 5) == 0)
		{
			response->has_topup = false;
		}
	}

	if(!response->has_topup) return;

	// Dizide ilgili kısım varmı kontrol et.
	p = strstr(arr, "\"AmountKurus\":");

	// Varsa ilgili kısımdaki değeri struct yapısındaki değişkene ata.
	if(p != NULL)
	{
		p = p + strlen("\"AmountKurus\":");
		response->amount = atoi(p);
	}

	// Dizide ilgili kısım varmı kontrol et.
	p = strstr(arr, "\"RequestId\":");

	// Varsa ilgili kısımdaki değeri struct yapısındaki değişkene ata.
	if(p != NULL)
	{
		p = p + strlen("\"RequestId\":");
		response->request_id = atoi(p);
	}

	p = strstr(arr, "\"Status\":");
	if(p != NULL)
	{
		p = p + strlen("\"Status\":\"");
		char* end = strchr(p, '"');
		if(end != NULL)
		{
			size_t len = end - p;
			if(len < sizeof(status))
			{
				memcpy(status, p, len);
				status[len] = '\0';
				response->status = Helper_Get_Progress_Status_Value(status);
			}
		}
	}
}

// int değere karşılık gelen kart tipi string'i döndür.
const char* Helper_Get_Card_Type_Name(uint8_t type)
{
	switch (type) {
		case 0x00: return "Full Fare Card";
		case 0x01: return "Student Card";
		case 0x02: return "Teacher Card";
		case 0x03: return "Senior Citizen Card";
		case 0x04: return "Disabled Card";
		default:   return "Invalid Card";
	}
}

// Fonksiyona gelen dizideki string'e göre int sonucu dön.
uint8_t Helper_Get_Card_Type_Value(const char* arr)
{
    if(strcmp(arr, "Full Fare Card") == 0)       return 0x00;
    if(strcmp(arr, "Student Card") == 0)      	 return 0x01;
    if(strcmp(arr, "Teacher Card") == 0)     	 return 0x02;
    if(strcmp(arr, "Senior Citizen Card") == 0)  return 0x03;
    if(strcmp(arr, "Disabled Card") == 0)     	 return 0x04;

    return 0x05;
}

// Fonksiyona gelen dizideki string'e göre int sonucu dön.
http_status_value_type Helper_Get_Progress_Status_Value(const char* arr)
{
    if(strcmp(arr, "Pending") == 0)          return Pending;
    if(strcmp(arr, "Completed") == 0)        return Completed;
    if(strcmp(arr, "Failed") == 0)           return Failed;

    return Failed;
}
//Fonksiyona gelecek dizideki tarih değerini ayrıştır,
uint16_t Helper_Get_Date_Value(char* arr)
{
	// "05/05/2030"
	uint8_t day = 0;
	uint8_t month = 0;
	uint16_t full_year = 0;
	uint8_t year = 0;
	char* p;
	p = arr;

	day = atoi(p);

	p = p + 3;

	month = atoi(p);

	p = p + 3;

	full_year = (uint16_t)atoi(p);

	year = (uint8_t)(full_year - 2000);

	// Kart bloğuna yazılacak şekilde paketle.
	return Helper_Date_Packing(year, month, day);

}



// Hexadecimal tipinde kart id'yi string'e çevirme.
void Helper_Get_Card_ID_String(char* arr, size_t size, uint8_t* id, uint8_t id_len)
{
	// dizinin ilk elemanına string sonu atama
	arr[0] = '\0';

	// Gelen id uzunluğu kadar dolaş ve string'e çevir
	for(int i = 0 ; i < id_len ;  i++)
	{
		// ID kopyalamak için geçici dizi.
		char temp[3];

		// Gelen id'deki sayı değerlerini hexadecimal olarak string'e çevir.
		snprintf(temp, sizeof(temp), "%02X", id[i]);

		// Hexadecimal id sayı değerlerini dışarıdan gelen diziye kopyala.
		strncat(arr, temp, size - strlen(arr) - 1);

	}
}

// Kart işlem tarihi string diziye çevirme
void Helper_Get_Card_Date_To_String(uint16_t date, char* buffer, uint8_t size)
{
	snprintf(buffer, size, "%02d/%02d/%d", (date & 0x1F), ((date >> 5) & 0x0F), 2000 + ((date >> 9) & 0x7F));
}

// Kart işlem zamanı string diziye çevirme.
void Helper_Get_Card_Time_To_String(uint32_t time, char* buffer, uint8_t size)
{
	snprintf(buffer, size, "%02ld:%02ld:%02ld", ((time >> 12) & 0x1F), ((time >> 6) & 0x3F), (time & 0x3F));
}

// Tarih değerini binary değere çevirme.
uint16_t Helper_Date_Packing(uint8_t year, uint8_t month, uint8_t day)
{
	// Gün 1-31 arasındadır. 5 bit yer kaplar.
	// Ay 1-12 arasındadir.  4 bit yer kaplar.
	// Yıl 1-99 arasındadır. 7 bit yer kaplar.  (16byte sığdırmak için yıl sayısının son 2 rakamı alınır.)
	// Toplam 16bit ve 2 byte yapar.
	return (uint16_t)((year << 9) | (month << 5) | day);
}

// Zaman değerini binary değere çevirme.
uint32_t Helper_Time_Packing(uint8_t hour, uint8_t minute, uint8_t second)
{
	// Saniye 1-59 arasındadır. 6 bit yer kaplar.
	// Dakika 1-59 arasındadir. 6 bit yer kaplar.
	// Saat 1-23 arasındadır. 5 bit yer kaplar.
	// Toplam 17 bit ve 3 byte yapar.
	return (uint32_t)((hour << 12) | (minute << 6) | second);
}

// Kart bloğundaki verilerin crc değerinin hesaplanması.
uint16_t Helper_Calculate_Crc16(uint8_t* data, uint8_t len)
{
	uint16_t crc = 0xFFFF;

	for(int i = 0; i < len; i++)
	{
		crc = crc ^ (uint16_t)(data[i] << 8);
		for(int j = 0; j < 8; j++)
		{
			if(crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}

			else
			{
				crc = crc << 1;
			}
		}
	}

	return crc;
}

// Buffer ile gelen değerlerin 0x00'mı kontrolü.
bool Helper_IsAllZero(uint8_t* buffer, uint8_t buffer_len)
{
	for(int i = 0; i < buffer_len ; i++)
	{
		if(buffer[i] != 0x00) return false;
	}

	return true;
}

// Buffer ile gelen değerlerin 0xFF'mi kontrolü.
bool Helper_IsAllFF(uint8_t* buffer, uint8_t buffer_len)
{
	for(int i = 0; i < buffer_len ; i++)
	{
		if(buffer[i] != 0xFF) return false;
	}

	return true;
}

// GET isteği yapmadan önce, GET isteğinin neye göre yapılacağına dair dinamik url oluşturma.
void Helper_Build_Get_Url(char* url, size_t size_url, const char* base_url, const char* key, const char* value)
{
	snprintf(url, size_url, "%s?%s=%s", base_url, key, value);
}

// Bu fonksiyona gelecek olan required time, şimdiki zaman ve gelecek son zaman arasındaki farkı bulup required time çıkarsa true döner.
bool Helper_GetElapsedTime(TickType_t last_time, uint8_t required_time)
{
	static TickType_t now;
	static TickType_t elapsed_time;

	now = xTaskGetTickCount();

	elapsed_time = now - last_time;

	if(elapsed_time >= pdMS_TO_TICKS(required_time*60*1000))
	{
		return true;
	}

	return false;
}

// POST cevabından gelen mesajı ilgili değerlere atama.
http_post_result Helper_Post_Response_Code(char* buff)
{
	char* ptr_true = strstr((void *)buff, "true");

	if(ptr_true != NULL) return Post_Result_Successfull;

	char* ptr_false = strstr((void*)buff, "false");

	if(ptr_false != NULL)
	{
		char* ptr_code1 = strstr((void*)buff, "INVALID_JSON");
		char* ptr_code2 = strstr((void*)buff, "DB_ERROR");
		char* ptr_code3 = strstr((void*)buff, "SERVER_ERROR");

		if(ptr_code1 != NULL) return Post_Result_Invalid_Json;
		if(ptr_code2 != NULL) return Post_Result_DB_Error;
		if(ptr_code3 != NULL) return Post_Result_Server_Error;
	}

	return Post_Result_Unknown_Error;
}

// Zamanı string'e çevirme.
void Helper_Convert_Time_To_String(char* buffer, uint8_t size, RTC_DS321_Time* time)
{
	snprintf(buffer, size, "%02d:%02d:%02d", time->hour, time->minute, time->second);
}

// Tarihi string'e çevirme.
void Helper_Convert_Date_To_String(char* buffer, uint8_t size, RTC_DS321_Time* time)
{
	snprintf(buffer, size, "%02d/%02d/%d", time->day_number, time->month, 2000 + time->year);
}

// Bakiyeyi string'e çevirme.
void Helper_Convert_Balance_To_String(char* buffer, uint8_t size, uint32_t balance)
{
	uint16_t balance_kurus = balance;
	uint16_t balance_lira = balance_kurus / 100;

	snprintf(buffer, size, "Yeni Bakiye: %d TL", balance_lira);
}

// Ücret tarifesini string'e çevirme.
void Helper_Convert_Fare_To_String(char* buffer, uint8_t size, uint32_t fare)
{
	uint16_t fare_kurus = fare;
	uint16_t fare_lira = fare_kurus / 100;

	snprintf(buffer, size, "Tutar: %d TL", fare_lira);
}

// Yüklenecek bakiye değerini string'e çevirme.
void Helper_Convert_Loaded_Amount_To_String(char* buffer, uint8_t size, uint32_t amount)
{
	uint16_t amount_kurus = amount;
	uint16_t amount_lira = amount_kurus / 100;

	snprintf(buffer, size, "Yuklenen Bakiye: %d TL", amount_lira);
}
