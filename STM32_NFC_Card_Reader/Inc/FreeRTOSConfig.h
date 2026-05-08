#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* STM32F4xx Standart kütüphane veya CMSIS başlığı */
#include <stdint.h>
extern uint32_t SystemCoreClock; // Saat hızını otomatik çekmesi için

/******************************************************************************/
/* Temel Zamanlama ve Çekirdek Ayarları                                       */
/******************************************************************************/
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) // 1ms Tick
#define configMAX_PRIORITIES                    ( 7 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 130 )
#define configMAX_TASK_NAME_LEN                 ( 10 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               8
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configGENERATE_RUN_TIME_STATS           0

/******************************************************************************/
/* Bellek Yönetimi (Heap 4 kullanıyorsan)                                     */
/******************************************************************************/
// STM32F407 için 20KB heap başlangıç için idealdir.
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 20 * 1024 ) )
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

/******************************************************************************/
/* Yazılımsal Zamanlayıcılar (Software Timers)                                */
/******************************************************************************/
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( 2 )
#define configTIMER_QUEUE_LENGTH                5
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/******************************************************************************/
/* API Fonksiyonlarını Dahil Etme                                             */
/******************************************************************************/
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTaskGetSchedulerState          1

/******************************************************************************/
/* STM32F407 Kesme (Interrupt) Ayarları - ÇOK KRİTİK                          */
/******************************************************************************/
/* STM32F407 4 bit öncelik (priority) kullanır. */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS                     __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                     4
#endif

/* En düşük kesme önceliği */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15

/* FreeRTOS'un içinden çağrılabilecek en yüksek kesme önceliği (Safe-Zone) */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* Donanımsal öncelik değerleri */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/******************************************************************************/
/* Kesme Vektör Yönlendirmeleri - BARE-METAL İÇİN HAYATİ                      */
/******************************************************************************/
/* Bu tanımlamalar, startup_stm32f407xx.s dosyasındaki standart isimleri
   FreeRTOS'un port fonksiyonlarına bağlar. */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Geliştirme aşamasında hataları yakalamak için Assert */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */
