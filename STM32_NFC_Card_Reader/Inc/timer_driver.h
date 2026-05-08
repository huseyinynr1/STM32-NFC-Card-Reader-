#ifndef TIMER_DRIVER_H_
#define TIMER_DRIVER_H_

#include <stm32f4xx.h>

// TIM6 başlangıç konfigürasyonları.
void TIM6_Init(void);

// Gecikme işlemini sağlayan fonksiyon.
void Delay_Ms(uint32_t delay_time);

// O anki tick değerini verir.
uint32_t TIM6_Get_Millis(void);

// TIM6 ve ISR disable işlemini sağlayan fonksiyon.
void TIM6_Disable(void);

// TIM6 ile 1ms'de tick değeri artıran ISR.
void TIM6_DAC_IRQHandler(void);

#endif /* TIMER_DRIVER_H_ */
