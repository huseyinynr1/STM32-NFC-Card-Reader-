# STM32 NFC Card Reader & Balance Management System

STM32F407 tabanlı bu proje; **RC522 RFID/NFC kart okuyucu**, **SIM800C GSM/GPRS modülü**, **ILI9341 TFT ekran**, **DS3231 RTC**, **FreeRTOS** ve **STM32 bootloader** mimarisi kullanılarak geliştirilen gömülü bir kart okuma, bakiye yönetimi ve firmware güncelleme sistemidir.

Sistem, toplu taşıma kart okuyucu mantığına benzer şekilde çalışır. Yeni kartları algılar, kart kişiselleştirme bilgilerini sunucudan alır, MIFARE kart bloklarına yazar, kayıtlı kartlarda bakiye/tarife kontrolü yapar, bekleyen yükleme isteklerini karta işler ve işlem sonuçlarını API servisine bildirir.

Bootloader yapısı sayesinde cihaz, SIM800C üzerinden sunucuda aktif olarak işaretlenen yeni firmware’i sorgulayabilir, parça parça indirebilir, STM32 FLASH ana uygulama alanına yazabilir ve CRC32 doğrulaması sonrası güvenli şekilde ana uygulamaya geçiş yapabilir.

Bu yapı ile proje; RFID/NFC kart yönetimi, GSM/GPRS tabanlı HTTP haberleşmesi, FreeRTOS görev mimarisi, MIFARE veri yönetimi, FLASH bellek işlemleri ve bootloader tabanlı firmware update süreçlerini bir araya getiren uçtan uca bir gömülü sistem uygulamasına dönüştürülmüştür.

> Bu README, projenin genel mimarisini ve çalışma mantığını anlatır. API servisi projede destekleyici backend olarak kullanılmıştır; ana odak STM32 gömülü yazılım tarafıdır.

---

## Genel Bakış

Bu proje, STM32F407 tabanlı bir NFC kart okuyucu, bakiye yönetimi ve firmware güncelleme sistemidir. RC522 ile MIFARE kart okuma/yazma işlemleri yapılır, SIM800C üzerinden GSM/GPRS tabanlı HTTP haberleşmesi gerçekleştirilir, DS3231 ile gerçek zaman bilgisi alınır ve ILI9341 TFT ekran üzerinden kullanıcıya işlem durumu gösterilir.

Bootloader mimarisi ile sistem, sunucuda aktif olan yeni firmware’i SIM800C üzerinden parça parça indirebilir, STM32 FLASH ana uygulama alanına yazabilir ve CRC32 doğrulaması sonrası güvenli şekilde çalıştırabilir.

Bootloader, firmware metadata bilgilerini ve ana uygulama bütünlüğünü kontrol ederek geçerli firmware’e güvenli geçiş sağlar. Böylece proje; kart yönetimi, GSM/GPRS haberleşmesi, FreeRTOS uygulama mimarisi ve firmware update süreçlerini tek bir gömülü sistem yapısında birleştirir.

Temel senaryolar:

- Bootloader üzerinden firmware güncelleme moduna girme
- Sunucudan aktif firmware bilgisini sorgulama
- Firmware dosyasını HTTP üzerinden parça parça indirme
- Yeni firmware’i STM32 FLASH ana uygulama alanına yazma
- Firmware boyutu ve CRC32 bütünlük kontrolü yapma
- Firmware metadata bilgisini FLASH üzerinde saklama
- Geçerli ana uygulamayı doğrulayıp bootloader’dan application’a güvenli geçiş yapma
- Yeni kart algılama
- Kart UID bilgisini API servisine gönderme
- API’den gelen kart kişiselleştirme bilgilerini MIFARE karta yazma
- Kayıtlı kartlarda bakiye ve tarife kontrolü yapma
- Kart için bekleyen bakiye yükleme isteğini sorgulama
- Bakiye yükleme sonucunu karta ve API’ye bildirme
- Kartta yapılan ücret çekim ve zamanlarını API’ye bildirme
- İşlem sonucunu TFT ekranda gösterme

---

## Kullanılan Teknolojiler

### Gömülü Yazılım

