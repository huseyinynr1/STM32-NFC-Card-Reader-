#include "app_control.h"

// Bootloader harici ana program varmı kontorlü
Application_Status_Typedef Application_Control(void)
{
	// Ana uygulama stack pointer ve Ana uygulama reset handler değerlerini tutucak değişkenler.
	uint32_t app_stack_pointer = 0;
	uint32_t app_reset_handler = 0;

	// Ana uygulama FLASH başlangıç adresindeki (0x08008000) ilk 4 byte'lık değeri al.
	app_stack_pointer = *(uint32_t*)MAIN_APP_START_ADDRESS;

	// Ana uygulama FLASH başlangıç adresindeki(0x08008004) ikinci 4 byte'lık değeri al.
	app_reset_handler = *(uint32_t*)(MAIN_APP_START_ADDRESS + 4U);

	// Ana uygulama FLASH başlangıç adresindeki değer RAM aralığında olmalı. Stack alanı oluşmuş olmalı.
	if(app_stack_pointer < SRAM_START_ADDRESS || app_stack_pointer > SRAM_END_ADDRESS)
	{
		// Şart sağlanmıyorsa bootloader harici uygulama yoktur veya yanlış adreslenmiştir.
		return Application_Invalid;
	}

	// Reset handler değeri FLASH start ve FLASH end adres aralığında olmalı.
	if(app_reset_handler < MAIN_APP_START_ADDRESS || app_reset_handler > FLASH_END_ADDRESS)
	{
		// Şart sağlanmıyorsa bootloader harici uygulama yoktur veya yanlış adreslenmiştir.
		return Application_Invalid;
	}

	// Şartlar sağlandıysa FLASH adresinde bootloader harici uygulama mevcuttur, durumu dön.
	return Application_Valid;
}

// Bootloader'den ana uygulamaya geçiş fonksiyonu.
void Jump_To_Application()
{
    /*
     * Application başlangıç adresindeki ilk 4 byte,
     * application'ın initial stack pointer değeridir.
     */
	uint32_t app_stack_pointer = *(uint32_t*)MAIN_APP_START_ADDRESS;

	/*
	 * Ana uygulama başlangıcı olan Reset handler fonksiyonunun hafızadaki adresini al,
	 * O adresi burada yeni bir fonksiyon oluşturup ana uygulama başlangıç fonksiyonu Reset Handler'ın adresini ver.
	 * Yani App_Reset_Handler: parametre almayan ve geriye void döndüren bir fonksiyonun (Ana uygulama başlangıç fonksiyonu olan Reset_Handler'ın) adresini tutacak.
	 * app_reset_handler sayısal değeri normal bir sayı değeri değil, fonksiyon adres değeridir. Bu değeri bir fonksiyon tipine kast etmek gerekir.
	 * (void(*)(void)): fonksiyon cast'ı
	 */
	uint32_t app_reset_handler = *(uint32_t*)(MAIN_APP_START_ADDRESS + 4U);
	void (*App_Reset_Handler)(void);
	App_Reset_Handler = (void(*)(void))app_reset_handler;

	// Tüm ISR fonksiyonları kapat.
	__disable_irq();

	// Ana uygulamaya geçiş öncesi bootloader'de kullanılan çevre birimlerini resetler.
	Bootloader_DeInit();

	// Vector table'daki adresi, ana uygulama program adresinin başlangıç adresini yap.
	SCB->VTOR = MAIN_APP_START_ADDRESS;
	__set_MSP(app_stack_pointer);

	App_Reset_Handler();  // Ana uygulama başlangıcı olan Reset_Handler() fonksiyonuna git.
}
