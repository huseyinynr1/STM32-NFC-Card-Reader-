#ifndef SIM800C_H_
#define SIM800C_H_

#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "usart_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app_tasks.h"

// AT komutları
#define AT 		 "AT\r\n"             // İlk başlatma komutu.
#define AT_BAUD  "AT+IPR=115200\r\n"  // SIM800C baud rate sabitleme komutu.
#define AT_E0 	 "ATE0\r\n"           // Yankı(Gönderilen komutların geri dönmesi) kapatma komutu.
#define ATW 	 "AT&W\r\n"           // Ayarları kalıcı olarak kaydetme komutu.
#define AT_CPIN  "AT+CPIN?\r\n"       // SIM800C sim ve pin sorgulama komutu.
#define AT_CREG  "AT+CREG?\r\n"       // Şebeke durumu kontrolü komutu.
#define AT_CGATT "AT+CGATT?\r\n"      // GPRS hizmetine bağlanılıp ayrılma durum kontrolü
#define AT_CGREG "AT+CGREG?\r\n"      // GPRS yetkisi sorgulama komutu.
#define AT_SAPBR "AT+SAPBR=\r\n"      // IP tabanlı uygulamalar için taşıyıcı ayarları

#define AT_SAPBR_CON 		 "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n" 	// Bağlantı ayarları.
#define AT_SAPBR_CONN 		 "AT+SAPBR=3,1,\"APN\",\"internet\"\r\n"    // APN ayarı komutu
#define AT_SAPBR_OPEN_BEARER "AT+SAPBR=1,1\r\n"              			// Bearer açmak.
#define AT_SAPBR_QUERY_IP    "AT+SAPBR=2,1\r\n"                 		// IP adres sorgusu
#define AT_HTTP_INIT 		 "AT+HTTPINIT\r\n"         			     	// HTTP hizmeti başlatma.
#define AT_HTTP_TERM 		 "AT+HTTPTERM\r\n"                   	 	// HTTP hizmeti sonlandırma.
#define AT_HTTP_PARA_CID     "AT+HTTPPARA=\"CID\",1\r\n"         		// Bearer ID ayarlamak.
#define AT_HTTP_ACTION_GET   "AT+HTTPACTION=0\r\n"      		 		// HTTP GET eylemi başlatmak.
#define AT_HTTP_ACTION_POST  "AT+HTTPACTION=1\r\n"      		 		// HTTP POST eylemi başlatmak.
#define AT_HTTP_READ 		 "AT+HTTPREAD\r\n"          		  	 	// HTTP sunucu yanıtını okumak.

#define AT_HTTP_JSON     "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n"   // JSON tipinde veri POST işlemi için komut

#define AT_HTTP_SSL_ON   "AT+HTTPSSL=1\r\n"				  // HTTPS/SSL desteğini aktif eder.
#define AT_HTTP_SSL_OFF  "AT+HTTPSSL=0\r\n" 			  // HTTPS/SSL desteğini kapatır.
#define AT_HTTP_REDIR_ON "AT+HTTPPARA=\"REDIR\",1\r\n"    // HTTP yönlendirmelerini aktif eder.


// Komut yanıtları.
#define ANS_OK "OK"
#define ANS_READY "READY"
#define ANS_CGATT_1 "+CGATT: 1"             // GPRS hizmeti durum cevabı
#define ANS_CGREG_1 "+CGREG: 0,1"           // GRPS yetkisi sorgulama cevabı. (Kayıtlı kendi operatörü)
#define ANS_CGREG_5 "+CGREG: 0,5"           // GPRS yetkisi sorgulama cevabı. (Kayıtlı, Roaming)
#define ANS_0_200 "0,200"                   // HTTP GET başarılı durum kodu
#define ANS_1_200 "1,200"                   // HTTP POST başarılı durum kodu
#define ANS_DOWNLOAD "DOWNLOAD"             // HTTP POST yazma isteği cevabı

// AT_CREG Yanıtları.
#define NET_NOT_REGISTERED      0
#define NET_REGISTERED_HOME     1
#define NET_SEARCHING           2
#define NET_REG_DENIED          3
#define NET_UNKNOWN             4
#define NET_REGISTERED_ROAMING  5

// SIM800C fonksiyon sonuç durumları.
typedef enum
{
	SIM800C_Status_Start = 0,
	SIM800C_Status_OK,
	SIM800C_Status_No_Response,
	SIM800C_Status_Network_OK,
	SIM800C_Status_Network_Not_OK,
	SIM800C_Status_GPRS_Conn_Fail,
    SIM800C_Status_HTTP_Read_Fail,
    SIM800C_Status_HTTP_Parse_Fail,
    SIM800C_Status_Buffer_Overflow,
	SIM800C_Status_HTTP_Data_Send_Fail,
	SIM800C_Status_Send_HTTP_Command_Fail
}SIM800C_Status_Type;

extern char ip_adress[20];                   // IP adres kaydedilecek dizi.
extern volatile uint8_t waiting_response;    // SIM800C'den yanıt bekleniyor bayrağı.

// SIM800C başlatma fonksiyonu.
SIM800C_Status_Type SIM800C_Init();

// SIM800C şebeke durumu sorgulama
SIM800C_Status_Type SIM800C_CheckNetworkRegistration(void);

// GPRS hizmetine bağlantı başlatma fonksiyonu.
SIM800C_Status_Type SIM800C_ConnectGPRS(void);

// HTTP GET işlemi başlatma fonksiyonu
SIM800C_Status_Type HTTP_GET(char* url, char* http_data, uint16_t data_max_len);

// HTTP POST işlemi başlatma fonksiyonu.
SIM800C_Status_Type HTTP_POST(char* url, char* data, char* http_response_data, uint16_t data_max_len, uint16_t timeout);

// Başlangıç komutları gönderme yardımcı fonksiyon.
SIM800C_Status_Type SIM800C_Send_Init_Command(char *cmd, char* expected_ans, uint16_t timeout, uint16_t retry_delay_ms);

// Komut gönderme yardımcı fonksiyon.
SIM800C_Status_Type SIM800C_SendCommand(char *cmd, char* expected_ans, uint16_t timeout, uint16_t error_timeout);

// HTTP komutları gönderme yardımcı fonksiyonu.
SIM800C_Status_Type SIM800C_Send_HTTP_Command(char *cmd, uint16_t timeout);

// HTTP servis hizzmeti başlatma fonksiyonu.
SIM800C_Status_Type HTTP_Init();

#endif /* SIM800C_H_ */