- STM32F407VET6 / STM32F4 serisi
- STM32CubeIDE
- C
- **Bare-metal embedded programming yaklaşımı**
- **Register-level peripheral initialization**
- **Custom low-level driver development**
- FreeRTOS task/queue/mutex mimarisi
- SPI, I2C, USART, DMA, Timer, GPIO
- GSM/GPRS tabanlı hücresel haberleşme
- SIM800C AT command yönetimi
- HTTP GET/POST haberleşmesi
- UART interrupt/DMA tabanlı modem cevap yönetimi
- Interrupt, timeout ve donanım durum bayrağı yönetimi
- Custom STM32 bootloader mimarisi
- FLASH bellek bölümleme ve uygulama alanı yönetimi
- Firmware metadata ve sürüm yönetimi
- CRC32 tabanlı firmware doğrulama
- Bootloader’dan ana uygulamaya güvenli geçiş
- Vector Table relocation ve MSP yapılandırması

### Donanım Modülleri

- RC522 RFID/NFC okuyucu
- MIFARE Classic kart
- SIM800C GSM/GPRS modülü
- ILI9341 TFT LCD
- DS3231 RTC modülü
- 12VDC/4VDC LDO

### Backend / Test Servisi

- C#
- ASP.NET Web API
- SQL Server
- Postman ile endpoint testleri
- Kart kişiselleştirme ve bakiye yükleme servisleri
- Firmware dosyası yükleme ve sürüm kayıt altyapısı
- Bootloader için aktif firmware sorgulama servisi
- Raw binary firmware chunk aktarımı
- Firmware güncelleme durumu yönetimi

---

## Bootloader ve Firmware Update Mimarisi

Projeye eklenen bootloader yapısı, STM32F407 üzerinde ana uygulamadan bağımsız olarak çalışan ayrı bir firmware katmanıdır. Bootloader’ın görevi; cihaz açılışında güncelleme isteğini kontrol etmek, yeni firmware mevcutsa bunu sunucudan almak, FLASH belleğe güvenli şekilde yazmak ve geçerli uygulamaya kontrollü geçiş yapmaktır.

Bu yapı sayesinde ana uygulama kodu güncellenebilir hale getirilmiş ve proje gerçek ürünlerde kullanılan firmware update mantığına yaklaştırılmıştır.

### FLASH Bellek Yerleşimi

STM32F407VET6 üzerindeki FLASH alanı bootloader, metadata ve ana uygulama olarak ayrılmıştır.

```text
0x08000000 ─────────────────────
             Bootloader Area
             Sector 0 - Sector 1
             32 KB
0x08008000 ─────────────────────
             Metadata Area
             Sector 2
             16 KB
0x0800C000 ─────────────────────
             Main Application Area
             Sector 3 - Sector 7
             464 KB
0x08080000 ─────────────────────
             FLASH End
```

### Bootloader Açılış Akışı

Cihaz açıldığında bootloader önce temel donanım hazırlıklarını yapar. Güncelleme butonu aktifse firmware update moduna girer. Güncelleme isteği yoksa mevcut ana uygulamanın geçerliliğini kontrol ederek application’a geçer.

```text
System Reset
    │
    ▼
GPIO + SysTick Init
    │
    ▼
Update Button Check
    │
    ├── Pressed
    │      │
    │      ▼
    │   SIM800C + GPRS Init
    │      │
    │      ▼
    │   Check Latest Firmware
    │      │
    │      ▼
    │   Download + Write Firmware
    │      │
    │      ▼
    │   Write Metadata
    │      │
    │      ▼
    │   System Reset
    │
    └── Not Pressed
           │
           ▼
        Validate Metadata
           │
           ▼
        Validate Application CRC
           │
           ▼
        Check Stack Pointer + Reset Handler
           │
           ▼
        Jump to Main Application
```

### Firmware Güncelleme Akışı

Bootloader, yeni firmware bilgisini backend servisinden alır. Gelen cevap içinde firmware ID, version, dosya boyutu ve CRC32 bilgisi bulunur. Firmware aktif olarak işaretlenmişse güncelleme süreci başlar.

Güncelleme sırasında önce metadata alanı ve ana uygulama FLASH sektörleri silinir. Daha sonra firmware dosyası belirlenen maksimum parça boyutuna göre indirilir. Her parça HTTP GET isteğiyle raw binary formatta alınır ve FLASH bellekte ana uygulama başlangıç adresinden itibaren yazılır.

