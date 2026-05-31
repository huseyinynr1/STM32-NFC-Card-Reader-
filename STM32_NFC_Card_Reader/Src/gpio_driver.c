#include <gpio_driver.h>

void GPIOA_Config(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // GPIOA portu clock hattı aktif etme.

	// PA4(Chip Select) output mode config
	GPIOA->MODER &= ~(3U << 8);       // 8. ve 9. biti temizle
	GPIOA->MODER |= (1U << 8);        // Output Mode 9.bit =0 , 8.bit = 1
	GPIOA->OTYPER &= ~(1U << 4);      // 4.bit temizle, Push - pull
	GPIOA->OSPEEDR &= ~(3U << 8);     // 8. ve 9. bit temizleme
	GPIOA->OSPEEDR |= (3U << 8);      // Very high speed mode, 9.bit =1 , 8.bit = 1
	GPIOA->PUPDR &= ~(3U << 8);       // 8. ve 9. biti temizleme
	GPIOA->PUPDR |= (1 << 8);         // Pull-up mode 9.bit = 0 , 8.bit = 1
	GPIOA->BSRR = GPIO_BSRR_BS4;     // Başlangıç olarak lojik 1 (Herhangi bir spi cihazı hemen bağlanmasın.)

	// PA5 (SCK) AF mode config.
	GPIOA->MODER &= ~(3U << 10);     // 10. ve 11.biti temizle(PA5)
	GPIOA->MODER |= (2U << 10);      // Alternate function mode, 11.bit = 1, 10.bit = 0 (PA5)
	GPIOA->OSPEEDR &= ~(3U << 10);   // 10. ve 11.biti temizle(PA5)
	GPIOA->OSPEEDR |= (3U << 10);    // Very high speed mode, 11.bit = 1, 10.bit = 1 (PA5)
	GPIOA->AFR[0] &= ~(0xFU << 20);  // 20., 21., 22., 23. bitleri temizle
	GPIOA->AFR[0] |= (5U << 20);     // Alternate Function 5 (SPI1).

	// PA6 (MISO) AF mode config.
	GPIOA->MODER &= ~(3U << 12);    // 12. ve 13. biti temizleme (PA6)
	GPIOA->MODER |= (2U << 12);     // AF mode 13. bit= 1 , 12.bit = 0 (PA6)
	GPIOA->OSPEEDR &= ~(3U << 12);  // 12. ve 13. biti temizleme (PA6)
	GPIOA->OSPEEDR |= (3U << 12);   // Very high speed mode, 13.bit = 1, 12.bit = 1 (PA6)
	GPIOA->AFR[0] &= ~(0xFU << 24); // 24., 25., 26., 27. bitleri temizle (PA6)
	GPIOA->AFR[0] |= (5U << 24);   // Alternate Function 5 (SPI1). (PA6)

	// PA7 (MOSI) AF mode config.
	GPIOA->MODER &= ~(3U << 14);    // 14. ve 15. biti temizleme (PA7)
	GPIOA->MODER |= (2U << 14);     // AF mode 15. bit= 1 , 14.bit = 0 (PA7)
	GPIOA->OSPEEDR &= ~(3U << 14);  // 14. ve 15. biti temizleme (PA7)
	GPIOA->OSPEEDR |= (3U << 14);   // Very high speed mode, 15.bit = 1, 14.bit = 1 (PA7)
	GPIOA->AFR[0] &= ~(0xFU << 28); // 28., 29., 30., 31. bitleri temizle (PA7)
	GPIOA->AFR[0] |= (5U << 28);   // Alternate Function 5 (SPI1). (PA7)

	// PA3 (NRSTPD) mode Output
	GPIOA->MODER &= ~(3U << 6);
	GPIOA->MODER |= (1U << 6);

	// PA3 (NRSTPD) output type Output push-pull
	GPIOA->OTYPER &= ~(1U << 3);

	// PA3 (NRSTPD) output speed Very high speed
	GPIOA->OSPEEDR &= ~(3U << 6);
	GPIOA->OSPEEDR |= (3U << 6);

	// PA3 (NRSTPD) pull-up/pull-down: No pull-up, pull-down
	GPIOA->PUPDR &= ~(3U << 6);

	// PA3 (NRSTPD) başlangıçta HIGH olarak başlasın
	GPIOA->BSRR = GPIO_BSRR_BS3;


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

	// PA12(SIM800C_STATUS) mode input
	GPIOA->MODER &= ~(3U << 24);

	// PA12(SIM800C_STATUS) no pull-up/no pull-down
	GPIOA->PUPDR &= ~(3U << 24);

}


