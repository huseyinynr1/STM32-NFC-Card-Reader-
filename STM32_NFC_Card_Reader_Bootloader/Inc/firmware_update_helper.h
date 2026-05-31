#ifndef FIRMWARE_UPDATE_HELPER_H_
#define FIRMWARE_UPDATE_HELPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Yeni firmware bilgisi sorgusu için GET endpoint.
#define GET_CHECK_FIRMWARE_URL 				"https://ocean-circumstances-falling-disclaimer.trycloudflare.com/api/firmware/latest"

// Yeni firmware dosya parçaları bilgisi için GET endpoint.
#define GET_FIRMWARE_CHUNK_URL 				"https://ocean-circumstances-falling-disclaimer.trycloudflare.com/api/firmware/chunk/raw"

// Güncelleme yapıldıktan sonra dosyayı inaktif yapmak ve güncelleme gerçekleştirildiğini bildirmek için POST URL.
#define POST_FIRMWARE_ISACTIVE_UPDATE_URL   "https://ocean-circumstances-falling-disclaimer.trycloudflare.com/api/firmware/update-active"

// Alınacak max yeni firmware dosya parçası boyutu.
#define FIRMWARE_CHUNK_MAX_SIZE   4096

// Aktif yeni firmware bilgisi için GET isteği ile dönen response'taki verileri tutucak structure.
typedef struct{
	bool success;				// GET isteği başarılı ile gerçekleştirildimi bilgisi tutucak değişken.
	bool isActive;              // API tarafında güncellemeye hazır aktif firmware kaydını belirten cevapı tutucak değişken.
	uint8_t firmware_id;        // Yeni firmware id'si tutucak değişken.
	uint32_t firmware_version;  // Yeni firmware versiyon bilgisi tutucak değişken.
	uint32_t firmware_size;     // Yeni firmware büyüklüğü tutucak değişken.
	uint32_t firmware_crc32;    // Yeni firmware crc(bütünlük) değeri tutucak değişken.
}http_check_firmware_typedef;


void Firmware_Check_Response_Json_Convert_to_Object(char* arr, http_check_firmware_typedef* response_object);

// Firmware karta başarıyla yüklendiği cevabı Json olarak hazırlamak.
void Firmware_Update_IsActive_Convert_Json(char* arr, uint8_t firmware_id, bool is_active);

void Firmware_Update_IsActive_Response_Json_Convert(char* arr, bool *success);

// // Yeni firmware dosya parça değerini almak GET isteği ile almak için gerekli URL'i oluşturan fonksiyon.
void Firmware_Build_Chunk_URL(char* out_url, size_t out_url_size, const char* base_url, uint32_t firmware_id, uint32_t offset, uint16_t chunk_size);

uint32_t Helper_Convert_Firmware_Version(uint8_t major, uint8_t minor, uint8_t patch);

#endif /* FIRMWARE_UPDATE_HELPER_H_ */