```text
GET /api/firmware/latest
        │
        ▼
Firmware metadata received
        │
        ▼
Erase metadata sector
        │
        ▼
Erase main application sectors
        │
        ▼
GET /api/firmware/chunk/raw
        │
        ▼
Write chunk to FLASH
        │
        ▼
Repeat until full firmware is written
        │
        ▼
Calculate FLASH CRC32
        │
        ▼
Compare with server CRC32
        │
        ▼
Write firmware metadata
        │
        ▼
POST /api/firmware/update-active
        │
        ▼
System reset
```

## Metadata Yapısı

Bootloader, yüklenen firmware’in geçerli olup olmadığını anlamak için FLASH üzerinde ayrı bir metadata alanı kullanır.

```c
typedef struct
{
    uint32_t metadata_signature;
    uint32_t firmware_version;
    uint32_t firmware_size;
    uint32_t firmware_crc32;
    uint32_t valid_flag;
    uint32_t metadata_crc32;
} Firmware_Metadata_Typedef;
```

Metadata içinde firmware sürümü, firmware boyutu, firmware CRC32 değeri, geçerlilik bayrağı ve metadata’nın kendi CRC32 değeri tutulur.

Bu yapı sayesinde bootloader her açılışta şu kontrolleri yapabilir:

- Metadata imza değeri doğru mu?
- Firmware valid flag değeri geçerli mi?
- Firmware boyutu ana uygulama alanı sınırları içinde mi?
- Metadata CRC32 değeri doğru mu?
- FLASH üzerindeki uygulamanın CRC32 değeri metadata ile eşleşiyor mu?

### Ana Uygulamaya Güvenli Geçiş

Bootloader, ana uygulamaya geçmeden önce application başlangıç adresindeki stack pointer ve reset handler değerlerini kontrol eder. Stack pointer değerinin SRAM adres aralığında, reset handler adresinin ise ana uygulama FLASH alanı içinde olması gerekir.

Kontroller başarılıysa bootloader interrupt’ları kapatır, SysTick’i durdurur, vector table adresini ana uygulama başlangıç adresine yönlendirir, MSP değerini ana uygulamanın stack pointer değeriyle günceller ve reset handler üzerinden ana firmware’i çalıştırır.

```text
Validate Application
        │
        ▼
Disable Interrupts
        │
        ▼
Stop SysTick
        │
        ▼
SCB->VTOR = MAIN_APP_START_ADDRESS
        │
        ▼
Set MSP
        │
        ▼
Jump to Application Reset Handler
```

## Bare-Metal ve FreeRTOS Yaklaşımı

Bu projede düşük seviye donanım altyapısı, STM32 çevresel birimlerinin doğrudan register seviyesinde yapılandırılmasıyla geliştirilmiştir. GPIO, SPI, I2C, USART, DMA, Timer ve interrupt ayarları sadece hazır örnek kodlara bağlı kalmadan, STM32F4 çevresel birimlerinin çalışma mantığı dikkate alınarak hazırlanmıştır.

FreeRTOS, bu düşük seviye bare-metal altyapı üzerine entegre edilmiştir. Böylece donanım sürücüleri register-level yapıda korunurken, uygulama seviyesi RFID okuma, SIM800C HTTP haberleşmesi, RTC zaman yönetimi ve TFT ekran güncelleme görevleri ayrı task yapılarıyla yönetilmiştir.

```text
Register-Level Peripheral Init
        │
        ▼
Custom Bare-Metal Drivers
        │
        ▼
FreeRTOS Task / Queue Architecture
        │
        ▼
RFID + GSM + RTC + TFT Application Flow
```

---

## Temel Özellikler

### STM32 Bootloader

- STM32 üzerinde bootloader mimarisi
- FLASH bellek üzerinde bootloader, metadata ve ana uygulama alan ayrımı
- Manuel firmware update modu
- SIM800C ile aktif firmware sorgulama
- HTTP üzerinden raw binary firmware chunk indirme
- Ana uygulama FLASH alanına firmware yazma
- FLASH sektör silme ve yazma kontrolü
- CRC32 ile firmware bütünlük doğrulaması
- FLASH üzerinde firmware metadata yönetimi
- Metadata signature, valid flag ve metadata CRC kontrolü
- Ana uygulama stack pointer ve reset handler doğrulaması
- Vector table relocation ve MSP güncellemesi
- Bootloader’dan ana uygulamaya güvenli geçiş
- Güncelleme sonrası firmware aktiflik durumunun API’ye bildirilmesi

