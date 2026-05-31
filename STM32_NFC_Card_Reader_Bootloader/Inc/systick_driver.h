#ifndef SYSTICK_DRIVER_H_
#define SYSTICK_DRIVER_H_

#include <stdint.h>
#include <stm32f4xx.h>
#include "boot_config.h"

// SysTick başlangıç konfigürasyonları
void SysTick_Timer_Config(void);

//SysTick ISR
void SysTick_Handler(void);

// Gecikme fonksiyonu
void Delay_ms(uint32_t delay_value);

// Tick değerini verir.
uint32_t Get_Tick(void);

#endif /* SYSTICK_DRIVER_H_ */
