#include <stdint.h>
#include <stm32f4xx.h>
#include "fpu_driver.h"
#include "system_clock_driver.h"
#include "timer_driver.h"
#include "gpio_driver.h"
#include "i2c_driver.h"
#include "spi_driver.h"
#include "usart_driver.h"
#include "app_tasks.h"
#include "rc522.h"
#include "sim800c.h"
#include "rtc_ds3231.h"
#include "tft_ili9341.h"

uint32_t SystemCoreClock = 168000000;  // Sistem clock frekansı 168 MHz olarak bildir.

int main(void)
{
	FPU_Init();        			// FPU(Floating Point Unit) Başlat.
	Clock_Config();    			// Sistem clock başlat.
	TIM6_Init();       			// Timer6 başlat.
	GPIOA_Config();    			// GPIO A portu başlat.
	GPIOB_Config();    			// GPIO B portu başlat.
	I2C1_Config();     			// I2C 1 başlat.
	SPI1_Config();     			// SPI 1 Başlat.
	USART3_Config();   			// USART 3 başlat.
	DMA1_USART3_TX_Config();    // DMA 1 başlat. ( USART TX hattı için.)
	Delay_Ms(3000);             // 3 saniye bekle. ( Ayarlar başlatıldıktan sonra stabilize için.)
	bool hardware_ready = false;  // Harici donanımlar hazır bayrağı

	while(!hardware_ready)
	{
		// RC522'yi for düngüsünde 250ms aralıklarla 3 defa başlatmayı dene.
		RC522_Status_Type rc522_st;
		for(int i = 0; i < 5; i++)
		{
			rc522_st = RC522_Init();    // RC522 başlat

			Delay_Ms(250);      		// 250ms bekle.

			// RC hazırsa for döngüsünden çık.
			if(rc522_st == RC522_Status_OK) break;

			// Değilse döngü başına dön.
			else
				continue;
		}

		// RC522 5 kez deneme başlatılamadıysa while başına dön.
		if(rc522_st != RC522_Status_OK) continue;

		Delay_Ms(2000);

		/*
		* SIM800C'yi for dongüsünde 1 saniye aralıklarla 5 defa başlatmayı dene.
		* Başlatıldıysa GPRS bağlantısını yap.
		*/

		SIM800C_Status_Type sim800c_st;
		for(int i = 0; i < 5; i++)
		{
			sim800c_st = SIM800C_Init();

			if(sim800c_st == SIM800C_Status_OK)
			{
				for(int j = 0 ; j < 5; j++)
				{
					sim800c_st = SIM800C_ConnectGPRS();
					if(sim800c_st == SIM800C_Status_OK) break;
				}
			}

			if(sim800c_st == SIM800C_Status_OK) break;
			Delay_Ms(1000);
		}

		if(sim800c_st != SIM800C_Status_OK) continue;
		Delay_Ms(2000);


		// RTC başlat.
		RTC_DS3231_Status rtc_st;
		for(int i = 0; i < 5; i++)
		{
			rtc_st = RTC_Init();
			Delay_Ms(250);
			if(rtc_st == RTC_DS3231_OK) break;
			else
				continue;
		}

		if(rtc_st != RTC_DS3231_OK) continue;
		Delay_Ms(2000);

		// TFT 1 saniye aralıkla 3 kere başlatma dene.
		TFT_Status_t tft_st;
		for(int i = 0; i < 5 ; i++)
		{
			tft_st = TFT_Init();

			if(tft_st == TFT_INIT_SUCCESS){
				char time_buffer[6]     = "00:00";
				char date_buffer[11]    = "01/01/2001";
				tft_st = TFT_Main_Screen(time_buffer, date_buffer);
				if(tft_st != TFT_STATUS_OK) continue;
				TFT_Status_Screen current_screen = MAIN_SCREEN;    // Şuanki ekran bilgisini ana ekran olarak güncelle.
				break;
			}

			// Başarısızsa 1 saniye bekle ve döngünün başına dön
			else
			{
				Delay_Ms(1000);
				continue;
			}
		}

		if(tft_st != TFT_STATUS_OK) continue;
		Delay_Ms(2000);


		hardware_ready = true;  // Tüm harici donanımlar hazır.
	}

	Task_Init();    // FreeRTOS başlat.
}
