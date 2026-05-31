#include "gpio_driver.h"

// GPIO başlangıç konfigürasyonları.
void GPIO_Init(void)
{
	// GPIOD clock enable.
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

	// PD12 Output mode.
	GPIOD->MODER &= ~GPIO_MODER_MODER12;
	GPIOD->MODER |= (1U << 24);

	// PD12 Output push pull.
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT12;

	// PD12 Medium speed.
	GPIOD->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR12;
	GPIOD->OSPEEDR |= (1U << 24);

	// PD12 no pull-up and pull-down
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPD12;

	// PA0 input mode.
	GPIOA->MODER &= ~GPIO_MODER_MODER0;

	// PA0 no pull up and pull down.
	GPIOA->PUPDR &= ~(3U << 0);


	// PA15 (SIM800C_PWRKEY) için mod seçimi, General purpose Output
	GPIOA->MODER &= ~(3U << 30);
	GPIOA->MODER |=  (1U << 30);

	// PA15 (SIM800C_PWRKEY) için çıkış tipi seçimi, output push-pull
	GPIOA->OTYPER &= ~(1U << 15);


	// PA15 (SIM800C_PWRKEY) için çıkış hızı seçimi, very high speed
	GPIOA->OSPEEDR &= ~(3U << 30);
	GPIOA->OSPEEDR |=  (3U << 30);

	// PA15 (SIM800C_PWRKEY) için pull-up/pull-down seçimi, no pull-up/no pull-down
	GPIOA->PUPDR &= ~(3U << 30);

	// PB10(USART3_TX) AF Mode
	GPIOB->MODER &= ~(3U << 20);
	GPIOB->MODER |= (2U << 20);

	// PB10(USART3_TX) output push pull
    GPIOB->OTYPER &= ~(1U << 10);

	// PB10(USART3_TX) Speed mode Very high speed.
	GPIOB->OSPEEDR &= ~(3U << 20);
	GPIOB->OSPEEDR |=  (3U << 20);

	// PB10(USART3_TX) no pull up and pull down.
	GPIOB->PUPDR &= ~(3U << 20);

	// PB10(USART3_TX) AF7 (USART3 -> AF7)
	GPIOB->AFR[1] &= ~(0xFU << 8);
	GPIOB->AFR[1] |= (7U << 8);

	// PB11(USART3_RX) AF Mode
	GPIOB->MODER &= ~(3U << 22);
	GPIOB->MODER |= (2U << 22);

	// PB11(USART3_RX) output push pull
    GPIOB->OTYPER &= ~(1U << 11);

	// PB11(USART3_RX) Speed mode Very high speed.
	GPIOB->OSPEEDR &= ~(3U << 22);
	GPIOB->OSPEEDR |= (3U << 22);

	// PB11(USART3_RX) pull up.
	GPIOB->PUPDR &= ~(3U << 22);
	GPIOB->PUPDR |=  (1U << 22);

	// PB11 (USART1 RX) AF seçimi USART1.
	GPIOB->AFR[1] &= ~(0xFU << 12);
	GPIOB->AFR[1] |= (7U << 12);
}

// GPIO pini LOW yapmak.
void GPIO_WritePin_Low(GPIO_TypeDef *GPIOx, uint8_t pin_number)
{
	GPIOx->BSRR = (1U << (pin_number + 16));
}

// GPIO pini HIGH yapmak.
void GPIO_WritePin_High(GPIO_TypeDef *GPIOx, uint8_t pin_number)
{
	GPIOx->BSRR = (1U << pin_number);
}

// Bir GPIO pini lojik durumunu okumak.
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin_number)
{
	if(GPIOx->IDR & (1U << pin_number))
	{
		return 1;
	}

	else
	{
		return 0;
	}
}
