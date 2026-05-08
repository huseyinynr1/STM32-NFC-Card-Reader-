#include "interrupt_handlers.h"


void DMA1_Stream3_IRQHandler(void)
{
	// Transfer control bayrağı kalktı mı kontrol et.
	if(DMA1->LISR & DMA_LISR_TCIF3)
	{
		DMA1->LIFCR |= DMA_LIFCR_CTCIF3; // Bayrağı temizle.
		DMA1_Stream3->CR &= ~DMA_SxCR_EN;
		while(!(USART3->SR & USART_SR_TC));
		g_usart_tx_ready = 1;            // Transfer bittiyse bayrak kaldır.

	}
}


void USART3_IRQHandler(void)
{
	volatile uint32_t sr = 0;
	sr = USART3->SR;

	// Hata bayrakları kalktıysa veri hatalı gelmiştir veriyi oku ama işlem yapma.
	if(sr & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE))
	{
		(void)USART3->DR;
		return;
	}

	// RXNE biti 1 olmuş ise veri gelmiştir veriyi al.
    if(sr & USART_SR_RXNE)
    {
    	// DR register oku ve değeri al
        uint8_t received_data = (uint8_t)USART3->DR;

        // Cevap beklenmiyorsa veriyi oku ama işlem yapma.
        if(!waiting_response)
            return;

        // Alınan veriyi tutucak dizi index'i son index değerine kadar dolu değilse veriyi al.
        if(rx_buffer_index < sizeof(rx_buffer) - 1)
        {
            rx_buffer[rx_buffer_index++] = received_data;  // Alınan veriyi tutucak diziye aktar , index'i artır.
            last_byte_received = 1;                        // Veri alındı bayrağı kaldır.
        }
    }
}


