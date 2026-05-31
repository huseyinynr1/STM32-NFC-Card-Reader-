#include "uart_driver.h"

// USART3 başlangıç konfigürasyonlarını yap ve USART başlat.
void USART_Config(void)
{
	// USART3 için clock hatti aktif et.
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

	// USART3 disable.
	USART3->CR1 &= ~USART_CR1_UE;

	// USART3 Start bit 1, Data bits 8
	USART3->CR1 &= ~USART_CR1_M;

	// Stop bits 1
	USART3->CR2 &= ~(3U << 12);

	// USART3 Oversampling 16 samples
	USART3->CR1 &= ~USART_CR1_OVER8;

	// USART3 Parity disable.
	USART3->CR1 &= ~USART_CR1_PCE;

	// USART3 BRR register sıfırla.
	USART3->BRR &= ~(0xFFFF);

	/* USART_DIV = fck / 8x(2 - OVER8) x Baud Rate ,
	16.000.000 / 16 x 115200 = 8,681 ,
	8 tam kısım Mantissa = 8
	0,786 ondalıklı kısım  Fraction = 0.681 * 16 = 10.89 ≈ 11 */

	USART3->BRR |= (8U << 4) | (11U << 0);

	// USART3 Transmitter Enable
	USART3->CR1 &= ~USART_CR1_TE;
	USART3->CR1 |= USART_CR1_TE;

	// USART3 Receiver Enable
	USART3->CR1 &= ~USART_CR1_RE;
	USART3->CR1 |= USART_CR1_RE;

	// USART3 Enable.
	USART3->CR1 |= USART_CR1_UE;
}

// USART Transmit(veri transferi) işlemini gerçekleştiren fonksiyon.
USART_Status_Typedef USART_Transmit(uint8_t* data, size_t length)
{
	USART_Status_Typedef st;
	uint8_t flag_timeout = 10;

	for(size_t i = 0; i < length; i++)
	{
		// USART DR register'i boş olana kadar bekle.
		st = USART_Wait_Flag(USART_SR_TXE, flag_timeout);
		if(st != USART_Status_OK) return st;
		// USART DR register'a veri yaz
		USART3->DR = data[i];
	}


	// Transfer tamamlanana kadar bekle.
	st = USART_Wait_Flag(USART_SR_TC, flag_timeout);
	if(st != USART_Status_OK) return st;

	return USART_Status_OK;

}

// USART Receive(veri alımı) işlemini gerçekleştiren fonksiyon.
USART_Status_Typedef USART_Receive(uint8_t* data_arr, size_t data_size, uint32_t timeout_ms)
{
	USART_Status_Typedef st;

	uint32_t start = Get_Tick();
	size_t index = 0;
    data_arr[0] = '\0';

    // Dışarıdan gelen timeout süresi içinde RXNE biti 1 olduysa DR register'dan gelen veriyi oku.
	while(Get_Tick() - start < timeout_ms)
	{
		if(USART3->SR & USART_SR_RXNE)
		{
			// SR register'ı oku
			uint32_t sr = USART3->SR;

			// DR register'dan gelen veriyi al.
			uint8_t received_byte = (uint8_t)USART3->DR;

			// Status bayraklarını kontrol et.
			st = USART_Check_Error_Flag(sr);

			// Hata bayraklarından herhangi biri 1 ise bu durumu üst fonksiyona dön.
			if(st != USART_Status_OK)
			{
				USART_Clear_Error_Flags();
				return st;
			}

			// Dışarıdan gelen array büyüklüğüne ulaşana kadar DR register'dan veriyi oku.
			if(index < data_size - 1)
			{
				data_arr[index]= received_byte;
				index++;
				data_arr[index] = '\0';
			}

			// Taşma hatası dön.
			else
			{
				return USART_Status_Buffer_Overflow;
			}

		}
	}

	// Timeout süresi boyunca index artmış ise (yani veri alınıp okunmuş ise) OK dön.
	if(index > 0)
	{
		return USART_Status_OK;
	}

	// Değilse timeout durumu dön.
	return USART_Status_Timeout;
}

// USART status bayrakları sonuçlarını beklemek.
USART_Status_Typedef USART_Wait_Flag(uint32_t flag, uint8_t timeout_ms)
{
	uint32_t start = Get_Tick();

	while((Get_Tick() - start) <= timeout_ms)
	{
		if(USART3->SR & flag) return USART_Status_OK;
	}

	return USART_Status_Timeout;
}


// USART Hata bayrakları kontrolü.
USART_Status_Typedef USART_Check_Error_Flag(uint32_t sr)
{
	// Parity error set ise bu durumu dön.
	if(sr & USART_SR_PE)
	{
		return USART_Status_Parity_Error;
	}

	// Framing error set ise bu durumu dön.
	if(sr & USART_SR_FE)
	{
		return USART_Status_Framing_Error;
	}

	// Noise error set ise bu durumu dön.
	if(sr & USART_SR_NE)
	{
		return USART_Status_Noise_Error;
	}

	// Overrun error set ise bu durumu dön.
	if(sr & USART_SR_ORE)
	{
		return USART_Status_Overrun_Error;
	}

	// Hiçbir hata bayrağı kalkmadıysa OK dön.
	return USART_Status_OK;
}

// USART hata bayrakları temizlemek.
void USART_Clear_Error_Flags(void)
{
	volatile uint32_t temp;

	// USART SR ve DR register'ları okunarak hata bayrakları temizlenir.
	temp = USART3->SR;
	temp = USART3->DR;

	(void)temp;
}

// USART DR register'da önceden kalan verileri temizle.
void USART_Flush_RX(void)
{
    volatile uint32_t temp;

    // RXNE biti 1 olunca DR register oku. Bu şekilde DR register temizlenir.
    while(USART3->SR & USART_SR_RXNE)
    {
        temp = USART3->DR;
        (void)temp;       // Değişkeni kullanma.
    }

    // USART hata bayrakları temizlemek.
    USART_Clear_Error_Flags();
}
