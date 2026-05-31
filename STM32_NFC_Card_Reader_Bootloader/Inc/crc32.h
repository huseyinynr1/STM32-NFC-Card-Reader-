#ifndef CRC32_H_
#define CRC32_H_

#include <stdint.h>
#include <stddef.h>

#define CRC32_INITIAL     0xFFFFFFFFU   // CRC32 hesaplamasında kullanılan başlangıç değeri.
#define CRC32_POLYNOMIAL  0xEDB88320U   // CRC32 algoritmasında kullanılan ters polinom değeri.
#define CRC32_FINAL_XOR   0xFFFFFFFFU   // Son CRC sonucuna uygulanan final XOR değeri.

// Buffer üzerindeki veriler için CRC32 bütünlük değeri hesaplar.
uint32_t CRC32_Calculate_Buffer(uint8_t* data, size_t length);

// FLASH bellekteki veriler için CRC32 bütünlük değeri hesaplar.
uint32_t CRC32_Calculate_Flash(uint32_t start_address, size_t length);

#endif /* CRC32_H_ */
