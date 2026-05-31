#include "sim800c.h"

char ip_adress[20];
volatile uint8_t waiting_response = 0;

// SIM800C başlatma fonksiyonu.
SIM800C_Status_Type SIM800C_Init()
{
	SIM800C_Status_Type st = SIM800C_Status_Start;

    // PWRKEY başlangıçta HIGH
    GPIOA->BSRR |= (1U << 15);

	Delay_ms(500);

	GPIOA->BSRR |= (1U << 31); // PWRKEY LOW

	Delay_ms(1500); 		   // PWKEY pini 1.5 saniye LOW

	GPIOA->BSRR |= (1U << 15); // PWRKEY tekrar HIGH

	Delay_ms(8000);  		  // Modülün ayağa kalkması için beklemek

	// 500ms'de bir ilk AT komutunu gönder ve 5 saniye boyunca yap.
	for(int i= 0; i < 5 ; i++)
	{
		st = SIM800C_SendCommand(AT, ANS_OK, 200, 100);

		if(st == SIM800C_Status_OK) break;

		Delay_ms(500);

	}

	if(st != SIM800C_Status_OK) return st;

	// Baud Rate sabitle ve gönderilen komut geri dönüşü (yankı) kapat ve kalıcı olarak kaydet
	st = SIM800C_SendCommand(AT_BAUD, ANS_OK, 200, 200);
	if(st != SIM800C_Status_OK) return st;
	st = SIM800C_SendCommand(AT_E0, ANS_OK, 200, 200);
	if(st != SIM800C_Status_OK) return st;
	st = SIM800C_SendCommand(ATW, ANS_OK, 200, 200);
	if(st != SIM800C_Status_OK) return st;


	Delay_ms(3000);

	// PIN kodu aktif mi kontrolü.
	st = SIM800C_SendCommand(AT_CPIN, ANS_READY, 1000, 500);

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
		USART_Flush_RX();

		USART_Transmit((uint8_t*)AT_CREG, strlen(AT_CREG));

		char temp[256] = {0};        // Gelen yanıt için geçici dizi

		USART_Receive((uint8_t*)temp, sizeof(temp), 200);

		char *ptr = strstr((void*)temp, "+CREG: "); // Gelen yanıtta +CREG varmı kontrol et.

		// Varsa
		if(ptr != NULL)
		{
			char *comma = strchr(ptr, ',');   // Virgül olan yere git
			// Virgül varsa
			if(comma != NULL)
			{
				int status = atoi(comma + 1); // Adres değeri bir arttır sayı olan kısma gel
				switch (status)
				{

				// [GSM] Kayıt başarılı (Kendi Şebekesi).\n
				case NET_REGISTERED_HOME: return SIM800C_Status_Network_OK;

				//[GSM] Roaming (Dolaşım) ile bağlandı.\n"
				case NET_REGISTERED_ROAMING: return SIM800C_Status_Network_OK;

				case NET_REG_DENIED: return SIM800C_Status_Network_Not_OK;

				// [GSM] Kayıtlı değil, aranmıyor.\n
				case NET_NOT_REGISTERED:

				// GSM] Şebeke aranıyor...\n"
				case NET_SEARCHING:

				// [GSM] Durum belirsiz.\n
				case NET_UNKNOWN:
					Delay_ms(200);
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

	return network_st;
}

// GPRS hizmetine bağlantı başlatma fonksiyonu.
SIM800C_Status_Type SIM800C_ConnectGPRS(void)
{
	SIM800C_Status_Type st;
	// 1. GPRS Servis Kontrolü
	st = SIM800C_SendCommand(AT_CGATT, ANS_CGATT_1, 200, 200);
	if(st != SIM800C_Status_OK) return st;

	// 2. GPRS Kayıt Kontrolü (Home veya Roaming)
	st = SIM800C_SendCommand(AT_CGREG, ANS_CGREG_1, 200, 200);
	if(st != SIM800C_Status_OK)
	{
		st = SIM800C_SendCommand(AT_CGREG, ANS_CGREG_5, 200, 200);
		if(st != SIM800C_Status_OK) return st;
	}

	// 3. Bearer Ayarları (Bağlantı Tipi ve APN)
	st = SIM800C_SendCommand(AT_SAPBR_CON, ANS_OK, 200, 200);
	if(st != SIM800C_Status_OK) return st;

	st = SIM800C_SendCommand(AT_SAPBR_CONN, ANS_OK, 200, 200);
	if(st != SIM800C_Status_OK) return st;

	// 4. Bearer Açma (İnternet Oturumunu Başlatır)
	// Bearer zaten açıksa error dönebilir bu yüzden buradan çıkmayıp IP sorgusu için devam edilmeli
	st = SIM800C_SendCommand(AT_SAPBR_OPEN_BEARER, ANS_OK, 200, 100);

	// Bütün işlemler geçilip IP adresi alınmış ise bağlantı başarılı.
	st = SIM800C_SendCommand(AT_SAPBR_QUERY_IP, ANS_OK, 200, 100);
	if(st != SIM800C_Status_OK) return st;

	// USART DR register'da önceden kalan verileri temizle.
	USART_Flush_RX();

	// IP adresi sorgusu.
	USART_Status_Typedef usart_st = USART_Transmit((uint8_t*)AT_SAPBR_QUERY_IP, strlen(AT_SAPBR_QUERY_IP));
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Transmit_Fail;

	char temp[256] = {0};       // Veriyi almak için geçici dizi

	// Komuta verilen cevabu geçici diziye al.
	usart_st = USART_Receive((uint8_t*)temp, sizeof(temp), 200);
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Receive_Fail;

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
		return SIM800C_Status_GPRS_Conn_Fail;   // GPRS bağlantısı başarısız
}

// HTTP GET işlemi. (Json response almak.)
SIM800C_Status_Type HTTP_GET_Json(char* url, char* http_data, uint16_t data_max_len)
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
    st = SIM800C_SendCommand(AT_HTTP_SSL_ON, ANS_OK, 200, 200);
    if(st != SIM800C_Status_OK) return st;

    // Redirect takibini aktif et.
    st = SIM800C_SendCommand(AT_HTTP_REDIR_ON, ANS_OK, 200, 200);
    if(st != SIM800C_Status_OK) return st;

	char cmd[256] = {0};  // Geçici dizi oluştur.

	// Dışarıdan gelecek URL'i AT komutun içine kopyala
	snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);

	// ilgili URL' GET isteği at.
	st = SIM800C_SendCommand(cmd, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	// GET eylemi başlatılmış ise,
	st = SIM800C_SendCommand(AT_HTTP_ACTION_GET, ANS_0_200, 5000, 1000);
	if(st != SIM800C_Status_OK)
	{
		st = SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 500, 100);    // HTTP işlemi bitti, kapat.
		if(st != SIM800C_Status_OK) return st;
		return SIM800C_Status_Send_HTTP_Command_Fail;
	}

	// Geçici dizi oluştur
	char ring_buffer[data_max_len];
	memset(ring_buffer, 0, data_max_len);

	USART_Flush_RX();

	USART_Status_Typedef usart_st = USART_Transmit((uint8_t*)AT_HTTP_READ, strlen(AT_HTTP_READ));   // Okuma isteği at
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Transmit_Fail;

	usart_st = USART_Receive((uint8_t*)ring_buffer, sizeof(ring_buffer), 1000);
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Receive_Fail;

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

// HTTP GET işlemi. (Raw(ham) veri almak.)
SIM800C_Status_Type HTTP_GET_Raw_Binary(char* url, uint8_t* http_data, uint16_t expected_size)
{
	if(http_data == NULL || expected_size == 0)
	{
	    return SIM800C_Status_Buffer_Overflow;
	}

	SIM800C_Status_Type st;

	// Eğer HTTP servisi başlatılmış ise
	st = HTTP_Init();
	if(st != SIM800C_Status_OK) return st;


    // HTTPS/SSL kullanımını aktif et.
    st = SIM800C_SendCommand(AT_HTTP_SSL_ON, ANS_OK, 200, 200);
    if(st != SIM800C_Status_OK) return st;

    // Redirect takibini aktif et.
    st = SIM800C_SendCommand(AT_HTTP_REDIR_ON, ANS_OK, 200, 200);
    if(st != SIM800C_Status_OK) return st;

	char cmd[256] = {0};  // Geçici dizi oluştur.

	// Dışarıdan gelecek URL'i AT komutun içine kopyala
	snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);

	// ilgili URL' GET isteği at.
	st = SIM800C_SendCommand(cmd, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	// GET eylemi başlat.
	st = SIM800C_SendCommand(AT_HTTP_ACTION_GET, ANS_0_200, 5000, 1000);
	if(st != SIM800C_Status_OK)
	{
		st = SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 500, 100);    // HTTP işlemi bitti, kapat.
		if(st != SIM800C_Status_OK) return st;
		return SIM800C_Status_Send_HTTP_Command_Fail;
	}

	// GET ile alınacak cevabın uzunluğunu bildir.
	char cmd2[32] = {0};
	sprintf(cmd2, "AT+HTTPREAD=0,%u\r\n", expected_size);

	// USART DR register temizle.
	USART_Flush_RX();

	// GET isteği yap.
	USART_Status_Typedef usart_st = USART_Transmit((uint8_t*)cmd2, strlen(cmd2));
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Transmit_Fail;

	// Geçici dizi oluştur ve başlangıçta sıfırla, boyutu üst fonksiyondan belirlenecek.
	uint8_t extra_size = 100;
	char ring_buffer[expected_size + extra_size];
	memset(ring_buffer, 0, sizeof(ring_buffer));

	// GET isteği ile dönen response raw (ham) veriyi diziye al.
	usart_st = USART_Receive((uint8_t*)ring_buffer, sizeof(ring_buffer), 2000);
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Receive_Fail;

	char* ptr= strstr((void*)ring_buffer, "+HTTPREAD: "); // Gelen yanıtta ilgili kısım varsa değişkene ata

	// Değişkenin değeri varsa, yani istenen yanıtı içermiyorsa okuma hatası dön.
	if(ptr == NULL) return SIM800C_Status_HTTP_Read_Fail;

	// Alınan uzunluk istenen uzunluğa eşitmi kontrol et.
	ptr += strlen("+HTTPREAD: ");
	uint16_t received_size = atoi(ptr);
	if(received_size != expected_size) return SIM800C_Status_Invalid_Size;

	// OK\r\n sonrasına git, çünkü dönen cevap bu kısımdan sonra.
	char* body_start = strstr(ptr, "\r\n");
	if(body_start == NULL) return SIM800C_Status_HTTP_Read_Fail;
	body_start += strlen("\r\n");

	// Gelen cevabı üst fonksiyondan gelen diziye kopyala.
	memcpy(http_data, body_start, expected_size);

	// HTTP işlemi kapat.
	st = SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 500, 100);
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

	//USART_Flush_RX();

	USART_Status_Typedef usart_st = USART_Transmit((uint8_t*)data, data_len);
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Transmit_Fail;

	char data_response[256] = {0};

	usart_st = USART_Receive((uint8_t*)data_response, sizeof(data_response), 4000);
	if(usart_st != USART_Status_OK)
	{
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
		return SIM800C_Status_USART_Receive_Fail;
	}

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

	USART_Flush_RX();

	usart_st = USART_Transmit((uint8_t*)AT_HTTP_READ, strlen(AT_HTTP_READ));
	if(usart_st != USART_Status_OK) return SIM800C_Status_USART_Transmit_Fail;

    usart_st = USART_Receive((uint8_t*)ring_buffer, sizeof(ring_buffer), 2000);

	if(usart_st != USART_Status_OK)
	{
        SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1000, 200);
		return SIM800C_Status_USART_Receive_Fail;
	}

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
SIM800C_Status_Type SIM800C_SendCommand(char *cmd, char* expected_ans, uint16_t timeout, uint16_t retry_delay_ms)
{
	for(int retry = 0; retry < 3; retry++)
	{
		USART_Status_Typedef usart_st;

		USART_Flush_RX();

		usart_st = USART_Transmit((uint8_t*)cmd, strlen(cmd));
		if(usart_st != USART_Status_OK) continue;

		char temp_arr[256] = {0};

		usart_st = USART_Receive((uint8_t*)temp_arr, sizeof(temp_arr), timeout);
		if(usart_st != USART_Status_OK) continue;

		if(strstr(temp_arr, expected_ans) != NULL) return SIM800C_Status_OK;

        if(strstr(temp_arr, "ERROR") != NULL)
        {
        	Delay_ms(retry_delay_ms);
            continue;
        }


		Delay_ms(retry_delay_ms);
	}

	return SIM800C_Status_No_Response;
}

/*
// HTTP komutları gönderme yardımcı fonksiyonu.
SIM800C_Status_Type SIM800C_Send_HTTP_Command(char *cmd, uint16_t timeout)
{
    USART_Transmit((uint8_t*)cmd, strlen(cmd));

    uint32_t start = Get_Tick();

    while(1)
    {
    	uint32_t now = Get_Tick();

        if((now - start) >= timeout)
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

                if(method == 0 && code == 200 && len > 0)
                {
                    return SIM800C_Status_OK;
                }

                return SIM800C_Status_HTTP_Read_Fail;
            }
        }

        Delay_ms(1);
    }

    return SIM800C_Status_No_Response;
}*/

// HTTP servis hizzmeti başlatma fonksiyonu.
SIM800C_Status_Type HTTP_Init()
{
	SIM800C_Status_Type st;

	(void)SIM800C_SendCommand(AT_HTTP_TERM, ANS_OK, 1500, 200);
	Delay_ms(300);

	st = SIM800C_SendCommand(AT_HTTP_INIT, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	st = SIM800C_SendCommand(AT_HTTP_PARA_CID, ANS_OK, 500, 200);
	if(st != SIM800C_Status_OK) return st;

	return SIM800C_Status_OK;
}