### STM32 Ana Uygulama Firmware

- STM32 çevresel birimlerinin register seviyesinde başlatılması ve kontrol edilmesi
- Bare-metal seviyede GPIO, SPI, I2C, USART, DMA, Timer ve interrupt yapılandırmaları
- FreeRTOS öncesi düşük seviye sürücü altyapısının oluşturulması
- FreeRTOS sonrası task/queue/mutex tabanlı uygulama mimarisine geçilmesi
- RC522 ile kart algılama, UID okuma ve MIFARE blok okuma/yazma
- MIFARE kart üzerinde proje kimliği için `Magic Number` kontrolü
- Kart durum sınıflandırması:
  - Yeni kart
  - Sisteme kayıtlı kart
  - Geçersiz/yabancı kart
- Kart kişiselleştirme işlemi
- Kart bakiyesi, maksimum bakiye, vize tarihi ve işlem sayaçlarının kart bloklarında tutulması
- CRC16 ile kart verisi bütünlük kontrolü
- SIM800C GSM/GPRS modülü ile hücresel veri bağlantısı kurulması
- AT komutlarıyla SIM durumu, şebeke kaydı, GPRS bearer ve HTTP servis yönetimi
- Uzak API servisine HTTP GET/POST istekleri gönderilmesi
- HTTP cevaplarının UART interrupt/DMA tabanlı alınması ve timeout/paket mantığıyla işlenmesi
- API’den gelen JSON cevaplarının manuel parse edilmesi
- FreeRTOS task/queue mimarisi
- TFT ekranda saat, tarih, bakiye, yüklenen tutar ve işlem sonucu gösterimi
- RTC DS3231 ile gerçek zamanlı tarih/saat takibi
- SPI ortak kullanımı için mutex yaklaşımı

### API Servisi

API servisi, STM32 cihazının demo ve geliştirme sürecinde haberleştiği destekleyici backend olarak kullanılmıştır. Bu servis üzerinden kart kişiselleştirme bilgileri sorgulanır, bakiye yükleme istekleri oluşturulur, işlem durumları güncellenir ve bootloader için firmware dağıtım süreci yönetilir. Firmware update tarafında `.bin` dosyası servise yüklenir, dosya boyutu ve CRC32 bilgisi hesaplanarak kaydedilir; bootloader ise aktif firmware bilgisini sorgulayarak ilgili dosyayı raw binary parçalar halinde indirir.

---

## Sistem Mimarisi

```text
┌─────────────────────┐
│     MIFARE Card      │
└──────────┬──────────┘
           │ RF
           ▼
┌─────────────────────┐       SPI        ┌─────────────────────┐
│     RC522 RFID       │◀──────────────▶│      STM32F407      │
└─────────────────────┘                  │                     │
                                         │  FreeRTOS Tasks     │
┌─────────────────────┐       I2C        │  - RFID Task        │
│     DS3231 RTC       │◀──────────────▶│  - SIM800C Task     │
└─────────────────────┘                  │  - Timer/RTC Task   │
                                         │  - TFT LCD Task     │
┌─────────────────────┐       SPI        │                     │
│    ILI9341 TFT       │◀──────────────▶│                     │
└─────────────────────┘                  └──────────┬──────────┘
                                                    │ USART + AT Commands
                                                    ▼
                                         ┌─────────────────────┐
                                         │      SIM800C        │
                                         │     GPRS / HTTP     │
                                         └──────────┬──────────┘
                                                    │ HTTP GET / POST
                                                    ▼
                                         ┌─────────────────────┐
                                         │  ASP.NET Web API    │
                                         │  SQL Server Backend │
                                         └─────────────────────┘
```

---

## FreeRTOS Görev Yapısı

Projede işlemler birbirinden ayrılmış task yapılarıyla yönetilir.

| Task | Görev |
|---|---|
| `vRFID_TASK` | Kart algılama, UID okuma, kart tipi belirleme, MIFARE okuma/yazma, bakiye ve işlem kontrolü |
| `vSIM800C_Task` | HTTP GET/POST işlemleri, API haberleşmesi, JSON cevaplarının işlenmesi |
| `vTimer_Service_Task` | DS3231 RTC’den tarih/saat okuma ve ilgili kuyruklara aktarma |
| `vTFT_LCD_Task` | TFT ana ekranı, saat/tarih güncelleme ve kart işlem sonucunu ekranda gösterme |

