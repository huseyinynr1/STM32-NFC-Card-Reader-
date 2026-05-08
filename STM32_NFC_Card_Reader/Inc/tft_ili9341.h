#ifndef TFT_ILI9341_H_
#define TFT_ILI9341_H_

#include "gpio_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi_driver.h"
#include <string.h>
#include "font_and_images.h"

#define POWER_CONTROL_A 0xCB      // TFT'nin iç voltaj regülatörünü ayarlayan register.
#define PCA_Parameter_1 0x39      // Vcore voltajı değeri
#define PCA_Parameter_2 0x2C      // DDVDH (ekranın source driver bloğunun besleme voltajı) değeri (5.6V)
#define PCA_Parameter_3 0x00      // Ekranın referans voltajı değeri. (Varsayılan voltaj değeri)
#define PCA_Parameter_4 0x34      // VRH ayarı, GVDD (gri ton voltaj üretecinin referansı) değeri. (4.8V)
#define PCA_Parameter_5 0x02      // SAP ayarı (source amplifier) akımı değeri.	(Orta seviye akım)

#define POWER_CONTROL_B 0xCF      // TFT'nin güç devrelerini hangi sırayla devreye gireceğini ayarlayan register.
#define PCB_Parameter_1 0x00      // Reserved alan, 0 gönderilir.
#define PCB_Parameter_2 0xA2      // Güç tasarrufu ve VCOM güçlendirmesi aktif değeri
#define PCB_Parameter_3 0xF0      // ESD koruması açık değeri.

/* TFT ekran 320 satırdan oluşuyor. Gate sürücüsü bu satırları birer birer sırayla açıp kapatıyor, her satır açıkken
 o satırın piksellerine renk voltajı uygulanıyor. Driver Timing control A bu açıp kapatma işleminin sırasını kontrol ediyor.*/
#define Driver_Timing_Control_A 0xE8    // TFT panelin gate sürücülerinin açılış zamanlamasını yapan register.
#define DTCA_Parameter_1        0x84    // Bir satır kapanmadan diğerini açmamak değeri.

// Bir satırı kapatırken o satırdaki voltajı sıfırlama zamanı(EQ) ve gate sürücüsü şarj oranı(CR) kontrol eden değer.
#define DTCA_Parameter_2        0x11
#define DTCA_Parameter_3        0x7A    // Satır açılırken düzgün açılması için ön şarj değeri.

// LCD panelin gate sürücülerinin GND ve DDVDH arasındaki geçişlerinin ne kadar süreceğini belirleyen register.
#define Driver_Timing_Control_B 0xEA
#define DTCB_Parameter_1        0x66  // Gate'in GND ve DDVDH arasındaki geçiş 1-2 unit süre
#define DTCB_Parameter_2        0x00  // Anlamsız bitlere 0 değeri gönder.

#define Power_On_Sequence_Control 0xED  // Güç açılırken hangi birimin ne zaman devreye gireceğini ayarlayan register.
#define POSC_Parameter_1          0x55  // Soft start 2 frame süre değeri.
#define POSC_Parameter_2          0x01  /* İlk VCL(negatif voltaj kaynağı) açılsın , DDVDH (piksel sürücüsünün ana besleme voltajı)
2. frame'de devreye girme değeri. */
#define POSC_Parameter_3          0x23  /* VGH (gate sürücüsünün pozitif voltajı) 3.frame'de
VGL (gate sürücüsünün negatif voltajı) 4.frame'de devreye girsin.*/
#define POSC_Parameter_4          0x01  // DDVDH güçlendirme modu aktif değeri

#define Pump_Ratio_Control 0xF7   // Charge pump devresi voltaj oranını ayarlayan register.
#define PRC_Parameter_1    0x10   // DDVDH (piksel sürücüsü besleme voltajı) 2xVCI yanı 6.6V değeri

#define MADCTL_Reg       0x36     // TFT yön ayarları yapılan register.
#define MADCTL_Parameter 0x48     // Dikey (portrait), yukarıdan aşağıya ve soldan sağa yazma ve yenileme.
#define COLMOD_Reg       0x3A     // RGB565 ve RGB666 seçme ve ayarlama register.
#define COLMOD_Parameter 0x55     // RGB565 formatı seçme.
#define Sleep_Out_Reg    0x11     // Uyku modundan çıkma registerı
#define Display_On_Reg   0x29     // Ekranı açma registerı.

#define Column_Address_Set_Reg 0x2A   // Sütun seçmeye yarayan register.
#define Page_Address_Set_Reg   0x2B   // Satır seçmeye yarayan register.
#define Memory_Write_Reg       0x2C   // Piksele veri yazmak için gerekli register.

