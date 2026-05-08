#include "timer_driver.h"

static volatile uint32_t tim6_ms = 0;

void TIM6_Init(void)
{
	// TIM6 clock hattı aktif
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

	// TIM6 disable güvenli başlangıç.
	TIM6->CR1 &= ~TIM_CR1_CEN;

	/*
	 * APB1 clock hattı 42MHz, APB1 prescaler 4 olduğu için Timer Clock = APB1 x 2 = 84MHz
	 * 1ms interrupt(tick) için PSC = 8400 - 1, 84.000.000/ 8400 = 10kHz
	 * 1ms interrupt(tick) için ARR = 10 - 1, 10 counter adımı 1ms
	 */
	uint32_t TIM6_Timer_Clock_Value = 84000000;
	TIM6->PSC = (TIM6_Timer_Clock_Value / 10000) - 1;

	TIM6->ARR = 10 - 1;

	// Counter(sayaç) değeri başlangıçta 0.
	TIM6->CNT = 0;

	// Timer'in yeni prescaler değerini yüklemesi için update generation.
	TIM6->EGR |= TIM_EGR_UG;

	// UIF(Update Interrupt flag) başlangıçta temizle.
	while(!(TIM6->SR & TIM_SR_UIF));  // UIF'in set olmasını bekle
	TIM6->SR &= ~(TIM_SR_UIF);

	// Update Interrupt Enable.
	TIM6->DIER &= ~(TIM_DIER_UIE);
	TIM6->DIER |=  (TIM_DIER_UIE);

	// Öncelik ayarlama ve NVIC interrupt enable.
	NVIC_SetPriority(TIM6_DAC_IRQn, 15);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);

	// Timer6 başlat.
	TIM6->CR1 |= TIM_CR1_CEN;
}

// Gecikme işlemini sağlayan fonksiyon.
void Delay_Ms(uint32_t delay_time)
{
	uint32_t start = TIM6_Get_Millis();

	while((TIM6_Get_Millis() - start) < delay_time);

	return;
}


// O anki tick değerini verir.
uint32_t TIM6_Get_Millis(void)
{
	uint32_t ms = 0;
	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	ms = tim6_ms;
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	return ms;
}

// TIM6 ve ISR disable işlemini sağlayan fonksiyon.
void TIM6_Disable(void)
{
	TIM6->DIER &= ~(TIM_DIER_UIE);
	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	TIM6->CR1 &= ~TIM_CR1_CEN;
}

// TIM6 ile 1ms'de tick değeri artıran ISR.
void TIM6_DAC_IRQHandler(void)
{
	// Update Interrupt Flag kalktı mı kontrol et.
	if(TIM6->SR & TIM_SR_UIF)
	{
		// UIF temizle ve tim6_ms 1 artır.
		TIM6->SR &= ~(TIM_SR_UIF);
		tim6_ms++;
	}
}
