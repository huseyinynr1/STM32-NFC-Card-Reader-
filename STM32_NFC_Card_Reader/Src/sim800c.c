#include "sim800c.h"

char ip_adress[20];
volatile uint8_t waiting_response = 0;

SIM800C_Status_Type SIM800C_Init()
{
	SIM800C_Status_Type st = SIM800C_Status_Start;

    // PWRKEY başlangıçta HIGH
    GPIOA->BSRR |= (1U << 15);

	Delay_Ms(500);

	GPIOA->BSRR |= (1U << 31); // PWRKEY LOW

	Delay_Ms(1500); 		   // PWKEY pini 1.5 saniye LOW

	GPIOA->BSRR |= (1U << 15); // PWRKEY tekrar HIGH

	Delay_Ms(8000);  		  // Modülün ayağa kalkması için bekleme

	// 500ms'de bir ilk AT komutunu gönder ve 5 saniye boyunca yap.
	for(int i= 0; i < 5 ; i++)
	{
		st = SIM800C_Send_Init_Command(AT, ANS_OK, 500, 100);

		if(st == SIM800C_Status_OK) break;

	    Delay_Ms(500);

	}

	if(st != SIM800C_Status_OK) return st;

	// Baud Rate sabitle ve gönderilen komut geri dönüşü (yankı) kapat ve kalıcı olarak kaydet
	st = SIM800C_Send_Init_Command(AT_BAUD, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;
	st = SIM800C_Send_Init_Command(AT_E0, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;
	st = SIM800C_Send_Init_Command(ATW, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;


	Delay_Ms(3000);
    USART3_Reset_Buffer();

	// PIN kodu aktif mi kontrolü.
	st = SIM800C_Send_Init_Command(AT_CPIN, ANS_READY, 5000, 3000);

	if(st != SIM800C_Status_OK) return st;

	// Şebeke durumu kontrolü
	st = SIM800C_CheckNetworkRegistration();

	if(st != SIM800C_Status_Network_OK) return st;

	return SIM800C_Status_OK;
}

// SIM800C şebeke durumu sorgulama
SIM800C_Status_Type SIM800C_CheckNetworkRegistration(void)
{
	SIM800C_Status_Type network_st = SIM800C_Status_No_Response;

	for(int i = 0; i < 20; i++)
	{
		USART3_Reset_Buffer();

		waiting_response = 1;
		package_ready = 0;

		USART3_SendWithDMA(AT_CREG, strlen(AT_CREG));

		USART3_Wait_For_Init_Packet(5000, 200);

		waiting_response = 0;

		if(package_ready == 1)
		{
			char temp[256] = {0};        // Gelen yanıt için geçici dizi
			USART3_Receive(temp, sizeof(temp));  // Gelen yanıyı geçici diziye al.
			char *ptr = strstr((void*)temp, "+CREG: "); // Gelen yanıtta +CREG varmı kontrol et.

			// Varsa
			if(ptr != NULL)
			{
				char *comma = strchr(ptr, ',');   // Virgül olan yere git
				// Virgül varsa
				if(comma != NULL)
				{
					int status = atoi(comma + 1); // Adres değeri bir arttır sayı olan kısma gel
					switch (status) {

					// [GSM] Kayıt başarılı (Kendi Şebekesi).\n
						case NET_REGISTERED_HOME:
							return SIM800C_Status_Network_OK;

					//[GSM] Roaming (Dolaşım) ile bağlandı.\n"
						case NET_REGISTERED_ROAMING:
							return SIM800C_Status_Network_OK;

						case NET_REG_DENIED:
							return SIM800C_Status_Network_Not_OK;

					// [GSM] Kayıtlı değil, aranmıyor.\n
						case NET_NOT_REGISTERED:

					// GSM] Şebeke aranıyor...\n"
						case NET_SEARCHING:

					// [GSM] Durum belirsiz.\n
						case NET_UNKNOWN:
							Delay_Ms(200);
							network_st = SIM800C_Status_Network_Not_OK;
							break;

						default:
							network_st = SIM800C_Status_Network_Not_OK;
							break;
					}
				}

				else
				{
					network_st = SIM800C_Status_Network_Not_OK;
				}
			}

			else
			{
				network_st = SIM800C_Status_Network_Not_OK;
			}
		}

		else
		{
			network_st = SIM800C_Status_No_Response;
		}
	}

	return network_st;
}

// GPRS hizmetine bağlantı başlatma fonksiyonu.
SIM800C_Status_Type SIM800C_ConnectGPRS(void)
{
	SIM800C_Status_Type st;
	// 1. GPRS Servis Kontrolü
	st = SIM800C_Send_Init_Command(AT_CGATT, ANS_CGATT_1, 4000, 200);
	if(st != SIM800C_Status_OK) return st;

	// 2. GPRS Kayıt Kontrolü (Home veya Roaming)
	st = SIM800C_Send_Init_Command(AT_CGREG, ANS_CGREG_1, 2000, 200);
	if(st != SIM800C_Status_OK)
	{
		st = SIM800C_Send_Init_Command(AT_CGREG, ANS_CGREG_5, 2000, 200);
		if(st != SIM800C_Status_OK) return st;
	}

	// 3. Bearer Ayarları (Bağlantı Tipi ve APN)
	st = SIM800C_Send_Init_Command(AT_SAPBR_CON, ANS_OK, 2000, 200);
	if(st != SIM800C_Status_OK) return st;

	st = SIM800C_Send_Init_Command(AT_SAPBR_CONN, ANS_OK, 2000, 200);
	if(st != SIM800C_Status_OK) return st;

	// 4. Bearer Açma (İnternet Oturumunu Başlatır)
	// Bearer zaten açıksa error dönebilir bu yüzden buradan çıkmayıp IP sorgusu için devam edilmeli
	st = SIM800C_Send_Init_Command(AT_SAPBR_OPEN_BEARER, ANS_OK, 5000, 500);
	//if(st != SIM800C_Status_OK) return st;

	// Bütün işlemler geçilip IP adresi alınmış ise bağlantı başarılı.
	st = SIM800C_Send_Init_Command(AT_SAPBR_QUERY_IP, ANS_OK, 5000, 500);
	if(st != SIM800C_Status_OK) return st;


	// IP adresi almak için yardımcı fonksiyon yerine manuel alma adımları. Son tamponu sıfırla ve IP adresi sorgulama komutu gönder.
	USART3_Reset_Buffer();

	waiting_response = 1;
	package_ready = 0;

	USART3_SendWithDMA(AT_SAPBR_QUERY_IP, strlen(AT_SAPBR_QUERY_IP));

	USART3_Wait_For_Init_Packet(5000, 200);

	waiting_response = 0;

	// Veri geldi ve paket hazırsa
	if(package_ready == 1)
	{
		char temp[256] = {0};       // Veriyi almak için geçici dizi
		USART3_Receive(temp, sizeof(temp)); // Veriyi al
		char* ptr = strstr((void*)temp, "+SAPBR: 1,1");  // Cevaptaki istenen konuma git
		// Cevapta istenen komut varsa
		if(ptr != NULL)
		{
			// IP adres başlangıç ve bitiş yerlerini al
			char* ip_start = strchr(ptr, '\"');
			if(ip_start)
			{
				char* ip_end = strchr(ip_start+1 , '\"');
				if(ip_end)
				{
					int len = ip_end  - (ip_start + 1); // IP adres uzunluğu hesapla
					if(len > 0 && len < (int)sizeof(ip_adress))
					{
						strncpy(ip_adress, ip_start + 1, len); // IP adresi geçici diziye kopyala
						ip_adress[len] = '\0';             // String sonu ekle, başka yerlerde kullanmak için
						return SIM800C_Status_OK;                  // Tüm işlemler tamamlandı bağlantı başarılı.
					}
				}
			}
		}
	}
		return SIM800C_Status_GPRS_Conn_Fail;   // GPRS bağlantısı başarısız
}

// HTTP GET işlemi başlatma fonksiyonu
SIM800C_Status_Type HTTP_GET(char* url, char* http_data, uint16_t data_max_len)
{
	// Eski response temizleme.
	if(http_data != NULL && data_max_len > 0)
	{
	    http_data[0] = '\0';
	}

	SIM800C_Status_Type st;

	// Eğer HTTP servisi başlatılmış ise
	st = HTTP_Init();
	if(st != SIM800C_Status_OK) return st;


    // HTTPS/SSL kullanımını aktif et.
    st = SIM800C_SendCommand(AT_HTTP_SSL_ON, ANS_OK, 1000, 200);
    if(st != SIM800C_Status_OK) return st;

    // Redirect takibini aktif et.
    st = SIM800C_SendCommand(AT_HTTP_REDIR_ON, ANS_OK, 1000, 200);
    if(st != SIM800C_Status_OK) return st;

	char cmd[256] = {0};  // Geçici dizi oluştur.

	// Dışarıdan gelecek URL'i AT komutun içine kopyala
	snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);
	 // ilgili URL' GET isteği at.
	st = SIM800C_SendCommand(cmd, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	// GET eylemi başlatılmış ise,
	st = SIM800C_SendCommand(AT_HTTP_ACTION_GET, ANS_0_200, 5000, 3000);
	if(st != SIM800C_Status_OK)
	{
		st = SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 500, 100);    // HTTP işlemi bitti, kapat.
		if(st != SIM800C_Status_OK) return st;
		return SIM800C_Status_Send_HTTP_Command_Fail;
	}

	char ring_buffer[512] = {0};  // Geçici dizi oluştur

	USART3_Reset_Buffer();   // Önceki tamponu temizle

	waiting_response = 1;
	package_ready = 0;

	USART3_SendWithDMA(AT_HTTP_READ, strlen(AT_HTTP_READ));   // Okuma isteği at

	USART3_Wait_For_Packet(5000, 1000);

	waiting_response = 0;

	// Gelen veri paketi hazırsa
	if(package_ready != 1) return SIM800C_Status_No_Response;

	USART3_Receive(ring_buffer, sizeof(ring_buffer));     // Geçici diziye al
	char* ptr= strstr((void*)ring_buffer, "+HTTPREAD: "); // Gelen yanıtta ilgili kısım varsa değişkene ata

	// Değişkenin değeri varsa, yani istenen yanıtı içeriyorsa
	if(ptr == NULL) return SIM800C_Status_HTTP_Read_Fail;

	char* start = strchr(ptr,'{');   // Veri JSON formatında geleceği için başlangıç süslü paranteze git

	// start ve end null değilse yani aranan kısımlar bulunmuş ise bu şarta gir.
	if(start == NULL) return SIM800C_Status_HTTP_Parse_Fail;

	char* end = strrchr(start, '}');  // Bitiş süslü paranteze git.

	if(end == NULL || end <= start)  return SIM800C_Status_HTTP_Parse_Fail;

	if(http_data == NULL || data_max_len == 0) return SIM800C_Status_Buffer_Overflow;

	int len = (end - start) + 1;   // Başlangıç ve bitiş prantezi dahil uzunluğu al

	// Gelen yanıt uzunluğu dışarıdn gelecek dizinin uzunluğundan küçükse ( Büyükse taşma olur.)
	if(len >= data_max_len) return SIM800C_Status_Buffer_Overflow;

	memcpy(http_data, start, len);   // Ayarlanan kısımdaki karakterleri dışarıdan gelen diziye kopyala.
	http_data[len] = '\0';            // String sonu

	st = SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 500, 100); // Servisi kapat
	if(st != SIM800C_Status_OK) return st;

	return SIM800C_Status_OK;
}

// HTTP POST işlemi başlatma fonksiyonu.
SIM800C_Status_Type HTTP_POST(char* url, char* data, char* http_response_data, uint16_t data_max_len, uint16_t timeout)
{
	uint16_t data_len = strlen(data);  // Gelen dizinin uzunluğunu al
	SIM800C_Status_Type st;

	// HTTP Eylemi başlat, başlatılamadıysa fonksiyondan çık
	st = HTTP_Init();
	if(st != SIM800C_Status_OK) return st;

	// HTTPS/SSL kullanımını aktif et.
	st = SIM800C_SendCommand(AT_HTTP_SSL_ON, ANS_OK, 1000, 200);
	if(st != SIM800C_Status_OK)
	{
	    SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
	    return st;
	}

    // Redirect takibini aktif et.
    st = SIM800C_SendCommand(AT_HTTP_REDIR_ON, ANS_OK, 1000, 200);
    if(st != SIM800C_Status_OK)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return st;
    }

	// URL bildirimi için geçici dizi oluştur.
	char cmd[256] = {0};

	// URL alınmış AT komutunu string dizisine kaydet.
	snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);

	// URL ayarlaması için AT komutunu gönder
    st = SIM800C_SendCommand(cmd, ANS_OK, 1000, 200);
    if(st != SIM800C_Status_OK)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return st;
    }

	// Gönderilecek verinin JSON tipinde olduğunu söyle
    // Content-Type: application/json
    st = SIM800C_SendCommand(AT_HTTP_JSON, ANS_OK, 1000, 200);
    if(st != SIM800C_Status_OK)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return st;
    }

	// Gönderliecek veri boyutunu ve timeout süresini string diziye kaydet
	snprintf(cmd, sizeof(cmd), "AT+HTTPDATA=%u,%u\r\n", data_len, timeout);

	// Hazırlanmış AT dizisini gönder ve DOWNLOAD cevabını beklle.
    st = SIM800C_SendCommand(cmd, ANS_DOWNLOAD, 6000, 200);
    if(st != SIM800C_Status_OK)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return st;
    }

    /*
     * DOWNLOAD geldikten sonra JSON body gönderilir.
     * Bu aşamada SIM800C body'yi alır ve genellikle OK döner.
     * Bu yüzden burada da waiting_response açılmalı ve paket beklenmelidir.
     */

	USART3_Reset_Buffer();

    waiting_response = 1;
    package_ready = 0;

	USART3_SendWithDMA(data, data_len);

	USART3_Wait_For_Packet(5000, 1000);

    waiting_response = 0;

    if(package_ready != 1)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_HTTP_Data_Send_Fail;
    }


    char data_response[256] = {0};
    USART3_Receive(data_response, sizeof(data_response));
    if(strstr(data_response, ANS_OK) == NULL)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_HTTP_Data_Send_Fail;
    }

	// HTTP POST işlemini başlat.
    st = SIM800C_SendCommand(AT_HTTP_ACTION_POST, ANS_1_200, 5000, 2000);
    if(st != SIM800C_Status_OK)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_Send_HTTP_Command_Fail;
    }

	char ring_buffer[512] = {0};

	USART3_Reset_Buffer();

	waiting_response = 1;
	package_ready = 0;

	USART3_SendWithDMA(AT_HTTP_READ, strlen(AT_HTTP_READ));

    USART3_Wait_For_Packet(5000, 2000);


    waiting_response = 0;

    if(package_ready != 1)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_No_Response;
    }

	USART3_Receive(ring_buffer, sizeof(ring_buffer));

	char* ptr = strstr(ring_buffer, "+HTTPREAD:");
    if(ptr == NULL)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_HTTP_Read_Fail;
    }


	// Dönen Json yanıtı (iki süslü parantez arası) ayrıştır ve al.
    char* start = strchr(ptr, '{');
    if(start == NULL)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_HTTP_Parse_Fail;
    }

    char* end = strrchr(start, '}');
    if(end == NULL || end <= start)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_HTTP_Parse_Fail;
    }

    if(http_response_data == NULL || data_max_len == 0)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_Buffer_Overflow;
    }

    int len = (end - start) + 1;

    if(len >= data_max_len)
    {
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
        return SIM800C_Status_Buffer_Overflow;
    }

	memcpy(http_response_data, start, len);
	http_response_data[len] = '\0';

	st = SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 500, 100);    // HTTP işlemi bitti, kapat.
	if(st != SIM800C_Status_OK) return st;

	return SIM800C_Status_OK;
}

