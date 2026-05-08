#include <system_clock_driver.h>

// MCU'yu ayağa kaldırmak için ilk olarak sistem saat kaynağı belirlenir ve etkinleştirilir.
void Clock_Config(void)
{
	RCC->CR |= RCC_CR_HSEON;             // HSE aktif etme
	while(!(RCC->CR & RCC_CR_HSERDY));   // HSE kararlı olana kadar bekle.

	RCC->CR |= RCC_CR_CSSON;             // CSS(Clock Security System) enable. HSE izlenip arıza veya bağlantı kopulmasını görmek için.
	RCC->APB1ENR  |= RCC_APB1ENR_PWREN;  // İç voltaj reg. yüksek performans moduna almak için PWR register birimini aktif etme.
	PWR->CR |= PWR_CR_VOS;               // Max. performasn için (168MHz) voltaj reg. scale ayarı (scale = 1, yüksek performans ).
	FLASH->ACR |= FLASH_ACR_LATENCY_5WS; // İşlemci saat frekansı 168MHz ise  işlemci flash'tan komutları doğru şekilde okuyabilsin diye 5 saat çevrimi bekleme. (FLASH= 30MHz)


	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLSRC); // PLL yapılandırması yapılacak PLL bitlerini temizleme.
	RCC->PLLCFGR |= (8 << 0) | (336 << 6) | (0 << 16) | RCC_PLLCFGR_PLLSRC_HSE; // 168MHz yapmak için PLL değerleri M = 8, N = 336, P = 2 ve PLL saat kaynak HSE(1 << 22).

	//Clock hattı hızlarını ayarla. PLL aktif edilmeden ayarlanmalı aksi halde APB1 ve APB2 hatlarındaki çevre birimlerinde sorunlar çıkabilir.
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;  // APB2 = 84MHz Prescaler = 2
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;  // APB1 = 42MHz Prescaler = 4
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   // AHB = 168MHz Prescaler = 1

	RCC->CR |= RCC_CR_PLLON;           // PLL aktif et.
	while(!(RCC->CR & RCC_CR_PLLRDY)); // PLL hazır oluncaya kadar bekle.

	// Sistem saat kaynağı PLL seçme.
	RCC->CFGR &= ~RCC_CFGR_SW;        // Sistem saat kaynağı sıfırla
	RCC->CFGR |= RCC_CFGR_SW_PLL;    // Sistem saat kaynağı PLL seç.
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // PLL sistem saati olarak doğrulanana kadar bekle
}