// TFT fonksiyonları sonuç durumları.
typedef enum
{
	TFT_STATUS_OK = 0,
	TFT_INIT_SUCCESS,
	TFT_STATUS_ERROR,
	TFT_STATUS_SPI_COMM_FAIL,
	TFT_STATUS_WRITE_COMMAND_FAIL,
	TFT_STATUS_WRITE_DATA_FAIL
}TFT_Status_t;

// TFT'de güncel ekranın hangi ekran olduğunu bildiren durumlar.
typedef enum
{
	MAIN_SCREEN = 0,        // Ana ekran
	NEW_CARD_SCREEN,        // Yeni kart ekranı
	UPLOAD_BALANCE_SCREEN,  // Bakiye yüklendi ekranı.
	SUCCESS_SCREEN,         // İşlem başarılı ekranı.
	FAIL_SCREEN             // İşlem başarısız ekranı.
}TFT_Status_Screen;


// TFT başlangıç konfigürasyonları.
TFT_Status_t TFT_Init(void);

// Power Control A register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Power_Control_A(void);

// Power Control B register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Power_Control_B(void);

// Driver Timing Control A register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Driver_Timing_Control_A(void);

// Driver Timing Control B register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Driver_Timing_Control_B(void);

// Power On Sequence Control register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Power_On_Sequence_Control(void);

// Pump Ratio Control register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Pump_Ratio_Control(void);

// Memory Access Control register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Memory_Access_Control(void);

// COLMOD register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Pixel_Format(void);

// TFT uykudan çıkarma.
TFT_Status_t TFT_Config_Sleep_Out(void);

// TFT ekranı açma.
TFT_Status_t TFT_Config_Display_On(void);

// Ana ekran tasarımı.
TFT_Status_t TFT_Main_Screen(const char* time, const char* date);

// Yeni kart kaydetme işlemi başarılı için ekran tasarımı.
TFT_Status_t TFT_New_Card_Saved_Screen(void);

// Bakiye yükleme işlemi bildirim ekranı.
TFT_Status_t TFT_Upload_Balance_Screen(char* loaded_amount, char* new_balance);

// İşlem başarılı ekranı tasarımı.
TFT_Status_t TFT_Process_Successfull_Screen(char* fare, char* balance);

// İşlem başarısız ekran tasarımı.
TFT_Status_t TFT_Process_Not_Successfull_Screen(uint8_t status);

// Ekrandaki tarihi güncelleme fonksiyonu.
TFT_Status_t TFT_Update_Date(const char* date);

// Ekrandaki zamanı güncelleme fonksiyonu
TFT_Status_t TFT_Update_Time(const char* time);

// Bir alan seçip o alanı belirtilen renkler ile doldurma
TFT_Status_t TFT_Fill_Rect(uint16_t start_col, uint16_t end_col, uint16_t start_row, uint16_t end_row , uint16_t color);

// Ekrana birden çok karakteri çizme.
TFT_Status_t TFT_Draw_String(uint16_t x, uint16_t y, const char* str, uint16_t text_color, uint16_t background_color, float scale);

TFT_Status_t TFT_Draw_Image(uint16_t x, uint16_t y, uint8_t width, uint16_t height, const uint16_t* image_array);

// Tek bir karakteri ekranda çizme.
TFT_Status_t TFT_Draw_Char(uint16_t x, uint16_t y, const char ch, uint16_t text_color, uint16_t background_color, float scale);

// Çizilecek karakterin yerini ve rengini gönderme.
TFT_Status_t TFT_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color);

// Belirli bir alan seç.
TFT_Status_t TFT_Set_Window(uint16_t start_col, uint16_t end_col, uint16_t start_row, uint16_t end_row);

// İşlem yapılacak sütunları seç.
TFT_Status_t TFT_Set_Column(uint16_t start_col, uint16_t end_col);

// İşlem yapılacak satırları seç.
TFT_Status_t TFT_Set_Row(uint16_t start_row, uint16_t end_row);

// Belirlenen alana işlenecek verileri gönderme.
TFT_Status_t TFT_Send_Pixel(uint16_t pixel);

// TFT CS pini LOW yapan fonksiyon.
void TFT_CS_Low();

// TFT CS pini HIGH yapan fonksiyon.
void TFT_CS_High();

// TFT'ye SPI ile komut yolla.
TFT_Status_t TFT_Write_Command(uint8_t cmd);

// TFT'ye SPI ile veri yolla.
TFT_Status_t TFT_Write_Data(uint8_t data);


#endif /* TFT_ILI9341_H_ */
