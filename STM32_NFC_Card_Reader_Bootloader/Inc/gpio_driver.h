#ifndef GPIO_DRIVER_H_
#define GPIO_DRIVER_H_

#include <stdint.h>
#include <stm32f4xx.h>

// GPIO başlangıç konfigürasyonları.
void GPIO_Init(void);

// GPIO pini LOW yapmak.
void GPIO_WritePin_Low(GPIO_TypeDef *GPIOx, uint8_t pin_number);

// GPIO pini HIGH yapmak.
void GPIO_WritePin_High(GPIO_TypeDef *GPIOx, uint8_t pin_number);

// Bir GPIO pini lojik durumunu okumak.
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin_number);

#endif /* GPIO_DRIVER_H_ */