// Başlangıç komutları gönderme yardımcı fonksiyon.
SIM800C_Status_Type SIM800C_Send_Init_Command(char *cmd, char* expected_ans, uint16_t timeout, uint16_t retry_delay_ms)
{
	for(int retry = 0; retry < 3; retry++)
	{
		USART3_Reset_Buffer();

		waiting_response = 1;
		package_ready = 0;

		USART3_SendWithDMA(cmd, strlen(cmd));

		USART3_Wait_For_Init_Packet(timeout, 200);

		waiting_response = 0;

		if(package_ready == 1)
		{
			char temp_arr[256] = {0};
			USART3_Receive(temp_arr, sizeof(temp_arr));

			if(strstr(temp_arr, expected_ans) != NULL) return SIM800C_Status_OK;

            if(strstr(temp_arr, "ERROR") != NULL)
            {
        		Delay_Ms(retry_delay_ms);
                continue;
            }
		}

		Delay_Ms(retry_delay_ms);
	}

	return SIM800C_Status_No_Response;
}

// Komut gönderme yardımcı fonksiyon.
SIM800C_Status_Type SIM800C_SendCommand(char *cmd, char* expected_ans, uint16_t timeout, uint16_t silence_ms)
{
	for(int retry = 0; retry < 3; retry++)
	{
		USART3_Reset_Buffer();

		waiting_response = 1;
		package_ready = 0;

		USART3_SendWithDMA(cmd, strlen(cmd));

		USART3_Wait_For_Packet(timeout, silence_ms);

		if(package_ready == 1)
		{
			char temp_arr[256] = {0};
			USART3_Receive(temp_arr, sizeof(temp_arr));

			waiting_response = 0;

			if(strstr(temp_arr, expected_ans) != NULL) return SIM800C_Status_OK;

            if(strstr(temp_arr, "ERROR") != NULL)
            {
                vTaskDelay(pdMS_TO_TICKS(1));
                continue;
            }
		}

		waiting_response = 0;
		vTaskDelay(pdMS_TO_TICKS(1));
	}

	waiting_response = 0;
	return SIM800C_Status_No_Response;
}

