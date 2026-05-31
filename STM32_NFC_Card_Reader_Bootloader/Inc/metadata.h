#ifndef METADATA_H_
#define METADATA_H_

#include "flash_driver.h"

#define METADATA_SIGNATURE  0x48594657U            // Metadata aitlik değeri. (ASCII = HYFW)
#define METADATA_VALID_FLAG 0xA5A5A5A5U

// Metadata işlemler ve fonksiyonların sonuç durum enum yapısı.
typedef enum{
	Metadata_Start_Value = 0,               // Enum başlangıç değeri.
	Metadata_Status_OK,                     // Sonuç veya işlem başarılı.
	Metada_Status_Erase_Not_Successfull,    // Metadata silme işlemi başarısız.
	Metadata_Status_Write_Not_Successfull,  // Metadata flash'a yazma başarısız.
	Metadata_Read_Error,                    // Metadata değişkenleri için FLASH'tan veri okuma başarısız.
	Metadata_Status_Signature_Error,        // Metadata aitlik hatası.
	Metadata_Status_Metadata_Invalid,       // Metadata geçerli değil hatası.
	Metadata_Status_Invalid_Firmware_Size,  // Hatalı boyut değeri.
	Metadata_Status_CRC_Error,              // Hatalı CRC değeri.
}Metadata_Status_Typedef;


// Firmware doğrulama ve geçerlilik bilgilerini tutan metadata yapısı.
typedef struct
{
	uint32_t metadata_signature;  // Metadata alanının geçerli yapıya ait olduğunu belirtir.
	uint32_t firmware_version;    // Yüklü firmware sürüm bilgisini tutar.
	uint32_t firmware_size;       // Yüklü firmware boyutunu byte cinsinden tutar.
	uint32_t firmware_crc32;      // Yüklü firmware için beklenen CRC32 değerini tutar.
	uint32_t valid_flag;          // Firmware'in geçerli olduğunu belirten bayrak değeri.
	uint32_t metadata_crc32;      // Metadata yapısının CRC32 bütünlük değerini tutar.

} Firmware_Metadata_Typedef;

// Metadata alanını silme.
Metadata_Status_Typedef Metadata_Erase();

// Metadata struct nesnesini FLASH'a yazma.
Metadata_Status_Typedef Metadata_Write(Firmware_Metadata_Typedef* metadata);

// Metadata değerlerini FLASH'tan okuma işlemi
Metadata_Status_Typedef Metadata_Read(Firmware_Metadata_Typedef* metadata);

// FLASH'tan okunan metadata bilgilerinin geçerliliğini kontrol eder.
Metadata_Status_Typedef Metadata_Is_Valid(Firmware_Metadata_Typedef* metadata);

// Metadata yapısının CRC32 bütünlük değerini hesaplar.
uint32_t Metadata_Calculate_CRC(Firmware_Metadata_Typedef* metadata);

#endif /* METADATA_H_ */
