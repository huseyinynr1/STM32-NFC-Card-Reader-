#include <usart_driver.h>

volatile uint8_t g_usart_tx_ready = 1;
volatile uint8_t rx_buffer[512] = {0};
volatile uint16_t rx_buffer_index = 0;
volatile uint8_t package_ready = 0;
volatile TickType_t last_rx_tick_value = 0;
volatile uint8_t last_byte_received = 0;

void USART3_Config(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  // USART3 clock aktif.
	USART3->CR1 &= ~USART_CR1_UE;
	USART3->CR1 &= ~USART_CR1_M;           // M biti temizleme (8bits Word Length ayarlama.)
	USART3->CR2 &= ~(3U << 12);            // STOP biti temizleme (Stop bits 1 olarak ayarlama)

	USART3->CR1 &= ~USART_CR1_OVER8;       // OVER8 biti temizleme ve 16 samples olarak ayarlama.
	USART3->CR1 &= ~USART_CR1_PCE;         // Parity control disable.

	/* USART_DIV = fck / 8x(2 - OVER8) x Baud Rate ,
	42.000.000 / 16 x 115200 = 22.786 ,
	22 tam kısım Mantissa = 22
	0,786 ondalıklı kısım  Fraction = 0.786 * 16 = 12.58 ≈ 13 */

	USART3->BRR &= ~0xFFFF;                   // DIV_Mantissa biti ve  DIV_Fraction biti temizleme
    USART3->BRR |= (22U << 4) | (13U << 0);   // DIV_Mantissa biti değeri 45 yapma. (Tam kısım), DIV_Fraction biti 9 yapma. (Ondalıklı kısım, 0,5729 x 16 = 9,1 , 9'a yuvarlanır.)

	// USART3 TE biti temizleme ve aktif etme, veri göndermek için hazır
	USART3->CR1 &= ~USART_CR1_TE;
	USART3->CR1 |= USART_CR1_TE;

	// USART3 RE biti temizleme ve aktif etme, veri almak için hazır
	USART3->CR1 &= ~USART_CR1_RE;
	USART3->CR1 |= USART_CR1_RE;

	// USART3 DMA ile transmit işlemi için bit temizleme ve DMA aktif etme.
	USART3->CR3 &= ~USART_CR3_DMAT;
	USART3->CR3 |= USART_CR3_DMAT;

	USART3->CR1 |= USART_CR1_UE;         // USART3 aktif.

	// RXNEIE biti temizleme ve RXNE kesmesi aktif.
	USART3->CR1 |= USART_CR1_RXNEIE;

	// USART3 Receive için kesme fonksiyonu oluşturma,
	NVIC_EnableIRQ(USART3_IRQn);
	NVIC_SetPriority(USART3_IRQn, 6);

}

void DMA1_USART3_TX_Config(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;  // DMA1 clock aktif

	DMA1_Stream3->CR &= ~DMA_SxCR_EN;        // Konfigürasyondan önce stream'i kapat.
	while(DMA1_Stream3->CR & DMA_SxCR_EN);   // SxCR EN biti 0 olana kadar bekle.

	DMA1_Stream3->CR &= ~(7U << 25);     // Kanal seçimi biti temizleme.
	DMA1_Stream3->CR |= (4U << 25);      // Kanal 4 seçme. USART3_TX için Channel 4.

	DMA1_Stream3->CR &= ~(3U << 6);      // DTR biti temizleme
	DMA1_Stream3->CR |= (1U << 6);       // Memory to peripheral

	DMA1_Stream3->CR &= ~DMA_SxCR_CIRC;  // CIRC mode kapalı , normal mode.

	DMA1_Stream3->CR &= ~DMA_SxCR_PINC;  // PINC biti temizleme ve çevre birimi adresi sabit (sadece USART3)

	DMA1_Stream3->CR &= ~DMA_SxCR_MINC;  // MINC Biti temizleme
	DMA1_Stream3->CR |= DMA_SxCR_MINC;   // Memory adres artırma modu. Veriler üst üste binmemeli yanyana sıralanmalı

	DMA1_Stream3->CR &= ~(3U << 11);     // Peripheral size 8-bit
	DMA1_Stream3->CR &= ~(3U << 13);     // Memory size 8-bit

	DMA1_Stream3->CR &= ~(3U << 16);     // Öncelik durumu biti temizleme
	DMA1_Stream3->CR |= (3U << 16);      // Öncelik durumu  Very high.

	DMA1_Stream3->PAR = (uint32_t)&USART3->DR;     // Çevresel birim adresi USART1.

	DMA1_Stream3->CR |= DMA_SxCR_TCIE;   // Transfer tamamlandı kesmesi aktif.

	//NVIC üzerinden kesmeyi aktif etme.
	NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	NVIC_SetPriority(DMA1_Stream3_IRQn, 7);
}


