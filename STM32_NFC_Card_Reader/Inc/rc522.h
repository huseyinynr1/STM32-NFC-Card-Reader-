#ifndef RC522_H_
#define RC522_H_

#if defined(__GNUC__)
  #define PACKED_STRUCT __attribute__((packed))
#else
  #define PACKED_STRUCT
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  #define STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)
#else
  #define STATIC_ASSERT(cond, msg) typedef char static_assertion_##msg[(cond) ? 1 : -1]
#endif

#include <stm32f4xx.h>
#include <stdbool.h>
#include <string.h>
#include "timer_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi_driver.h"
#include "helper_function.h"
#include "rtc_ds3231.h"

#define Version_Reg 0x37       // RC522 versiyon bilgisini veren register.
#define T_Reload_Reg_H 0x2C    // Timer için yeniden yükleme değeri.
#define FIFO_Level_Reg 0x0A    // FIFO'da saklanan bayt sayısını gösterir.
#define Command_Reg 0x01       // RC522 komut register.
#define Demod_Reg 0x19         // Demodülatör ayarlarını tanımlar.
#define T_Prescaler_Reg_L 0x2B // Timer prescaler low register
#define T_Reload_Reg_L 0x2D    // Timer reloda low register
#define T_Mode_Reg 0x2A        // Zamanlayıcı ayarlarını tanımlar.
#define Com_Irq_Reg 0x04       // Kesme bitlerini içeren register.
#define Tx_Mode_Reg 0x12       // İletim iiçin veri hızını tanımlayan register.
#define Rx_Mode_Reg 0x13       // Alım için veri hızını tanımlayan register.
#define Mf_Rx_Reg 0x1D         // MIFARE iletişimde bazı alma parametrelerini kontrol eden register.
#define Bit_Framing_Reg 0x0D   // Bit odaklı çerçeceler için ayarlamaları yapan register.
#define Tx_Control_Reg 0x14    // Anten sürücü pinleri TX1 ve TX2'nin mantıksal davranışını kontrol eder.
#define Tx_ASK_Reg 0x15        // İletim modülasyonu ayarlarını kontrol eder.
#define Div_Irq_Reg 0x05       // Kesme bitlerini içeren register.
#define FIFO_Data_Reg 0x09     // FIFO data register.
#define Error_Reg 0x06         // Hangi hata durumu olduğunu belirten register.
#define Control_Reg 0x0C       // Bazı çeşitli kontrol bitleri içerir.
#define Status2_Reg 0x08       // Alıcı, verici ve authenticate işlemleri için durum bitleri içerir.

// CommandReg Command biti değer tanımlamaları
#define Transceive 0xC
#define Idle 0x00
#define MFAuthent 0xE

// Kart ve kart authenticate işlemleri için komutlar
#define KEYA 0x60                 // KEYA adresi
#define KEYB 0x61                 // KEYB adresi
#define REQA 0x26                 // Request A (kart tanıma isteği) adresi.
#define WUPA 0x52                 // Wakeup A (kart uyandırm isteği) adresi.
#define MIFARE_Write 0xA0         // Karta veri yazmak için komut kodu
#define MIFARE_Read 0x30          // Karttan veri okumak için komut kodu
#define MIFARE_Halt {0x50, 0x00}  // Kart durdurma komutu
#define KEYA_Default_Value {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  // KEYA default değeri
#define KEYB_Default_Value {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  // KEYB default değeri
#define Access_Bits_Default_Value {0xFF, 0x07, 0x80}             // Access Bits default değeri
#define General_Purpose_Default_Value 0x69
#define KEYA_Private_Value {0x48, 0x55, 0x53, 0x45, 0x59, 0x49}  // KEYA yeni değeri ( HUSEYI )
#define KEYB_Private_Value {0x4E, 0x59, 0x41, 0x4E, 0x41, 0x52}  // KEYB yeni değeri ( NYANAR )
#define Access_Bits_Private_Value {0x0F, 0xF0, 0x00}             // KEYA read, KEYB read/write

