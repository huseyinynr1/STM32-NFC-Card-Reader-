#ifndef USART_DRIVER_H_
#define USART_DRIVER_H_

#include <stm32f4xx.h>
#include <stdint.h>
#include <string.h>
#include "timer_driver.h"
#include "FreeRTOS.h"
#include "task.h"


extern volatile uint8_t g_usart_tx_ready;  		// Transfer bayrağı.
extern volatile uint8_t rx_buffer[512];    		// DR register'a veri geldiğinde , registerdan bu diziye veri alınacak.
extern volatile uint16_t rx_buffer_index;   	// Veri alma dizisi index değeri tututcak değişken.
extern volatile uint8_t package_ready;     		// Veri alma bitti paket hazır bayrağı.
extern volatile TickType_t last_rx_tick_value;  // Son veri alınma tick değeri.
extern volatile uint8_t last_byte_received;		// Son veri alınma bilgisi tutucak bayrak.



// USART3 için başlangıç konfigürasyon ayarları fonksiyonu
void USART3_Config(void);

// USART3 Transmit için DMA2 etkinleştirme.
void DMA1_USART3_TX_Config(void);

// DMA ile veri gönderme fonksiyonu
void USART3_SendWithDMA(char *message, uint16_t length);

// DR registerine gelen verileri alıp diziye yazma fonksiyonu.
void USART3_Receive(char *data, uint16_t size);

// USART3 bayrayaklar ve alınan veri dizisini sıfırlama fonksiyonu.
void USART3_Reset_Buffer(void);

// USART3 SIM800 başlangıç veri komutları cevaplarının alınıp almadığına karar veren fonksiyon.
void USART3_Wait_For_Init_Packet(uint32_t timeout_ms, uint32_t silence_ms);

// USART3 SIM800C için veri almanın bitip bitmediğine karar veren fonksiyon.
void USART3_Wait_For_Packet(uint32_t timeout_ms, uint32_t silence_ms);

#endif /* USART_DRIVER_H_ */