// DMA ile veri gönderme fonksiyonu
void USART3_SendWithDMA(char *message, uint16_t length)
{
	while(g_usart_tx_ready == 0); // Transfer bitene kadar bekle

	g_usart_tx_ready = 0;        // Transfer başladıysa bayrağı indir.

	// Önceki gönderimden kalan bayrakları temizle
	   DMA1->LIFCR |= (DMA_LIFCR_CTCIF3 |
	                    DMA_LIFCR_CHTIF3 |
	                    DMA_LIFCR_CTEIF3 |
	                    DMA_LIFCR_CDMEIF3 |
	                    DMA_LIFCR_CFEIF3);

	DMA1_Stream3->M0AR = (uint32_t)message;   // Gönderilecek mesajı register'a yaz
	DMA1_Stream3->NDTR = length;              // Gönderilecek mesaj boyutunu register'a yaz.

	DMA1_Stream3->CR |= DMA_SxCR_EN; // DMA'yı başlat
}

void USART3_Receive(char *data, uint16_t size)
{
	// RXNEIE biti temizleme ve RXNE kesmesi aktif.
	USART3->CR1 &= ~USART_CR1_RXNEIE;

	if(rx_buffer_index >= size) rx_buffer_index = size - 1;

	if(rx_buffer_index != 0)
	{
		// Geçici dizideki veriyi dışarıdan gelen diziye yaz.
		memcpy(data, (void*)rx_buffer, rx_buffer_index);
		data[rx_buffer_index] ='\0';  // String sonu ekle

		//USART3_Reset_Buffer();
	}

	// RXNEIE biti temizleme ve RXNE kesmesi aktif.
	USART3->CR1 |= USART_CR1_RXNEIE;
}


void USART3_Reset_Buffer(void)
{
	rx_buffer_index = 0;  // Bir sonraki mesaj için index'i sıfırlar
	package_ready = 0;  // Bir sonraki mesaj için bayrağı indir.

	memset((void*)rx_buffer, 0, sizeof(rx_buffer)); // Ana tamponu temizle.
}

// USART3 SIM800 başlangıç veri komutları cevaplarının alınıp almadığına karar veren fonksiyon.
void USART3_Wait_For_Init_Packet(uint32_t timeout_ms, uint32_t silence_ms)
{
	// Başlangıç ms değerini al.
    uint32_t start = TIM6_Get_Millis();

    // En son veri alma ms değerini tutucak değişken.
    uint32_t last_activity = start;

    // Fonksiyon başlangıcında paket hazır flag'ini sıfırla.
    package_ready = 0;

    // Timeput süresi boyunca while içinde kal.
    while((TIM6_Get_Millis() - start) < timeout_ms)
    {
        // ISR byte aldıysa ms'i burada güncelle
        if(last_byte_received)
        {
            last_byte_received = 0;
            last_activity = TIM6_Get_Millis(); // Task context'te güvenli
        }

        // En az bir byte alınmış mı kontrolü
        if(rx_buffer_index > 0)
        {
        	// Şimdiki ms değerini al.
        	uint32_t now = TIM6_Get_Millis();

            /* Şimdiki ms değeri - son byte alınma ms değeri,
             sessizlik süresinden büyükse veri alımı bitmiştir, cevap alınmış ve tamamlanmıştır,
             paket hazır bayrağını kaldır ve fonksiyondan çık*/
            if((now - last_activity) > silence_ms)
            {
                package_ready = 1;
                return;
            }
        }

        // Eğer 1 byte dahi alınmamışsa 1 ms bekle
        Delay_Ms(1);
    }

    // Eğer bir byte dahi alınmamışsa paket hazır değildir.
    package_ready = 0;
}


// UART veri almanın bitip bitmediğine karar veren fonksiyon.
void USART3_Wait_For_Packet(uint32_t timeout_ms, uint32_t silence_ms)
{
	// Başlangıç tick değerini al.
    TickType_t start = xTaskGetTickCount();

    // timeout: belirli bir süre boyunce bekleyip receive hattını kontrol etmek
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);   // dışarıdan gelen timeout değerini tick'e çevir.

    // En son veri alma tick değerini tutucak değişken.
    TickType_t last_activity = start;

    // Fonksiyon başlangıcında paket hazır flag'ini sıfırla.
    package_ready = 0;

    // Timeput süresi boyunca while içinde kal.
    while((xTaskGetTickCount() - start) < timeout_ticks)
    {
        // ISR byte aldıysa tick'i burada güncelle
        if(last_byte_received)
        {
            last_byte_received = 0;
            last_activity = xTaskGetTickCount(); // Task context'te güvenli
        }

        // En az bir byte alınmış mı kontrolü
        if(rx_buffer_index > 0)
        {
        	// Şimdiki tick değerini al.
            TickType_t now = xTaskGetTickCount();

            /* Şimdiki tick değeri - son byte alınma tick değeri,
             sessizlik süresinden büyükse veri alımı bitmiştir, cevap alınmış ve tamamlanmıştır,
             paket hazır bayrağını kaldır ve fonksiyondan çık*/
            if((now - last_activity) > pdMS_TO_TICKS(silence_ms))
            {
                package_ready = 1;
                return;
            }
        }

        // Eğer 1 byte dahi alınmamışsa 1 ms bekle
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Eğer bir byte dahi alınmamışsa paket hazır değildir.
    package_ready = 0;
}
