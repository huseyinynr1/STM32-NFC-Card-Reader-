#include "spi_driver.h"

static SemaphoreHandle_t spi1_mutex = NULL;

// SPI1 konfigürasyonları.
void SPI1_Config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	 	// SPI1 saati aktif.

	SPI1->CR1 &= ~(7U << 3);             	// SPI BR bitlerini temizle.
	SPI1->CR1 |= (2U << 3);             	// fpclk / 8 = 10.5Mbits/s

	SPI1->CR1 &= ~SPI_CR1_RXONLY;           // Full duplex (Transmit and receive9 modu seç.)
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;         // 2-line veri yolu modu seç.

	SPI1->CR1 &= ~SPI_CR1_CPHA;             // CPHA ilk kenarda veri yakalayacak.
	SPI1->CR1 &= ~SPI_CR1_CPOL;             // CPOL low, yükselen kenarda veri yakalanacak.

	SPI1->CR1 &= ~SPI_CR1_DFF;              // 8 bit veri formatı.

	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;         // Önce en anlamlı(MSB) bitleri gönder.

	SPI1->CR1 &= ~SPI_CR1_SSM;              // SSM biti temizle.
	SPI1->CR1 |= SPI_CR1_SSM;               // SSM=1 yazılımsal slave kontrolü aktif.
	SPI1->CR1 &= ~SPI_CR1_SSI;              // SSI biti temizle.
	SPI1->CR1 |= SPI_CR1_SSI;               // SSI=1 yazılımsal slave kontrolü aktif.

	SPI1->CR1 &= ~SPI_CR1_MSTR;             // MSTR biti temizle
	SPI1->CR1 |= SPI_CR1_MSTR;              // Bu cihazı master olarak ayarla.

	SPI1->CR1 |= SPI_CR1_SPE;               // SPI1'i aktif et.
}

// SPI transfer işlemi fonksiyonu.
SPI_Status_t SPI_Transfer(uint8_t out, uint8_t* value)
{
	uint32_t start = TIM6_Get_Millis();

	while(!(SPI1->SR & SPI_SR_TXE))     // DR boş olana ve yeni veri yazılabilir hale gelene kadar bekle.
	{
		if(SPI1->SR & SPI_SR_MODF)
		{
			return SPI_STATUS_MODF_ERR;
		}

		if(TIM6_Get_Millis() - start > 5)
		{
			return SPI_STATUS_TIMEOUT;
		}
	}

	*(__IO uint8_t*)&SPI1->DR = out;     // Veriyi DR register'ına yaz.(8bitlik yazma olacağı için register'i 8 bite cast et.)

	while(!(SPI1->SR & SPI_SR_RXNE))     // Alınan veri DR'ye gelene kadar bekle.
	{
		if(SPI1->SR & SPI_SR_OVR)
		{
			return SPI_STATUS_OVR_ERR;
		}

		if(SPI1->SR & SPI_SR_MODF)
		{
			return SPI_STATUS_MODF_ERR;
		}

		if(TIM6_Get_Millis() - start > 5)
		{
			return SPI_STATUS_TIMEOUT;
		}
	}

	uint8_t rx_data = *(__IO uint8_t*)&SPI1->DR;
	if(value != NULL)
	{
		*value = rx_data;    // Okunan veriyi döndür.
	}

	while(SPI1->SR & SPI_SR_BSY)          // İşlem bitene kadar bekle
	{
		if(TIM6_Get_Millis() - start > 5)
		{
			return SPI_STATUS_TIMEOUT;
		}
	}

	return SPI_STATUS_OK;
}


// SPI1 için recursive mutex oluşturma.
void SPI1_Mutex_Init(void)
{
	// SPI1 için recursive mutex oluştur , eğer oluşturma başarısız ise programı durdur.
	spi1_mutex = xSemaphoreCreateRecursiveMutex();

	configASSERT(spi1_mutex != NULL);
}


// SP1 için recursive mutex alma fonksiyonu.
bool SPI1_Lock(TickType_t timeout)
{
	/*
	 * xTaskGetSchedulerState() FreeRTOS scheduler'ın durum bilgisini verir.
	 * taskSCHEDULER_NOT_STARTED Bir FreeRTOS sabitidir, FreeRTOS scheduler henüz başlatılmadı bilgisini ifade eder.
	 * Eğer FreeRTOS scheduler henüz başlamadıysa SPI kullanımına izin ver.
	 */
	if(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
	{
		return true;
	}

	/*
	 * xSemaphoreTakeRecursive() fonksiyonu oluşturulan recursive mutex'i almaya çalışır.
	 * Eğer SPI1 mutex'i boşta ise ver, değilse timeout kadar bekle.
	 * Mutex almayı timeout süresi içinde dene.
	 * Başarılı olursa pdTrue, başarısız olursa pdFalse döner.
	 */
	if(xSemaphoreTakeRecursive(spi1_mutex, pdMS_TO_TICKS(timeout)) == pdTRUE)
	{
		return true;
	}

	return false;
}


// SPI için recursive mutex verme fonksiyonu.
void SPI1_Unlock(void)
{
	// Eğer FreeRTOS scheduler henüz başlamadıysa SPI kullanımına izin ver.
	if(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
	{
		return;
	}

	// Recursive mutex ver (serbest bırak).
	xSemaphoreGiveRecursive(spi1_mutex);
}
