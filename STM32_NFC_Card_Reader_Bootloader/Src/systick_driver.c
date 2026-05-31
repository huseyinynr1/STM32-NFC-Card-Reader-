#include "systick_driver.h"

static volatile uint32_t count = 0;

// SysTick başlangıç konfigürasyonları
void SysTick_Timer_Config(void)
{
	/*
	 * 1 ms'de tick için:
	 * T = 1 / f, 0,001 / 1 = f, f = 1000Hz
	 * HSI = 16MHz, F = 1kHz, 16.000.000 / 1000 = 16.000, CPU 1ms için 16.000 clock adımı atması gerekir.
	 * */
	SysTick->LOAD = (CPU_CLOCK_HZ / 1000) - 1;

	// SysTick değeri sıfırla
	SysTick->VAL = 0;

	// Clock Source seçimi Processor clock (AHB)
	SysTick->CTRL |= (1U << 2);

	// SysTick sıfıra geldiğinde interrupt üretsin.
	SysTick->CTRL |= (1U << 1);

	// SysTick enable.
	SysTick->CTRL |= (1U << 0);
}


//SysTick ISR
void SysTick_Handler(void)
{
	count++;
}

// Gecikme fonksiyonu
void Delay_ms(uint32_t delay_value)
{
	uint32_t start = count;

	while(count - start < delay_value);
}

// Tick değerini verir.
uint32_t Get_Tick(void)
{
	return count;
}
