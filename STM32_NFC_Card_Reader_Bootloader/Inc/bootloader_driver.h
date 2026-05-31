#ifndef BOOTLOADER_DRIVER_H_
#define BOOTLOADER_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "systick_driver.h"
#include "boot_config.h"
#include "app_control.h"
#include "flash_driver.h"
#include "metadata.h"
#include "gpio_driver.h"
#include "uart_driver.h"
#include "led_blink.h"
#include "sim800c.h"
#include "firmware_update_helper.h"


// Bootloader işlemleri ve fonksioynları için kontrol durumları.
typedef enum{
	Bootloader_Status_OK = 0,
	Bootloader_Status_Hardware_Init_Error,       // Harici donanım başlatma hatası.
	Bootloader_Status_HTTP_Error,                // HTTP Get veya POST işlemi hatası.
	Bootloader_Status_Metadata_Erase_Error,      // Metadata silme hatası.
	Bootloader_Status_Metadata_Write_Error,      // Metadata yazma hatası.
	Bootloader_Status_Flash_Erase_Error,         // FLASH sector silme hatası.
	Bootloader_Status_Flash_Write_Error,         // FLASH sector yazma hatası.
	Bootloader_Status_Flash_CRC_Error,           // FLASH sector CRC(bütünlük) hatası.
	Bootloader_Status_Firmware_Inactive_Error,   // Güncelleme işlemi için aktif olan firmware'i pasif yapamama hatası.
	Bootloader_Status_Metadata_Read_Error,       // Metadata okuma hatası.
	Bootloader_Status_Metadata_Invalid,          // Metadata geçersiz.
	Bootloader_Status_Main_App_CRC_Error,        // Ana uygulama CRC(bütünlük) hatası.
	Bootloader_Status_Main_App_Invalid,          // Ana uygulama geçersiz.
}Bootloader_Status_Typedef;

// Bootloader başlangıcı için buton okumak.
bool Bootloader_Is_Update_Button_Pressed();

// Bootloader harici donanımları başlatmak.
Bootloader_Status_Typedef Bootloader_Hardware_Init();

// Yeni firmware mevcut mu sorgusu yapar.
Bootloader_Status_Typedef Check_Bootloader_Firmware_Update(char* arr, uint16_t arr_size);

// Bootloader yeni firmware'i karta yüklemek için başlangıç hazırlıklarını gerçekleştirir.
Bootloader_Status_Typedef Bootloader_Initialize_Preparation(void);

// API'den alınan firmware bilgilerine göre güncelleme işlemini gerçekleştirir.
Bootloader_Status_Typedef Bootloader_Perform_Update(http_check_firmware_typedef* firmware_check_object);

// Ana uygulamaya geçiş öncesi bootloader'de kullanılan çevre birimlerini resetler.
void Bootloader_DeInit(void);

// Bootloader'den ana uygulamaya geçmeyi gerçekleştirir.
Bootloader_Status_Typedef Bootloader_Try_Jump_To_Application(void);

#endif /* BOOTLOADER_DRIVER_H_ */