// Kart içindeki veri tutan blok adresleri
#define card_sector1_block0 0x04
#define card_sector1_block1 0x05
#define card_sector1_block3 0x07

#define Magic_Number 0x4859         // Mifare kartların bu projeue ait olduğunu bildiren numaralar. (HY)
#define MAX_ALLOWED_BALANCE 30000   // İzin verilen max bakiye (300 tl)
#define Required_Time 5             // Kart bu süre içinde tekrar okutulursa tipi ne olursa olsun tam ücret tarifesi uygulanır.

// RC522 işlemler sonucu durum tipleri.
typedef enum{
	RC522_Status_Start_Value = 0,   	// Enum başlangıç durumu.
	RC522_Status_OK,                	// Genel başarılı sonuç durumu.
	RC522_Reg_Read_Fail,            	// Register okuma başarısız durumu sonucu.
	RC522_Reg_Write_Fail,           	// Register yazma başarısız durumu sonucu.
	RC522_SPI_Comm_Fail,            	// SPI bağlantı sorunu.
	RC522_Antenna_Enable_Fail,      	// RC522 anten aktif değil durumu.
	RC522_Selftest_Error_IRQ,       	// RC522 Selftest hata bayrakları kalkması durumu.
	RC522_Selftest_Timeout,         	// RC522 Selftest zaman aşımı durumu.
	RC522_Wait_Timeout,             	// RC522 IRQ bayrakları bekleme ve okuma durumunda zaman aşımı durumu.
	RC522_Wait_Error,               	// RC522 IRQ bayrakları bekleme ve okuma durumunda hata durumu
	RC522_Card_Found,               	// RC522 kart bulundu durumu.
	RC522_No_Card_Present,          	// RC522 kart bulunmadı durumu.
	RC522_Card_ID_Found,            	// RC522 kart id bulundu durumu.
	RC522_Card_ID_Not_Found,        	// RC522 kart id bulunamadı durumu.
	RC522_Auth_Error_Reg_Set,       	// RC522 Authenticate işleminde hata durumu.
	RC522_Auth_Crypto1_Not_Enabled, 	// RC522 Authenticate işleminde crypto1 aktif olmadu durumu.
	RC522_Test_Read_Fail,           	// RC522 Test işleminde okuma başarısız durumu.
	RC522_Test_Read_Invalid_Response,   // RC522 Test işleminde geçersiz cevap durumu.
	RC522_Card_Wakeup_Fail,             // RC522 kart uyandırma işlemi başarısız durumu.
	RC522_Auth_Not_Active,              // RC522 Authenticate işlemi aktif değil durumu.
	RC522_Mifare_Read_Error_Reg_Set,    // RC522 kart bloğundan veri okuma işleminde hata durumu.
	RC522_Mifare_Read_Invalid_Response, // RC522 kart bloğundan veri okuma işleminde geçersiz cevap durumu.
	RC522_Mifare_Write_Error_Reg_Set,   // RC522 kart bloğuna veri yazarken hata durumu.
	RC522_Mifare_Write_CMD_ACK_Fail,    // RC522 kart bloğuna veri yazarken CMD ACK hatası.
	RC522_Mifare_Write_Data_ACK_Fail,   // RC522 kart bloğuna veri yazarken Data ACK hatası.
	RC522_Card_ID_Mismatch,             // RC522 okutulan kart id sistemde yoksa hatalı durum bildirimi.
	RC522_Card_Data_CRC_Fail,           // Kart bloğundan veri okuma işleminde okunan verilerin bütünlük kontrolü hata durumu.
	Balance_Upload_Successfull,   		// Yeni bakiye yükleme başarılı.
	Process_Successfull,          		// İşlem başarılı.
	Process_NotSuccessfull,       		// İşlem başarısız.
	Balance_Insufficient,         		// Bakiye yetersiz.
	Is_Expired,                   		// Son kullanma tarihi geçmiş.
	Visa_Expired,                 		// Vize süresi dolmuş.
	RC522_Status_Error
}RC522_Status_Type;

