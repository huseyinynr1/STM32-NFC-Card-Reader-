#ifndef BOOT_CONFIG_H_
#define BOOT_CONFIG_H_


#define BOOTLOADER_START_ADDRESS 0x08000000U    // Bootloder'in FLASH'ta yazılmaya başlanacağı ilk adres.
#define BOOTLOADER_SIZE  		 (32U * 1024U)  // Bootloader'e ait sector'ler boyutu. (32kb)

#define METADATA_START_ADDRESS   0x08008000U    // Metadata'nın FLASH'ta yazılmaya başlanacağı ilk adres.
#define METADATA_SIZE            (16u * 1024U)  // Metadata'ya ait sector boyutu. (16kb)

#define MAIN_APP_START_ADDRESS   0x0800C000U    // Ana program'ın FLASH'ta yazılmaya başlanacağı ilk adres.
#define MAIN_APP_SIZE			 (464U * 1024U) // Ana uygulamaya ait sector'ler toplam boyutu.

#define FLASH_START_ADDRESS    	 0x08000000U    // STM32F407VET6 FLASH başlangıç adresi.
#define FLASH_SIZE 				 (512U * 1024U) // STM32F407VET6 FLASH toplam boyutu.
#define FLASH_END_ADDRESS        (FLASH_START_ADDRESS + FLASH_SIZE)  // STM32F407VET6 FLASH bitiş adresi.

#define SRAM_START_ADDRESS 		 0x20000000U    // STM32F407VET6 SRAM başlangıç adresi
#define SRAM_SIZE				 (128U * 1024U) // STM32F407VET6 SRAM boyutu
#define SRAM_END_ADDRESS		 (SRAM_START_ADDRESS + SRAM_SIZE)    // STM32F407VET6 SRAM bitiş adresi.

#define FLASH_SECTOR_0   0x08000000U            // FLASH SECTOR 0 Adresi
#define FLASH_SECTOR_1   0x08004000U			// FLASH SECTOR 1 Adresi
#define FLASH_SECTOR_2   0x08008000U			// FLASH SECTOR 2 Adresi
#define FLASH_SECTOR_3   0x0800C000U			// FLASH SECTOR 3 Adresi
#define FLASH_SECTOR_4   0x08010000U			// FLASH SECTOR 4 Adresi
#define FLASH_SECTOR_5   0x08020000U			// FLASH SECTOR 5 Adresi
#define FLASH_SECTOR_6   0x08040000U			// FLASH SECTOR 6 Adresi
#define FLASH_SECTOR_7   0x08060000U			// FLASH SECTOR 7 Adresi

#define FLASH_SECTOR_0_SIZE (16U * 1024U)       // FLASH SECTOR 0 Boyutu = 16 kb
#define FLASH_SECTOR_1_SIZE (16U * 1024U)		// FLASH SECTOR 1 Boyutu = 16 kb
#define FLASH_SECTOR_2_SIZE (16U * 1024U)		// FLASH SECTOR 2 Boyutu = 16 kb
#define FLASH_SECTOR_3_SIZE (16U * 1024U)		// FLASH SECTOR 3 Boyutu = 16 kb
#define FLASH_SECTOR_4_SIZE (64U * 1024U)		// FLASH SECTOR 4 Boyutu = 64 kb
#define FLASH_SECTOR_5_SIZE (128U * 1024U)		// FLASH SECTOR 5 Boyutu = 128 kb
#define FLASH_SECTOR_6_SIZE (128U * 1024U)		// FLASH SECTOR 6 Boyutu = 128 kb
#define FLASH_SECTOR_7_SIZE (128U * 1024U)		// FLASH SECTOR 7 Boyutu = 128 kb

#define FLASH_CR_PSIZE_x32 (2U << 8)			// FLASH Control Register PSIZE bit adres değeri.

#define CPU_CLOCK_HZ 16000000U					// STM32F407VET6 Sistem clock frekansı. (16MHz)

#endif /* BOOT_CONFIG_H_ */
