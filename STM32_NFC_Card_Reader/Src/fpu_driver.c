#include "fpu_driver.h"

// FPU başlangıç konfigürasyonları.
void FPU_Init()
{
	/*
	 * System Control Block (Sistem Kontrol Bloğu) içinde FPU(Floating Point Unit) aktif et.
	 * CPACR (Coprocessor Access Control Register) üzerinden CP10 ve CP11 alanlarına erişerek FPU kullanıma açılır.
	 * bit[23:20] 11 değerleri ile full access yapılarak FPU aktif edilir.
	 */
	SCB->CPACR |= (3U << 20) | (3U << 22);

	/*
	 * Önceki register yazma işlemi tamamlansın.
	 * CPACR'e register'a yazdığım FPU erişim ayarı tamamlanmandan devam etme.
	 */
	__DSB();

	/*
	 * Bundan sonraki komutları yeni sistem ayarlarına göre çalıştır.
	 * Yani FPU erişim açıldı. Bundan sonraki işlemlerde bunu dikkate alarak yap.
	 */
	__ISB();
}
