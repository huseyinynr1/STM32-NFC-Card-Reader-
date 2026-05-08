#ifndef FONT_AND_IMAGES_H_
#define FONT_AND_IMAGES_H_

#define FONT_TR_UPPER_I_DOTTED   123   // İ
#define FONT_TR_LOWER_S_CEDILLA  124   // ş
#define FONT_TR_LOWER_I_DOTLESS  125   // ı

#include <stdint.h>

// 5x7 ASCII tablosuna göre karakterler.
extern const uint8_t font5x7[][5];

// IBB logosu için RGB565 C array
extern const uint16_t ibb_logo[9216];

// IBB Kart logosu için RGB565 C array
extern const uint16_t card_logo[18432];

// Istanbul yazısı için RGB565 C array
extern const uint16_t ist[4096];

// 2G logosu için RGB565 C array
extern const uint16_t two_g_logo[1024];


#endif /* FONT_AND_IMAGES_H_ */
