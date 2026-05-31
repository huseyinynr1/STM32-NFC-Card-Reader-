#include "led_blink.h"

// Bootloader'e giriş bilgisi için led blink.
void Bootloader_Entry_Blink(void)
{
	for(int i = 0; i < 4; i++)
	{
		GPIO_WritePin_High(GPIOD, 12);
	    Delay_ms(500);
		GPIO_WritePin_Low(GPIOD, 12);
	    Delay_ms(500);
	}
}

void Bootloader_Application_Invalid_Blink(void)
{
	for(int i = 0; i < 6; i++)
	{
		GPIO_WritePin_High(GPIOD, 12);
	    Delay_ms(1000);
		GPIO_WritePin_Low(GPIOD, 12);
	    Delay_ms(1000);
	}
}

void Bootloader_Metadata_Invalid_Blink(void)
{
	for(int i = 0; i < 8; i++)
	{
		GPIO_WritePin_High(GPIOD, 12);
	    Delay_ms(250);
		GPIO_WritePin_Low(GPIOD, 12);
	    Delay_ms(250);
	}
}