// RC522 Kart tipleri
typedef enum
{
	Full_Fare_Card = 0x00,        // Tam kart.
	Student_Card = 0x01,          // Öğrenci kart.
	Teacher_Card = 0x02,          // Öğretmen kart.
	Senior_Citizen_Card = 0x03,   // Yaşlı kart.
	Disabled_Person_Card = 0x04,  // Engelli kart.
}RC522_Card_Type;


// RC522 kart durum tipleri.
typedef enum
{
	New_Card = 0,          // Yeni kart
	Registered_Card,   // Kayıtlı kart
	Invalid_Card       // Geçersiz kart.
}RC522_Card_Status_Type;

// RC522 kart tarife ücretleri.
typedef enum
{
	Full_Fare_Card_Fare = 4000,     // Tam kart ücret tarifesi (40 tl)
	Student_Card_Fare = 2000,       // Öğrenci kart ücret tarifesi (20 tl)
	Teacher_Card_Fare = 2000,       // Öğretmen kart ücret tarifesi (20 tl)
	Senior_Citizen_Card_Fare = 0,   // Yaşlı kart ücret tarifesi (0 tl)
	Disabled_Person_Card_Fare = 0,  // Engelli kart ücret tarifesi (0 tl)
}RC522_Card_Fare_Type;


// RC522 kart bloklarına yazılacak veri tipleri. ( sector1 block 0 )
typedef struct PACKED_STRUCT{
	uint8_t magic_number[2];       // 2 byte proje no: 0. ve 1. bayt
	uint8_t version;               // 1 byte versiyon no: 2.bayt
	RC522_Card_Type card_type;     // 1 byte kart tipi : 3. bayt
	uint8_t uid[4];                // 4 byte kart id: 4...7. bayt
	uint32_t operation_counter;    // 4 byte yapılan işlem için sayaç: 8...11. bayt
	uint16_t expiry_date;          // 2 byte son kullanma tarihi: 12. ve 13. bayt
	uint16_t crc;                  // 2 byte crc (bütünlük doğrulama) : 14. ve 15.bayt
}RC522_Card_Header;
STATIC_ASSERT(sizeof(RC522_Card_Header) == 16, "card_header must be 16 bytes");

// RC522 kart bloklarına yazılacak veri tipleri. ( sector1 block 1 )
typedef struct PACKED_STRUCT{
	uint32_t balance;             // 4 byte bakiye: 0...3. bayt
	uint32_t operation_counter;   // 4 byte yapılan işlem için sayaç: 4...7.bayt
	uint32_t max_balance;         // 4 byte yüklenebilecek max bakiye: 8...11.bayt
	uint16_t visa_date;           // 2 byte vize tarihi :12. ve 13. bayt
	uint16_t crc;                 // 2 byte crc (bütünlük doğrulama) : 14. ve 15.bayt
}RC522_Card_Balance;

STATIC_ASSERT(sizeof(RC522_Card_Balance) == 16, "card_balance must be 16 bytes");

// RC522 kart bloklarına yazılacak veri tipleri. ( sector1 block 3 )
typedef struct PACKED_STRUCT{
	uint8_t key_A[6];            // 6 byte KEYA değeri : 0...5.bayt.
	uint8_t access_bits[3];      // 3 byte AccessBits değeri : 6...8.bayt.
	uint8_t general_purpose;     // 1 byte genel kullanım byte'ı: 9.bayt
	uint8_t key_B[6];            // 6 byte KEYB değeri: 10...15.bayt.
}RC522_Card_Security;
STATIC_ASSERT(sizeof(RC522_Card_Security) == 16, "card_security must be 16 bytes");

// RC522 NFC başlangıç konfigürasyonları.
RC522_Status_Type RC522_Init(void);

// İlk çalışma testi yapılacak fonksiyon.
RC522_Status_Type RC522_Selftest(void);

// Authenticate test etmek için fonksiyon.
RC522_Status_Type RC522_Authenticate_Test(void);

// HALT durumundaki bir kartı WUPA komutu ile tekrar cevap verebilir hale getirmek için kullanılır.
RC522_Status_Type RC522_WakeupCard(void);

// Kart var mı diye sorgulama fonksiyonu.
RC522_Status_Type RC522_IsNewCardPresent(void);