void GPIOB_Config(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  //GPIOB clcok hattı aktif.

	// PB6 Mod bitleri temizleme ve AF yapma.
	GPIOB->MODER &= ~(3U << 12);
	GPIOB->MODER |= (2U << 12);

	// PB6 çıkış tipi temizleme ve open drain yapma.
	GPIOB->OTYPER &= ~(1U << 6);
	GPIOB->OTYPER |= (1U << 6);

	// PB6 Speed bitleri temizleme ve Very High Speed ayarlama.
	GPIOB->OSPEEDR &= ~(3U << 12);
	GPIOB->OSPEEDR |= (3U << 12);

	// PB6 için Alternate Function seçimi
	GPIOB->AFR[0] &= ~(0xFU << 24);
	GPIOB->AFR[0] |= (4U << 24);

	// PB7 Mod bitleri temizleme ve AF yapma.
	GPIOB->MODER &= ~(3U << 14);
	GPIOB->MODER |= (2U << 14);

	// PB7 çıkış tipi temizleme ve open drain yapma.
	GPIOB->OTYPER &= ~(1U << 7);
	GPIOB->OTYPER |= (1U << 7);

	// PB7 Speed bitleri temizleme ve Very High Speed ayarlama.
	GPIOB->OSPEEDR &= ~(3U << 14);
	GPIOB->OSPEEDR |= (3U << 14);

	// PB7 için Alternate Function seçimi
	GPIOB->AFR[0] &= ~(0xFU << 28);
	GPIOB->AFR[0] |= (4U << 28);

	// PB5 mode input
	GPIOB->MODER &= ~(3U << 10);

	// PB5 no pull-up pull-down
	GPIOB->PUPDR &= ~(3U << 10);

	// PB0(TFT_CS) General Purpose Output
	GPIOB->MODER |= (1U << 0);

	// PB0(TFT_CS) Output push-pull
	GPIOB->OTYPER &= ~(1U << 0);

	// PB0(TFT_CS) Output Speed medium
	GPIOB->OSPEEDR &= ~(3U << 0);
	GPIOB->OSPEEDR |=  (1U << 0);

	// PB0(TFT_CS) No pull-up, pull-down
	GPIOB->PUPDR &= ~(3U << 0);

	// PB0(TFT_CS) HIGH olarak başlasın.
	GPIOB->BSRR |= (1U << 0);


	// PB1(TFT_LED) General Purpose Output
	GPIOB->MODER |= (1U << 2);

	// PB1(TFT_LED) Output push-pull
	GPIOB->OTYPER &= ~(1U << 1);

	// PB1(TFT_LED) Output Speed low
	GPIOB->OSPEEDR &= ~(3U << 2);

	// PB1(TFT_LED) No pull-up, pull-down
	GPIOB->PUPDR &= ~(3U << 2);

	// PB1(TFT_LED) HIGH olarak başlasın.
	GPIOB->BSRR |= (1U << 1);


	// PB2(TFT_DATA_COMMAND) General Purpose Output
	GPIOB->MODER |= (1U << 4);

	// PB2(TFT_DATA_COMMAND) Output push-pull
	GPIOB->OTYPER &= ~(1U << 2);

	// PPB2(TFT_DATA_COMMAND) Output Speed medium
	GPIOB->OSPEEDR &= ~(3U << 4);
	GPIOB->OSPEEDR |=  (1U << 4);

	// PB2(TFT_DATA_COMMAND) No pull-up, pull-down
	GPIOB->PUPDR &= ~(3U << 4);

	// PB2(TFT_DATA_COMMAND) HIGH olarak başlasın.
	GPIOB->BSRR |= (1U << 2);


	// PB3(TFT_RST) General Purpose Output
	GPIOB->MODER |= (1U << 6);

	// PB3(TFT_RST) Output push-pull
	GPIOB->OTYPER &= ~(1U << 3);

	// PB3(TFT_RST) Output Speed low
	GPIOB->OSPEEDR &= ~(3U << 6);

	// PB3(TFT_RST) No pull-up, pull-down
	GPIOB->PUPDR &= ~(3U << 6);

	// PB3(TFT_RST) HIGH olarak başlasın.
	GPIOB->BSRR |= (1U << 3);

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