Task’lar arasında veri aktarımı için FreeRTOS queue yapıları kullanılır:

```text
RFID Task  ── card UID / request type ──▶ SIM800C Task
RFID Task  ◀─ new card info / topup info ─ SIM800C Task
RFID Task  ── card result info ─────────▶ TFT Task
RTC Task   ── date/time ────────────────▶ RFID Task + TFT Task
```

---

## Kart Hafıza Yapısı

Kart bilgileri MIFARE Classic kartın belirli bloklarında tutulur.

### Sector 1 / Block 0 — Card Header

```c
typedef struct PACKED_STRUCT {
    uint8_t magic_number[2];
    uint8_t version;
    RC522_Card_Type card_type;
    uint8_t uid[4];
    uint32_t operation_counter;
    uint16_t expiry_date;
    uint16_t crc;
} RC522_Card_Header;
```

Bu blokta kartın projeye ait olup olmadığı, kart tipi, UID, işlem sayacı, son kullanma tarihi ve CRC bilgisi tutulur.

### Sector 1 / Block 1 — Balance Info

```c
typedef struct PACKED_STRUCT {
    uint32_t balance;
    uint32_t operation_counter;
    uint32_t max_balance;
    uint16_t visa_date;
    uint16_t crc;
} RC522_Card_Balance;
```

Bu blokta kart bakiyesi, bakiye işlem sayacı, maksimum bakiye limiti, vize tarihi ve CRC bilgisi tutulur.

### Sector 1 / Block 3 — Security Trailer

```c
typedef struct PACKED_STRUCT {
    uint8_t key_A[6];
    uint8_t access_bits[3];
    uint8_t general_purpose;
    uint8_t key_B[6];
} RC522_Card_Security;
```

Bu blokta MIFARE sektör güvenlik bilgileri tutulur.

---

## Çalışma Senaryosu

### 1. Firmware Update / Bootloader Senaryosu

Firmware güncelleme işlemi bootloader seviyesinde gerçekleştirilir. Cihaz açılışında güncelleme butonuna basılıysa sistem ana uygulamaya geçmeden bootloader update moduna girer.

1. GPIO ve SysTick başlatılır.
2. Güncelleme butonu kontrol edilir.
3. Buton aktifse USART3 başlatılır.
4. SIM800C modülü başlatılır.
5. GPRS bağlantısı kurulur.
6. Aktif firmware bilgisi API servisinden sorgulanır.
7. Firmware ID, version, dosya boyutu ve CRC32 bilgisi alınır.
8. Metadata alanı temizlenir.
9. Ana uygulama FLASH sektörleri silinir.
10. Firmware dosyası raw binary chunk yapısıyla parça parça indirilir.
11. Her chunk ana uygulama FLASH alanına yazılır.
12. Toplam yazılan firmware boyutu kontrol edilir.
13. FLASH üzerindeki firmware için CRC32 hesaplanır.
14. Hesaplanan CRC32, API’den gelen CRC32 değeriyle karşılaştırılır.
15. Firmware geçerliyse metadata alanı güncellenir.
16. Firmware’in artık aktif olmadığını bildirmek için API’ye POST isteği gönderilir.
17. Sistem resetlenir.
18. Bootloader yeniden açıldığında metadata ve application doğrulaması yaparak ana uygulamaya geçer.

```text
Bootloader Mode
      │
      ▼
GET Latest Firmware Info
      │
      ▼
Erase Application Area
      │
      ▼
Download Firmware Chunks
      │
      ▼
Write Chunks to FLASH
      │
      ▼
Validate Firmware CRC32
      │
      ▼
Write Metadata
      │
      ▼
Reset MCU
      │
      ▼
Validate and Jump to Main Application
```

### 2. Main Application / Sistem Başlatma Senaryosu

Ana uygulamaya geçildiğinde STM32 çevresel birimleri ve harici donanımlar başlatılır:

1. FPU
2. Sistem clock
3. TIM6 zamanlayıcı
4. GPIO
5. I2C
6. SPI
7. USART + DMA
8. RC522
9. SIM800C + GPRS
10. DS3231 RTC
11. ILI9341 TFT
12. FreeRTOS scheduler

Tüm donanımlar hazır olduğunda FreeRTOS task’ları çalışmaya başlar.