// Kart'ın UID'sini alma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_ReadCardSerial(uint8_t *uid, uint8_t* len);

// Karta Authenticate olma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_Authenticate(uint8_t block_addrr, uint8_t auth_command, uint8_t* key, uint8_t* card_id);

// Yeni karta, API'den alınan kart bilgilerini aktarma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_CardPersonalization(new_card_response_helper* new_card_info,
		uint8_t* card_id, uint8_t block_addrr_header, uint8_t block_addrr_balance);

// Kart bloğundan okuma yapma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_MIFARE_READ(uint8_t block_addrr, uint8_t* read_buffer);

// Kart bloğuna yazma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_MIFARE_Write(uint8_t block_addrr, uint8_t* data, uint8_t data_len);

// Kart bloğundan kart bilgilerini alma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_Get_Card_Info(uint8_t* card_id, RC522_Card_Header *card_header_temp,
		RC522_Card_Balance *card_balance_temp);

// Kart tipine göre karttan tutar miktarını azaltıp yeni bakiyeyi oluşturan fonksiyon.
RC522_Status_Type RC522_Passanger_Card_Balance_Transaction(RC522_Card_Header* card_header_temp,
		RC522_Card_Balance* card_balance_temp,card_balance_info_helper *card_balance_info_helper_temp,
		RTC_DS321_Time* time_temp);

// Yeni bakiye yükleme isteği olursa bakiye güncelleme.
RC522_Status_Type RC522_Balance_Top_Up(RC522_Card_Header* card_header_temp,
		RC522_Card_Balance* card_balance_temp,uint32_t new_balance);

// Crypto durdurma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_StopCrypto(void);

// Kartı HALT durumuna alma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_HALT_Card(void);

// RC522 ile veri alışverişini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_Transceive(uint8_t* transceive_data, uint8_t txLen, uint8_t tx_last_bits, uint8_t* rxBuffer,
		                           uint8_t* rxLen, uint8_t rxMaxLen, uint8_t* rxLastBits, bool crc_enable,
								   bool expect_response);

// RC522 ile veri alışverişi başlatma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_StartSend(bool start_sent_value);

// RC522 TxLastBits değeri belirleme.
RC522_Status_Type RC522_SetTxLastBits(uint8_t tx_last_bits);

// RC522 FIFO temizleme.
RC522_Status_Type RC522_FIFO_Flush(void);

// RC522 IRQ bayraklarını temizleme.
RC522_Status_Type RC522_IRQ_Clear(void);

// RC522 RxLastBits değeri okuma.
RC522_Status_Type RC522_ReadRxLastBits(uint8_t* rx_last_bits);

// Tx_Mode_Reg CRC etkinleştir veya devre dışı bırak.
RC522_Status_Type RC522_TxCRCEnDis(uint8_t value);

// RC522 CommandReg ile yapılacak komut işlemi değerini gönderme.
RC522_Status_Type RC522_CommandReg_CommandBitValue(uint8_t cmd);

// RC522 FIFO'ya veri yazma
RC522_Status_Type RC522_Write_FIFO(uint8_t* data, uint8_t len);

// RC522 FIFO'dan veri okuma
RC522_Status_Type RC522_Read_FIFO(uint8_t* buffer, uint8_t len);

// RC522 Transceive işleminde IRQ bayraklarını kontrol etme.
RC522_Status_Type RC522_WaitIRQForTransceive(void);

// RC522 Authenticate işleminde IRQ bayraklarını kontrol etme.
RC522_Status_Type RC522_WaitIRQForMFAuthent(void);

// RC522 Register'lara veri yazma.
RC522_Status_Type RC522_WriteReg(uint8_t reg, uint8_t data);

// RC522 Register'lardan veri okuma.
RC522_Status_Type RC522_ReadReg(uint8_t reg, uint8_t* value);

// Test için kart sector block'lardan veri silme.
RC522_Status_Type RC522_Clear_Card_Blocks(uint8_t block0_addr, uint8_t block1_addr);

#endif /* RC522_H_ */
