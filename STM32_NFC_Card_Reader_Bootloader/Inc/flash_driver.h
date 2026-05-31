#ifndef FLASH_DRIVER_H_
#define FLASH_DRIVER_H_

#include <stm32f4xx.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "boot_config.h"
#include "crc32.h"

#define FLASH_SR_OPERR (1U << 1)
#define FLASH_SR_RDERR (1U << 8)

// İşlem ve fonksiyon sonuçlarını belirtmek için durum enum yapısı.
typedef enum
{
	FLASH_Start_Value = 0,                    // Başlangıç durum değeri.
	FLASH_Status_OK,                          // İşlem başarılı.
	FLASH_Status_Operation_Error,             // FLASH genel işlem hatası.
	FLASH_Status_Write_Protection_Error,      // Yazma koruması hatası.
	FLASH_Status_Programming_Alignment_Error, // Programlama hizalama hatası.
	FLASH_Status_Programming_Parallelism_Error, // Programlama paralellik hatası.
	FLASH_Status_Programming_Sequence_Error,  // Programlama sıralama hatası.
	FLASH_Status_RD_Error,                    // FLASH okuma koruması hatası.
	FLASH_Status_Invalid_Sector_Number,       // Geçersiz sector numarası.
	FLASH_Status_Invalid_Sector_Address,      // Geçersiz sector adresi.
	FLASH_Status_Erase_Not_Completed,         // Sector silme işlemi tamamlanamadı.
	FLASH_Status_Write_Not_Completed,         // FLASH yazma işlemi tamamlanamadı.
	FLASH_Status_Sector_Not_Empty,            // Sector beklenen şekilde boş değil.
	FLASH_Status_Null_Buffer,                 // Geçersiz veya NULL veri buffer'ı.
	FLASH_Status_Invalid_Length,              // Geçersiz veri uzunluğu.
	FLASH_Status_Data_CRC_Error               // Yazılan verinin CRC kontrolü başarısız.

} FLASH_Status_Typedef;


// FLASH kilidi açmak.
void FLASH_Unlock(void);

// FLASH kilitlemek.
void FLASH_Lock(void);

// FLASH Firmware güncelleme başlangıcını gerçekleştiren fonksiyon.
FLASH_Status_Typedef FLASH_Firmware_Update_Begin(void);

// Firmware parçasını FLASH'a yazar ve yazma adresini günceller.
FLASH_Status_Typedef FLASH_Firmware_Update_Write_Chunk(uint32_t* start_address, uint8_t *data, size_t data_size, uint32_t* received_size);

// Yeni firmware FLASH'a yazıldıktan sonra yapılacak işlemleri kapsar.
FLASH_Status_Typedef FLASH_Firmware_Update_End(uint32_t received_size, uint32_t expected_size, uint32_t expected_crc32);

// Ana uygulama sector blocks temizle.
FLASH_Status_Typedef FLASH_Erase_Application_Area(void);

// Belirtilen FLASH sector'ünü siler.
FLASH_Status_Typedef FLASH_Erase_Sector(uint8_t sector_number);

// Verilen byte dizisini FLASH'a word hizalı olarak yazar.
FLASH_Status_Typedef FLASH_Write_Bytes(uint32_t sector_address, uint8_t* data, size_t data_size);

// FLASH sector adresine veri yazmak.
FLASH_Status_Typedef FLASH_Write_Word(uint32_t sector_address, uint32_t data);

// FLASH durum bayraklarını kontrol etmek.
FLASH_Status_Typedef FLASH_Check_Status_Flag(void);

// FLASH durum bayraklarını temizlemek.
void FLASH_Clear_Status_Flag(void);

// FLASH tüm sector değer kontrolü.
FLASH_Status_Typedef FLASH_Check_Sector_Value(uint32_t sector_address, uint32_t sector_size);

// FLASH sector içindeki 4byte'lık alan değer kontrolü.
uint32_t FLASH_Check_Sector_Bit_Value(uint32_t sector_address);

// uint8_t veri dizisini FLASH yazımı için uint32_t word dizisine dönüştürür.
void FLASH_Helper_Uint8_to_Uint32(uint32_t* word_data, uint32_t word_count, uint8_t* data, size_t data_size);

#endif /* FLASH_DRIVER_H_ */