// HTTP komutları gönderme yardımcı fonksiyonu.
SIM800C_Status_Type SIM800C_Send_HTTP_Command(char *cmd, uint16_t timeout)
{
    USART3_Reset_Buffer();

    waiting_response = 1;
    package_ready = 0;

    USART3_SendWithDMA(cmd, strlen(cmd));

    TickType_t start = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout);

    while(1)
    {
        TickType_t now = xTaskGetTickCount();

        if((now - start) >= timeout_ticks)
        {
            break;
        }

        char *ptr = strstr((char*)rx_buffer, "+HTTPACTION:");

        if(ptr != NULL)
        {
            int method = 0;
            int code = 0;
            int len = 0;

            if(sscanf(ptr, "+HTTPACTION: %d,%d,%d", &method, &code, &len) == 3)
            {
                waiting_response = 0;

                if(method == 0 && code == 200 && len > 0)
                {
                    return SIM800C_Status_OK;
                }

                return SIM800C_Status_HTTP_Read_Fail;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    waiting_response = 0;
    return SIM800C_Status_No_Response;
}

// HTTP servis hizzmeti başlatma fonksiyonu.
SIM800C_Status_Type HTTP_Init()
{
	SIM800C_Status_Type st;

	(void)SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1500, 200);
	vTaskDelay(pdMS_TO_TICKS(300));

	st = SIM800C_SendCommand(AT_HTTP_INIT, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	st = SIM800C_SendCommand(AT_HTTP_PARA_CID, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	return SIM800C_Status_OK;
}