---

### 3. Yeni Kart Kişiselleştirme

Yeni veya boş bir kart okutulduğunda sistem şu adımları izler:

1. RC522 kartı algılar.
2. UID bilgisi okunur.
3. Kartın ilgili bloğu okunur.
4. Magic Number kontrolü yapılır.
5. Kart boş ise `New_Card` olarak değerlendirilir.
6. Kart UID bilgisi SIM800C task’a gönderilir.
7. SIM800C, API servisine GET isteği gönderir.
8. API’den gelen kart bilgileri parse edilir.
9. Kart bilgileri MIFARE bloklarına yazılır.
10. İşlem sonucu API servisine POST edilir.
11. Sonuç TFT ekranda gösterilir.

```text
Card UID Read
      │
      ▼
GET /personalization/getbyuid?cardUid=...
      │
      ▼
Write Card Header + Balance Blocks
      │
      ▼
POST /personalization/update-status
      │
      ▼
Show Result on TFT
```

---

### 4. Kayıtlı Kart Okuma, Bakiye Yükleme ve Ücret Çekme

Kayıtlı bir kart okutulduğunda sistem önce kartın projeye ait olup olmadığını ve veri bütünlüğünü kontrol eder. Bunun için kart üzerindeki `Magic Number`, kart tipi, CRC, bakiye, maksimum bakiye limiti, vize tarihi ve işlem sayaçları değerlendirilir.

Kart geçerli ise sistem iki ana akış üzerinden ilerler. İlk olarak ilgili kart için API tarafında bekleyen bir bakiye yükleme isteği olup olmadığı sorgulanır. Bekleyen yükleme isteği varsa, tutar kart bakiyesine eklenir, kart üzerindeki bakiye bloğu güncellenir ve yükleme işleminin sonucu API servisine bildirilir.

Bekleyen yükleme isteği yoksa sistem kart tipine göre normal kullanım akışına geçer. Kart tipi ve mevcut bakiye bilgisine göre karttan belirlenen ücret düşülür. İşlem başarılı olursa yeni bakiye karta yazılır, işlem sayacı güncellenir ve ücret çekme işlemi; kart UID bilgisi, çekilen tutar, kalan bakiye ve RTC’den alınan işlem zamanı ile birlikte API servisine bildirilir. Yetersiz bakiye, geçersiz kart, vize süresi dolmuş kart veya CRC hatası gibi durumlarda işlem yapılmaz ve sonuç TFT ekranda kullanıcıya gösterilir.

```text
Registered Card Read
      │
      ▼
Read Header + Balance Blocks
      │
      ▼
Magic Number + CRC + Expiry/Visa + Balance Checks
      │
      ▼
GET /topup/getbyuid?cardUid=...
      │
      ├── Topup exists
      │       │
      │       ▼
      │   Add amount to card balance
      │       │
      │       ▼
      │   POST topup result to API
      │
      └── No topup
              │
              ▼
          Calculate fare by card type
              │
              ▼
          Deduct balance from card
              │
              ▼
          Send usage/transaction result to API
              │
              ▼
          Show Result on TFT
```

---

## SIM800C GSM/GPRS ve HTTP Haberleşmesi

SIM800C modülü, projede STM32 cihazının GSM/GPRS ağı üzerinden uzak API servisiyle haberleşmesini sağlar. STM32, USART hattı üzerinden SIM800C’ye AT komutları gönderir; SIM kart, şebeke kaydı, GPRS bağlantısı, HTTP servis başlatma, GET/POST işlemleri ve sunucu cevaplarının okunması bu komut akışıyla yönetilir.

1. Modül başlatılır.
2. SIM ve şebeke durumu kontrol edilir.
3. GPRS bağlantısı açılır.
4. HTTP servisi başlatılır.
5. URL parametresi ayarlanır.
6. GET veya POST işlemi başlatılır.
7. `+HTTPACTION` sonucu beklenir.
8. `AT+HTTPREAD` ile cevap okunur.
9. Gelen JSON veri STM32 tarafında parse edilir.

Başarılı HTTP sonuçları için sistem özellikle şu cevapları kontrol eder:

```c
#define ANS_0_200 "0,200"   // HTTP GET success
#define ANS_1_200 "1,200"   // HTTP POST success
```

---

## API Endpoint Özeti

