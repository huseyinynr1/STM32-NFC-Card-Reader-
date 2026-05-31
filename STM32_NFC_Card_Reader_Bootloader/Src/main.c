#include <stdint.h>
#include <stdbool.h>
#include <stm32f4xx.h>
#include "bootloader_driver.h"
#include "firmware_update_helper.h"

int main(void)
{
	// GPIO başlangıç konfigürasyonlarını yap.
    GPIO_Init();

    // SysTick başlangıç konfigürasyonları
    SysTick_Timer_Config();

    Bootloader_Status_Typedef boot_st = Bootloader_Status_OK;

    // Güncelleme işlemi için başlangıçta butona basılma kontrolü.
    if(Bootloader_Is_Update_Button_Pressed())
    {
    	// Harici donanımları başlat.
        boot_st = Bootloader_Hardware_Init();

        // Harici donanımlar başlatıldıysa.
        if(boot_st == Bootloader_Status_OK)
        {
        	// Geçici dizi oluştur.
            char check_firmware_response[256] = {0};

            // Yeni firmware mevcutmu sorgusu.
            boot_st = Check_Bootloader_Firmware_Update(check_firmware_response,
                                                       sizeof(check_firmware_response));

            // Yeni firmware GET sorgusu başarı ile gerçekleştirildiyse:
            if(boot_st == Bootloader_Status_OK)
            {
                http_check_firmware_typedef new_firmware_check = {0};

                // Gelen cevabı ayrıştır ve bilgileri struct yapısındaki değişkenlere ata.
                Firmware_Check_Response_Json_Convert_to_Object(check_firmware_response,
                                                               &new_firmware_check);

                // Alınan firmware bilgilerine göre güncelleme işlemini başlat.
                boot_st = Bootloader_Perform_Update(&new_firmware_check);
            }
        }

        // Güncelleme işlemi sonrası sistemi yeniden başlat.
        Delay_ms(100);
        NVIC_SystemReset();
    }

    // Güncelleme isteği yoksa mevcut uygulamayı doğrula ve çalıştır.
    boot_st = Bootloader_Try_Jump_To_Application();

    while(1)
    {
        // Ana uygulamaya geçilemezse hata durumunu bildir.
        Bootloader_Application_Invalid_Blink();
    }
}
