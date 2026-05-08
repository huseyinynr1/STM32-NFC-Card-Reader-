#ifndef INTERRUPT_HANDLERS_H_
#define INTERRUPT_HANDLERS_H_

#include <stm32f4xx.h>
#include "usart_driver.h"
#include "spi_driver.h"
#include "sim800c.h"

// USART Transfer tamamlandı kesmesi.
void DMA1_Stream3_IRQHandler(void);

// USART Veri alma kesmesi.
void USART3_IRQHandler(void);
#endif /* INTERRUPT_HANDLERS_H_ */
