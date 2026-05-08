#ifndef APP_TASKS_H_
#define APP_TASKS_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "spi_driver.h"
#include "rc522.h"
#include "sim800c.h"
#include "helper_function.h"
#include "rtc_ds3231.h"
#include "tft_ili9341.h"

extern QueueHandle_t card_balance_info_queue_for_screen;
extern QueueHandle_t http_request_queue;
extern QueueHandle_t get_new_card_info_queue;
extern QueueHandle_t new_card_post_response_queue;
extern QueueHandle_t topup_response_queue;
extern QueueHandle_t topup_post_response_queue;
extern QueueHandle_t time_info_queue_for_rfid;
extern QueueHandle_t time_info_queue_for_screen;

// Task , Queue ve Mutex başlatmak.
void Task_Init();

// RC522 görev fonksiyonu.
void vRFID_TASK(void *pvParameters);

// SIM800C görev fonksiyonu.
void vSIM800C_Task(void *pvParameters);

//RTC DS3231 görev fonksiyonu.
void vTimer_Service_Task(void *pvParameters);

// TFT görev fonksiyonu.
void vTFT_LCD_Task(void *pvParameters);

#endif /* APP_TASKS_H_ */