API servisi, kart işlemleri ve firmware update akışı için kullanılan destekleyici backend katmanıdır.

| İşlem | Method | Endpoint |
|---|---|---|
| Yeni kart bilgisi sorgulama | GET | `/api/personalization/getbyuid?cardUid=...` |
| Yeni kart işlem sonucunu güncelleme | POST | `/api/personalization/update-status` |
| Bakiye yükleme isteği sorgulama | GET | `/api/topup/getbyuid?cardUid=...` |
| Bakiye yükleme sonucunu güncelleme | POST | `/api/topup/update-status` |
| Test amaçlı kart kaydı oluşturma | POST | `/api/cards/add` |
| Test amaçlı bakiye yükleme isteği oluşturma | POST | `/api/topup/add` |
| Aktif firmware bilgisi sorgulama | GET | `/api/firmware/latest` |
| Firmware dosyasını raw binary chunk olarak alma | GET | `/api/firmware/chunk/raw?firmwareId=...&offset=...&size=...` |
| Firmware aktiflik durumunu güncelleme | POST | `/api/firmware/update-active` |
| Test amaçlı firmware dosyası yükleme | POST | `/api/firmware/upload` |

---

## Postman Testleri

Postman ile API servisinin temel endpoint testleri yapılmıştır. Aşağıdaki görsellerde kart kişiselleştirme isteği oluşturma, UID ile kart bilgisi sorgulama, bakiye yükleme isteği oluşturma ve UID ile bekleyen bakiye yükleme isteğini sorgulama akışları gösterilmektedir.

### Kart Kişiselleştirme Testleri

![Create Card Personalization Request](docs/postman-personalization-add.png)

![Get Card Personalization By UID](docs/postman-personalization-getbyuid.png)

### Bakiye Yükleme Testleri

![Create Topup Request](docs/postman-topup-add.png)

![Get Topup Request By UID](docs/postman-topup-getbyuid.png)

Örnek kart oluşturma isteği:

```json
{
  "cardUid": "AABBCCDD",
  "magicNumber": 18521,
  "version": 1,
  "cardType": 0,
  "expiryDate": "01/01/2027",
  "visaDate": "01/01/2027",
  "currentBalanceKurus": 0,
  "processOperationCounter": 0,
  "balanceOperationCounter": 0
}
```

Örnek bakiye yükleme isteği:

```json
{
  "requestId": 17,
  "cardUid": "AABBCCDD",
  "amountKurus": 5000,
  "status": 0
}
```

---

## Demo Video

https://github.com/user-attachments/assets/29436a0f-e0b2-4e96-8eec-04d78dfcce11

Video; sistemin açılışını, kart kişiselleştirme işlemini, API üzerinden bakiye yükleme isteği oluşturulmasını, kart okutulduğunda bakiyenin karta yazılmasını, kart tipine göre ücret düşülmesini ve TFT ekrandaki işlem sonuçlarını göstermektedir.

---

## Ekran Görüntüleri

### Donanım ve TFT Çıktıları

![Circuit Diagram](docs/circuit-diagram.png)

![Insufficient Balance Screen](docs/tft-insufficient-balance.jpg)

![Success Screen](docs/success-screen.jpg)

---

## Proje Dosya Yapısı

Repository yapısı:

