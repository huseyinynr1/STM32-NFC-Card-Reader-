#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

#include <stm32f4xx.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "systick_driver.h"

typedef enum
{
	USART_Status_OK = 0,
	USART_Status_Parity_Error,
	USART_Status_Framing_Error,
	USART_Status_Noise_Error,
	USART_Status_Overrun_Error,
	USART_Status_Timeout,
	USART_Status_Buffer_Overflow,
}USART_Status_Typedef;

// USART3 başlangıç konfigürasyonlarını yap ve USART başlat.
void USART_Config(void);

// USART Transmit(veri transferi) işlemini gerçekleştiren fonksiyon.
USART_Status_Typedef USART_Transmit(uint8_t* data, size_t length);

// USART Receive(veri alımı) işlemini gerçekleştiren fonksiyon.
USART_Status_Typedef USART_Receive(uint8_t* data_arr, size_t data_size, uint32_t timeout_ms);

// USART status bayrakları sonuçlarını beklemek.
USART_Status_Typedef USART_Wait_Flag(uint32_t flag, uint8_t timeout_ms);

// USART Hata bayrakları kontrolü.
USART_Status_Typedef USART_Check_Error_Flag(uint32_t sr);

// USART hata bayrakları temizler.
void USART_Clear_Error_Flags(void);

// USART DR register temizler.
void USART_Flush_RX(void);

#endif /* UART_DRIVER_H_ */