```text
├── STM32_NFC_Card_Reader_Bootloader/
│   ├── Inc/
│   │   ├── app_control.h
│   │   ├── boot_config.h
│   │   ├── bootloader_driver.h
│   │   ├── crc32.h
│   │   ├── firmware_update_helper.h
│   │   ├── flash_driver.h
│   │   ├── gpio_driver.h
│   │   ├── led_blink.h
│   │   ├── metadata.h
│   │   ├── sim800c.h
│   │   ├── systick_driver.h
│   │   └── uart_driver.h
│   ├── Src/
│   │   ├── app_control.c
│   │   ├── bootloader_driver.c
│   │   ├── crc32.c
│   │   ├── firmware_update_helper.c
│   │   ├── flash_driver.c
│   │   ├── gpio_driver.c
│   │   ├── led_blink.c
│   │   ├── main.c
│   │   ├── metadata.c
│   │   ├── sim800c.c
│   │   ├── systick_driver.c
│   │   └── uart_driver.c
│   ├── Startup/
│   ├── Drivers/
│   ├── STM32F407VETX_FLASH.ld
│   └── STM32F407VETX_RAM.ld
│
├── STM32_NFC_Card_Reader/
│   ├── Inc/
│   │   ├── app_tasks.h
│   │   ├── rc522.h
│   │   ├── sim800c.h
│   │   ├── tft_ili9341.h
│   │   ├── rtc_ds3231.h
│   │   ├── spi_driver.h
│   │   ├── i2c_driver.h
│   │   ├── usart_driver.h
│   │   └── helper_function.h
│   ├── Src/
│   │   ├── main.c
│   │   ├── app_tasks.c
│   │   ├── rc522.c
│   │   ├── sim800c.c
│   │   ├── tft_ili9341.c
│   │   ├── rtc_ds3231.c
│   │   ├── spi_driver.c
│   │   ├── i2c_driver.c
│   │   ├── usart_driver.c
│   │   └── helper_function.c
│   ├── Drivers/
│   ├── MiddleWares/
│   ├── Startup/
│   ├── STM32F407VETX_FLASH.ld
│   └── STM32F407VETX_RAM.ld
│
├── CardControlService/
│   ├── Controllers/
│   ├── Models/
│   ├── Repositories/
│   └── CardControlService.csproj
│
├── docs/
│   ├── demo.mp4
│   ├── circuit-diagram.jpg
│   ├── tft-main-screen.jpg
│   └── postman-images...
│
├── README.md
└── .gitignore
```

---

## Bu Projede Öne Çıkan Teknik Noktalar

- STM32 üzerinde ana uygulamadan bağımsız bootloader mimarisi
- Bootloader, metadata ve application alanları için FLASH bellek planlaması
- SIM800C üzerinden HTTP tabanlı firmware sorgulama ve indirme akışı
- Raw binary chunk yapısıyla parça parça firmware aktarımı
- FLASH sektör silme, programlama ve yazım sonrası kontrol mekanizması
- CRC32 ile firmware bütünlük ve doğruluk kontrolü
- Metadata signature, valid flag, firmware size ve CRC doğrulama yapısı
- Application stack pointer ve reset handler adres geçerlilik kontrolü
- Vector table relocation ve MSP güncellemesi ile güvenli application jump
- STM32 çevresel birimlerinde register-level bare-metal driver geliştirme
- FreeRTOS ile task ayrımı, queue haberleşmesi ve mutex tabanlı kaynak yönetimi
- SPI hattının RC522 ve TFT arasında kontrollü şekilde paylaşılması
- SIM800C ile AT komut tabanlı GPRS ve HTTP haberleşmesi
- AT komut cevaplarında timeout, durum bayrağı ve paket bazlı cevap yönetimi
- MIFARE Classic kart blokları için özel veri yapısı ve kart hafıza düzeni
- Magic Number ve CRC16 ile kart verisi doğrulama mekanizması
- RTC tabanlı tarih/saat yönetimi ve işlem zamanı takibi
- TFT ekran üzerinden gerçek zamanlı kullanıcı bilgilendirme
- Kart tipine göre ücretlendirme, bakiye güncelleme ve işlem sonucu bildirimi
- STM32 firmware ile CardControlService arasında uçtan uca sistem entegrasyonu

---

## Geliştirici Notu

Bu proje, gömülü yazılım tarafında gerçek bir ürün akışını simüle etmek amacıyla geliştirilmiştir. Kart kişiselleştirme, bakiye yönetimi, GSM/GPRS üzerinden API haberleşmesi, TFT kullanıcı arayüzü ve FreeRTOS task mimarisini bir araya getiren uçtan uca bir gömülü sistem çalışmasıdır.

Bootloader katmanı ile proje; firmware güncelleme, FLASH bellek yönetimi, metadata tabanlı doğrulama, CRC32 bütünlük kontrolü ve bootloader’dan ana uygulamaya güvenli geçiş gibi gerçek ürün geliştirme süreçlerinde kullanılan konuları da kapsayacak şekilde genişletilmiştir.

CardControlService, bu mimaride kart kişiselleştirme, bakiye yükleme, işlem durumu takibi ve firmware dağıtım süreçlerini yöneten backend servis olarak konumlandırılmıştır. STM32 firmware’i ile haberleşerek hem kart operasyonlarını hem de bootloader’ın firmware update akışını destekler.

---

## Yazar

**Hüseyin Yanar**