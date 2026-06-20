DONATE: bc1qps62cyk9f9unmdkc9k3ccj9e2h8ywfhg2j53ec

Built with ❤️ for the crypto research community.

## 🔧 FAST BIP32 SCANNER v15 / БЫСТРЫЙ СКАНЕР BIP32 v15 / ШВИДКИЙ СКАНЕР BIP32 v15
(PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp)
### Opis / Description / Описание:

Multi-coin private key scanner supporting **6 cryptocurrencies** with **compressed + uncompressed** address formats.  
Мультивалютный сканер приватных ключей поддерживающий **6 криптовалют** с **сжатыми + несжатыми** форматами адресов.  
Мультивалютний сканер приватних ключів підтримує **6 криптовалют** з **стиснутими + нестиснутими** форматами адрес.

**Supported coins / Поддерживаемые монеты / Підтримувані монети:**

| Coin / Монета | P2PKH (compressed) | P2PKH (uncompressed) | P2SH | SegWit |
|---------------|-------------------|----------------------|------|--------|
| Bitcoin | `1...` | `1...` | `3...` | `bc1...` |
| Litecoin | `L...` | `L...` | `M...` | `ltc1...` |
| Dogecoin | `D...` | `D...` | - | - |
| Dash | `X...` | `X...` | - | - |
| Zcash | `t1...` | - | `t3...` | - |
| Ethereum | - | - | - | `0x...` |

### Key features / Ключевые особенности / Ключові особливості:

- ✅ **60 threads** for maximum speed / 60 потоков для максимальной скорости / 60 потоків для максимальної швидкості
- ✅ **200,000 queue size** / Размер очереди / Розмір черги
- ✅ **MMap** for fast binary file access / MMap для быстрого доступа к бинарным файлам / MMap для швидкого доступу до бінарних файлів
- ✅ **Binary search** in address database / Бинарный поиск в базе адресов / Бінарний пошук в базі адрес
- ✅ **Both compressed and uncompressed** address checking / Проверка сжатых и несжатых адресов / Перевірка стиснутих та нестиснутих адрес
- ✅ **Built-in test** for private key = 1 / Встроенный тест для приватного ключа = 1 / Вбудований тест для приватного ключа = 1

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -std=c++17 -O3 -march=native -mtune=native -funroll-loops -pthread scan.cpp -lssl -lcrypto -lsecp256k1 -o scan

# Scan private keys from file / Сканировать приватные ключи из файла / Сканувати приватні ключі з файлу
./scan adresy.bin keys.txt


╔══════════════════════════════════════════════════════════════╗
║     🔑 TEST DLA PRIVATE KEY = 1 (KOMPRESOWANE + NIEKOMPRESOWANE) ║
╚══════════════════════════════════════════════════════════════╝

Private Key (hex): 0000000000000000000000000000000000000000000000000000000000000001

💰 Bitcoin P2PKH (1...) [COMPRESSED]: 1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
  Oczekiwany: 1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
  ✅ OK

💰 Bitcoin P2PKH (1...) [UNCOMPRESSED]: 1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm
  Oczekiwany: 1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm
  ✅ OK

💰 Bitcoin P2SH (3...): 3JvL6Ymt8MVWiCNHC7oWU6nLeHNJKLZGLN ✅
💰 Bitcoin SegWit (bc1...): bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4 ✅
💰 Litecoin P2PKH (L...) [COMPRESSED]: LVuDpNCSSj6pQ7t9Pv6d6sUkLKoqDEVUnJ ✅
💰 Litecoin P2PKH (L...) [UNCOMPRESSED]: LYWKqJhtPeGyBAw7WC8R3F7ovxtzAiubdM ✅
💰 Litecoin P2SH (M...): MR8UQSBr5ULwWheBHznrHk2jxyxkHQu8vB ✅
💰 Litecoin SegWit (ltc1...): ltc1qw508d6qejxtdg4y5r3zarvary0c5xw7kgmn4n9 ✅
💰 Dogecoin P2PKH (D...) [COMPRESSED]: DFpN6QqFfUm3gKNaxN6tNcab1FArL9cZLE ✅
💰 Dogecoin P2PKH (D...) [UNCOMPRESSED]: DJRU7MLhcPwCTNRZ4e8gJzDebtG1H5M7pc ✅
💰 Dash P2PKH (X...) [COMPRESSED]: XmN7PQYWKn5MJFna5fRYgP6mxT2F7xpekE ✅
💰 Dash P2PKH (X...) [UNCOMPRESSED]: XoyDQM3xGhFW5JqYBwTLckjqZ67Q3jZfAL ✅
💰 Zcash P2PKH (t1...): t1UYsZVJkLPeMjxEtACvSxfWuNmddpWfxzs ✅
💰 Zcash P2SH (t3...): t3bnw6tC26gH7JqRB8YcdbutFtwZP4Xp5o5 ✅
💰 Ethereum (0x...): 0x7e5f4552091a69125d5dfcb7b8c2659029395bdf ✅

╔══════════════════════════════════════════════════════════════╗
║        ✅ KONIEC TESTU                                     ║
╚══════════════════════════════════════════════════════════════╝

📁 Wczytano 602776102 adresów z adresy.bin
🧵 Uruchamiam 60 wątków

🔍 Obsługiwane monety:
   - Bitcoin
   - Litecoin
   - Dogecoin
   - Dash
   - Ethereum
   - Zcash

🚀 Rozpoczynam skanowanie...
⚡ 0.35 Mkeys/s | seeds: 472085 | keys: 1888340 | found: 0

✅ Skanowanie zakończone!
   Przetworzono: 472085 kluczy
   Wygenerowano: 1888340 adresów
   Znaleziono: 0 trafień
---
<img width="1097" height="588" alt="image" src="https://github.com/user-attachments/assets/7a1a38c3-7dc0-481d-9c16-a1a50cf58920" />


## 🔧 FAST BIP32 SCANNER v15 - OPTIMAL / БЫСТРЫЙ СКАНЕР BIP32 v15 - ОПТИМАЛЬНЫЙ / ШВИДКИЙ СКАНЕР BIP32 v15 - ОПТИМАЛЬНИЙ
(SPRAWDZPRIVKEY.cpp)
### Opis / Description / Описание:

Optimized BIP32 scanner for **private keys/seed phrases** with **selected paths** (direct, m/0-1, BIP44).  
Оптимизированный сканер BIP32 для **приватных ключей/сид-фраз** с **выбранными путями** (direct, m/0-1, BIP44).  
Оптимізований сканер BIP32 для **приватних ключів/сид-фраз** з **вибраними шляхами** (direct, m/0-1, BIP44).

### Key features / Ключевые особенности / Ключові особливості:

- ✅ **30 threads** for parallel scanning / 30 потоков для параллельного сканирования / 30 потоків для паралельного сканування
- ✅ **100,000 queue size** / Размер очереди / Розмір черги
- ✅ **MMap** for fast binary file access / MMap для быстрого доступа к бинарным файлам / MMap для швидкого доступу до бінарних файлів
- ✅ **Binary search** in address database / Бинарный поиск в базе адресов / Бінарний пошук в базі адрес
- ✅ **6 keys per seed** / 6 ключей на сид / 6 ключів на сід
- ✅ **Compressed + Uncompressed** address formats / Сжатые + Несжатые форматы адресов / Стиснуті + Нестиснуті формати адрес

### Scanned paths / Сканируемые пути / Скановані шляхи:

| # | Path / Путь | Description / Описание |
|---|-------------|----------------------|
| 1 | `direct_compressed` | Seed as compressed private key / Сид как сжатый приватный ключ / Сід як стиснутий приватний ключ |
| 2 | `direct_uncompressed` | Seed as uncompressed private key / Сид как несжатый приватный ключ / Сід як нестиснутий приватний ключ |
| 3 | `m/0-1` | BIP32 child paths 0 and 1 / BIP32 дочерние пути 0 и 1 / BIP32 дочірні шляхи 0 та 1 |
| 4 | `m/44'/0'/0'/0/0-1` | BIP44 standard paths 0 and 1 / BIP44 стандартные пути 0 и 1 / BIP44 стандартні шляхи 0 та 1 |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -std=c++17 -O3 -march=native -mtune=native -pthread fastbip32.cpp -lssl -lcrypto -lsecp256k1 -o fastbip32

# Run scanner / Запустить сканер / Запустити сканер
./fastbip32 adresy.bin seeds.txt

📁 Wczytano 602776102 adresów z adresy.bin
🧵 Uruchamiam 30 wątków konsumenckich
📦 Kolejka: 100000 seedów

🔍 Szukane ścieżki:
   - direct (seed as key) compressed/uncompressed
   - m/0-1
   - BIP44: m/44'/0'/0'/0/0-1

🚀 Rozpoczynam skanowanie...

⚡ 0.45 Mkeys/s | seeds: 472085 | keys: 1888340 | found: 0

✅ Skanowanie zakończone!
   Przetworzono: 472085 seedów
   Wygenerowano: 1888340 kluczy
   Znaleziono: 0 adresów
   Czas: 4.20 s

⚠️ LEGAL DISCLAIMER

FOR EDUCATIONAL AND SECURITY RESEARCH PURPOSES ONLY

• This software is provided for authorized testing only
• Do not use to access wallets you do not own
• Unauthorized access to cryptocurrency wallets may be illegal
• The author assumes no responsibility for misuse
• Users are responsible for complying with local laws
## 🔧 SORTER / СОРТИРОВЩИК / СОРТУВАЛЬНИК

### Opis / Description / Описание:

External sorting tool for 20-byte binary address databases (hash160).  
Инструмент внешней сортировки для 20-байтовых бинарных баз адресов (hash160).  
Інструмент зовнішнього сортування для 20-байтових бінарних баз адрес (hash160).

### Key features / Ключевые особенности / Ключові особливості:

- ✅ **External sorting** for large files > RAM / Внешняя сортировка для больших файлов / Зовнішнє сортування для великих файлів
- ✅ **Chunk-based** processing (200 MB chunks) / Обработка чанками (200 MB) / Обробка чанками (200 MB)
- ✅ **Multi-threaded** sorting (8 threads) / Многопоточная сортировка (8 потоков) / Багатопотокове сортування (8 потоків)
- ✅ **MMap** for fast file access / MMap для быстрого доступа к файлам / MMap для швидкого доступу до файлів
- ✅ **Merge sort** for final assembly / Сортировка слиянием для финальной сборки / Сортування злиттям для фінальної збірки
- ✅ **Progress tracking** every 50 MB / Отслеживание прогресса каждые 50 MB / Відстеження прогресу кожні 50 MB

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -std=c++17 -O3 -march=native -pthread sorter.cpp -o sorter

# Run sorter / Запустить сортировщик / Запустити сортувальник
./sorter adresy.bin adresy_sorted.bin



Input: adresy.bin (unsorted)
         ↓
    ┌────┴────┐
    │ CHUNK 1 │ 200 MB -> sort -> chunk_0.bin
    │ CHUNK 2 │ 200 MB -> sort -> chunk_200.bin
    │ CHUNK 3 │ 200 MB -> sort -> chunk_400.bin
    └────┬────┘
         ↓
    [MERGE SORT]
         ↓
Output: adresy_sorted.bin (sorted)


## 🔧 ADRESSTOBIN / КОНВЕРТЕР АДРЕСОВ В BIN / КОНВЕРТЕР АДРЕС В BIN

### Opis / Description / Описание:

Converts address lists to binary format (20 bytes per address) for fast scanning.  
Конвертирует списки адресов в бинарный формат (20 байт на адрес) для быстрого сканирования.  
Конвертує списки адрес у бінарний формат (20 байт на адрес) для швидкого сканування.

### Key features / Ключевые особенности / Ключові особливості:

- ✅ **Multi-threaded** conversion (4 threads) / Многопоточная конвертация (4 потока) / Багатопотокова конвертація (4 потоки)
- ✅ **Batch processing** (500k lines per chunk) / Пакетная обработка (500k строк на чанк) / Пакетна обробка (500k рядків на чанк)
- ✅ **Base58 decoding** (BTC, LTC, DOGE, DASH, ZEC) / Декодирование Base58 / Декодування Base58
- ✅ **Bech32 decoding** (bc1..., ltc1...) / Декодирование Bech32 / Декодування Bech32
- ✅ **Ethereum address** support (0x...) / Поддержка Ethereum адресов / Підтримка Ethereum адрес
- ✅ **Checksum verification** / Проверка контрольной суммы / Перевірка контрольної суми
- ✅ **Progress tracking** every 1M lines / Отслеживание прогресса каждые 1M строк / Відстеження прогресу кожні 1M рядків

### Supported address types / Поддерживаемые типы адресов / Підтримувані типи адрес:

| Type / Тип | Format / Формат | Example / Пример |
|------------|-----------------|------------------|
| Bitcoin P2PKH | Base58 (1...) | `1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa` |
| Bitcoin P2SH | Base58 (3...) | `3D2oetdNuZUqQHPJmcMDDHYoqkyNVsFk9r` |
| Bitcoin Bech32 | Bech32 (bc1...) | `bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq` |
| Litecoin P2PKH | Base58 (L...) | `Lb4swLdwRYkUaJjo2hxD73XbqP1JY8CkoW` |
| Litecoin Bech32 | Bech32 (ltc1...) | `ltc1q2c6rpghxt2vj65l3392c0fg6p0pj08uxfytl28` |
| Dogecoin P2PKH | Base58 (D...) | `DPMTNx1CtKN624Qau5zapzvzQn3iFn8Mcv` |
| Dash P2PKH | Base58 (X...) | `Xwgy5XVhDfVrAe49i8suL7kCv5kQ5xAWs5` |
| Zcash t1 | Base58 (t1...) | `t1PUN5JPBsrhBQNxx7GScVL8LDk6icZWi8c` |
| Zcash t3 | Base58 (t3...) | `t3NU7akXkc2xJLBPsuad1SyjHfyHypYzkwZ` |
| Ethereum | HEX (0x...) | `0x742d35Cc6634C0532925a3b844Bc454e4438f44e` |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -std=c++17 -O3 -pthread adrestobin.cpp -lssl -lcrypto -o adrestobin

# Run converter / Запустить конвертер / Запустити конвертер
./adrestobin

wszystkieadresy_unique.txt
--------------------------
1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa
3D2oetdNuZUqQHPJmcMDDHYoqkyNVsFk9r
bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq
0x742d35Cc6634C0532925a3b844Bc454e4438f44e
...
🤝 CONTRIBUTING

Contributions are welcome.
Submit a Pull Request or open an Issue.

📝 LICENSE

MIT License

📞 CONTACT

Author: ethicbrudhack


⭐ SUPPORT

If you find this project useful for research,
please give it a star!

Built for the cryptocurrency security research community 🔒

## 🔧 GENERATOR SEEDÓW - DOWOLNE ZNAKI / SEED GENERATOR - ARBITRARY CHARACTERS / ГЕНЕРАТОР СИДОВ - ПРОИЗВОЛЬНЫЕ СИМВОЛЫ

### Opis / Description / Описание:

Generator creates large files with seeds using **custom character sets** (e.g., "01", "0123456789", "ABCDEF").  
Генератор создает большие файлы с сидами, используя **пользовательские наборы символов** (например, "01", "0123456789", "ABCDEF").  
Генератор створює великі файли з сідами, використовуючи **користувацькі набори символів** (наприклад, "01", "0123456789", "ABCDEF").

**Purpose / Назначение / Призначення:**  
Generate seeds for testing, pattern analysis, or custom format requirements.  
Генерация сидов для тестирования, анализа паттернов или требований к формату.  
Генерація сідів для тестування, аналізу патернів або вимог до формату.

### Parametry / Parameters / Параметры:

| Parametr / Parameter / Параметр | Opis / Description / Описание | Default / По умолчанию |
|--------------------------------|-------------------------------|------------------------|
| `-f <file>` | Output filename / Имя выходного файла / Ім'я вихідного файлу | `seeds01.txt` |

**Configuration inside code / Конфигурация в коде / Конфігурація в коді:**

| Variable / Переменная | Opis / Description / Описание | Example / Пример |
|-----------------------|-------------------------------|------------------|
| `TARGET_SIZE` | Output file size in bytes / Размер выходного файла в байтах | `10GB = 1024*1024*1024*10` |
| `SEED_LENGTH` | Seed length in characters / Длина сида в символах | `64` |
| `CHARS` | Character set to use / Используемый набор символов | `"01"`, `"0123456789"`, `"ABCDEF"` |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -O3 -march=native -o losoweseedy losoweseedy.cpp

# Generate with default settings (10GB, "01" chars) / Сгенерировать с настройками по умолчанию (10GB, "01") / Згенерувати з налаштуваннями за замовчуванням (10GB, "01")
./losoweseedy

# Save to custom file / Сохранить в свой файл / Зберегти у власний файл
./losoweseedy -f my_seeds.txt


======================================================================
GENERATOR SEEDÓW - DOWOLNE ZNAKI
SEED GENERATOR - ARBITRARY CHARACTERS
ГЕНЕРАТОР СИДОВ - ПРОИЗВОЛЬНЫЕ СИМВОЛЫ
======================================================================

🚀 Generating 10240 MB of seeds / Генерирую 10240 MB сидов / Генерую 10240 MB сідів
   Length: 64 characters / Длина: 64 символа / Довжина: 64 символи
   Characters: 01 (2 chars) / Символы: 01 (2 символа) / Символи: 01 (2 символи)
   Each seed: 64 chars + newline = 65 bytes / Каждый сид: 64 символа + newline = 65 байт
   Seeds needed: ~165,000,000 / Нужно сидов: ~165,000,000 / Потрібно сідів: ~165,000,000

📊 45.23%  |  4631 MB / 10240 MB  |  125 MB/s  |  Seeds: 74,500,000
📊 100.00% |  10240 MB / 10240 MB |  125 MB/s  |  Seeds: 165,000,000

✅ Generated 165,000,000 seeds in 82.00 s
✅ Сгенерировано 165,000,000 сидов за 82.00 с
   Average speed: 125 MB/s / Средняя скорость: 125 MB/s
   File size: 10240 MB / Размер файла: 10240 MB

📋 First 5 generated seeds / Первые 5 сидов / Перші 5 сідів:
   01010101010101010101010101010101...0101010101010101
   10101010101010101010101010101010...1010101010101010
   01010101010101010101010101010101...0101010101010101
   10101010101010101010101010101010...1010101010101010
   01010101010101010101010101010101...0101010101010101


## 🔧 GENERATOR HEX Z WIODĄCYMI ZERAMI - TYLKO CYFRY / HEX GENERATOR WITH LEADING ZEROS - DIGITS ONLY / ГЕНЕРАТОР HEX С ВЕДУЩИМИ НУЛЯМИ - ТОЛЬКО ЦИФРЫ

### Opis / Description / Описание:

Generator creates 64-character hex seeds with **leading zeros** and **digits only (0-9)** with **variable entropy** (50-256 bits).  
Генератор создает 64-символьные hex-сиды с **ведущими нулями** и **только цифрами (0-9)** с **переменной энтропией** (50-256 бит).  
Генератор створює 64-символьні hex-сіди з **провідними нулями** та **лише цифрами (0-9)** з **змінною ентропією** (50-256 біт).

**Format / Формат / Формат:** `[00...00][digits 0-9]` / `[00...00][цифры 0-9]` / `[00...00][цифри 0-9]`

**Key difference / Ключевое отличие / Ключова відмінність:**  
Uses ONLY digits 0-9 (no A-F letters) - reduces entropy per character!  
Использует ТОЛЬКО цифры 0-9 (без букв A-F) - уменьшает энтропию на символ!  
Використовує ЛИШЕ цифри 0-9 (без літер A-F) - зменшує ентропію на символ!

### Parametry / Parameters / Параметры:

| Parametr / Parameter / Параметр | Opis / Description / Описание | Default / По умолчанию |
|--------------------------------|-------------------------------|------------------------|
| `-f <file>` | Output filename / Имя выходного файла / Ім'я вихідного файлу | `seeds_zeros.txt` |
| `-min <bits>` | Minimum entropy in bits / Минимальная энтропия в битах / Мінімальна ентропія в бітах | `50` |
| `-max <bits>` | Maximum entropy in bits / Максимальная энтропия в битах / Максимальна ентропія в бітах | `256` |
| `-n <count>` | Number of seeds to generate / Количество сидов для генерации / Кількість сідів для генерації | `100` |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -O3 -march=native -o generator_hex_same_cyfry generator_hex_same_cyfry.cpp

# Generate 10 seeds with 50-bit entropy (digits only) / Сгенерировать 10 сидов с 50-битной энтропией (только цифры) / Згенерувати 10 сідів з 50-бітною ентропією (лише цифри)
./generator_hex_same_cyfry -min 50 -max 50 -n 10

# Generate 100 seeds with 50-256 bit entropy / Сгенерировать 100 сидов с энтропией 50-256 бит / Згенерувати 100 сідів з ентропією 50-256 біт
./generator_hex_same_cyfry -min 50 -max 256 -n 100

# Save to custom file / Сохранить в свой файл / Зберегти у власний файл
./generator_hex_same_cyfry -f my_seeds.txt -min 50 -max 256 -n 1000




======================================================================
GENERATOR HEX Z WIODĄCYMI ZERAMI - TYLKO CYFRY
HEX GENERATOR WITH LEADING ZEROS - DIGITS ONLY
ГЕНЕРАТОР HEX С ВЕДУЩИМИ НУЛЯМИ - ТОЛЬКО ЦИФРЫ
======================================================================

🚀 Generating 100 hex seeds / Генерирую 100 hex-сидов / Генерую 100 hex-сідів
   Length: 64 characters / Длина: 64 символа / Довжина: 64 символи
   Entropy: 50 - 256 bits / Энтропия: 50 - 256 бит / Ентропія: 50 - 256 біт
   Format: [zeros] + [random DIGITS 0-9] / [нули] + [случайные ЦИФРЫ 0-9]
   Random digits: ~13 - 64 / Случайных цифр: ~13 - 64

📊 100%  |  100 / 100  |  12500 seeds/s  |  Avg entropy: 153 bits
📊 100%  |  100 / 100  |  12500 seeds/s  |  Средняя энтропия: 153 бит

✅ Generated 100 seeds in 0.08 s / Сгенерировано 100 сидов за 0.08 с
   Average speed: 12500 seeds/s / Средняя скорость: 12500 сидов/с
   Avg entropy: 153 bits / Средняя энтропия: 153 бит
   Min entropy: 50 bits / Мин. энтропия: 50 бит
   Max entropy: 256 bits / Макс. энтропия: 256 бит

📋 First 10 seeds preview / Первые 10 сидов / Перші 10 сідів:
   00000000000000000000...0000123456  (zeros: 50, digits: 14, entropy: 56 bits)
   00000000000000000000...0000789012  (zeros: 48, digits: 16, entropy: 64 bits)
   00000000000000000000...0345678901  (zeros: 45, digits: 19, entropy: 76 bits)
   00000000000000000000...4567890123  (zeros: 42, digits: 22, entropy: 88 bits)
   ...



## 🔧 GENERATOR HEX Z WIODĄCYMI ZERAMI / HEX GENERATOR WITH LEADING ZEROS / ГЕНЕРАТОР HEX С ВЕДУЩИМИ НУЛЯМИ

### Opis / Description / Описание:

Generator creates 64-character hex seeds with **leading zeros** and **variable entropy** (60-130 bits).  
Генератор создает 64-символьные hex-сиды с **ведущими нулями** и **переменной энтропией** (60-130 бит).  
Генератор створює 64-символьні hex-сіди з **провідними нулями** та **змінною ентропією** (60-130 біт).

**Format / Формат / Формат:** `[00...00][random hex chars]` / `[00...00][hex-символи]` / `[00...00][hex-символи]`

### Parametry / Parameters / Параметры:

| Parametr / Parameter / Параметр | Opis / Description / Описание | Default / По умолчанию |
|--------------------------------|-------------------------------|------------------------|
| `-f <file>` | Output filename / Имя выходного файла / Ім'я вихідного файлу | `seeds_zeros.txt` |
| `-min <bits>` | Minimum entropy in bits / Минимальная энтропия в битах / Мінімальна ентропія в бітах | `60` |
| `-max <bits>` | Maximum entropy in bits / Максимальная энтропия в битах / Максимальна ентропія в бітах | `130` |
| `-n <count>` | Number of seeds to generate / Количество сидов для генерации / Кількість сідів для генерації | `100` |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -O3 -march=native -o generator_hex generator_hex.cpp

# Generate 10 seeds with 60-bit entropy / Сгенерировать 10 сидов с 60-битной энтропией / Згенерувати 10 сідів з 60-бітною ентропією
./generator_hex -min 60 -max 60 -n 10

# Generate 100 seeds with 60-130 bit entropy / Сгенерировать 100 сидов с энтропией 60-130 бит / Згенерувати 100 сідів з ентропією 60-130 біт
./generator_hex -min 60 -max 130 -n 100

# Save to custom file / Сохранить в свой файл / Зберегти у власний файл
./generator_hex -f my_seeds.txt -min 60 -max 130 -n 1000





======================================================================
GENERATOR HEX WITH LEADING ZEROS / ГЕНЕРАТОР HEX С ВЕДУЩИМИ НУЛЯМИ
======================================================================

🚀 Generating 100 hex seeds / Генерирую 100 hex-сидов / Генерую 100 hex-сідів
   Length: 64 characters / Длина: 64 символа / Довжина: 64 символи
   Entropy: 60 - 130 bits / Энтропия: 60 - 130 бит / Ентропія: 60 - 130 біт
   Format: [zeros] + [random hex chars] / [нули] + [случайные hex-символы]
   Random chars: ~15 - 33 / Случайных символов: ~15 - 33

📊 100%  |  100 / 100  |  12500 seeds/s  |  Avg entropy: 95 bits
📊 100%  |  100 / 100  |  12500 seeds/s  |  Средняя энтропия: 95 бит

✅ Generated 100 seeds in 0.08 s / Сгенерировано 100 сидов за 0.08 с
   Average speed: 12500 seeds/s / Средняя скорость: 12500 сидов/с
   Avg entropy: 95 bits / Средняя энтропия: 95 бит
   Min entropy: 60 bits / Мин. энтропия: 60 бит
   Max entropy: 130 bits / Макс. энтропия: 130 бит

📋 First 10 seeds preview / Первые 10 сидов / Перші 10 сідів:
   00000000000000000000...00000000  (zeros: 40, random: 24, entropy: 96 bits)
   00000000000000000000...0000a3f1  (zeros: 40, random: 24, entropy: 96 bits)
   00000000000000000000...00b5c9d2  (zeros: 42, random: 22, entropy: 88 bits)
   00000000000000000000...0c7e4a1b  (zeros: 40, random: 24, entropy: 96 bits)
   ...



## 🔧 GENEROWANIE SEEDÓW / ГЕНЕРАЦИЯ СИДОВ

### Opcje generatora / Опции генератора:

| Generator / Генератор | Entropia / Энтропия | Opis / Описание |
|-----------------------|---------------------|-----------------|
| `LCG 16-bit` | 16 bitów / бит | Wygląda na 256-bit ale ma TYLKO 16 bitów! / Выглядит как 256-битный, но имеет ТОЛЬКО 16 бит! |
| `glibc rand()` | 31 bitów / бит | Standardowy rand() z glibc / Стандартный rand() из glibc |
| `MSVC rand()` | 15 bitów / бит | Najgorszy - tylko 15 bitów! / Худший - только 15 бит! |
| `MINSTD` | 31 bitów / бит | Minimalny standard / Минимальный стандарт |
| `RANDU` | 31 bitów / бит | Słynnie zły - widać wzór! / Печально известный плохой - виден паттерн! |
| `Java Random` | 48 bitów / бит | Random z Javy / Random из Java |
| `Mersenne Twister` | 32 bity / бита | **Milk Sad** - podatność CVE-2023-31290 / **Milk Sad** - уязвимость CVE-2023-31290 |
| `time(NULL)` | 32 bity / бита | Czas jako seed / Время как сид |
| `PID + time` | 32 bity / бита | PID + czas / PID + время |
| `Weak /dev/urandom` | 32 bity / бита | Słaby RNG / Слабый RNG |

### Uruchomienie generatora / Запуск генератора:

```bash
# Kompilacja / Компиляция
g++ -O3 -march=native -o weak_generators weak_generators.cpp

# Generuj 10 seedów z każdego generatora / Сгенерировать 10 сидов из каждого генератора
./weak_generators -n 10

# Generuj 100 seedów / Сгенерировать 100 сидов
./weak_generators -n 100

# Zapisz do własnego pliku / Сохранить в свой файл
./weak_generators -n 50 -f moje_seedy.txt



======================================================================
GENERATOR HEX Z WIODĄCYMI ZERAMI
ГЕНЕРАТОР HEX С ВЕДУЩИМИ НУЛЯМИ
======================================================================

🚀 Generuję 100 seedów hex
🚀 Генерирую 100 hex-сидов
   Długość: 64 znaki
   Длина: 64 символа
   Entropia: 60 - 130 bitów
   Энтропия: 60 - 130 бит
   Format: [zera] + [losowe znaki hex]
   Формат: [нули] + [случайные hex-символы]
   Losowych znaków: ~15 - 33
   Случайных символов: ~15 - 33

📊 100%  |  100 / 100  |  12500 seeds/s  |  Śr. entropia: 95 bitów
📊 100%  |  100 / 100  |  12500 seeds/s  |  Ср. энтропия: 95 бит

✅ Wygenerowano 100 seedów w 0.08 s
✅ Сгенерировано 100 сидов за 0.08 с
   Średnia prędkość: 12500 seeds/s
   Средняя скорость: 12500 сидов/с
   Średnia entropia: 95 bitów
   Средняя энтропия: 95 бит
   Min entropia: 60 bitów
   Мин. энтропия: 60 бит
   Max entropia: 130 bitów
   Макс. энтропия: 130 бит

📋 Podgląd pierwszych 10 seedów:
📋 Просмотр первых 10 сидов:
   00000000000000000000...00000000  (zera: 40, losowych: 24, entropia: 96 bitów)
   00000000000000000000...0000a3f1  (zera: 40, losowych: 24, энтропия: 96 бит)
   00000000000000000000...00b5c9d2  (zera: 42, losowych: 22, entropia: 88 bitów)
   00000000000000000000...0c7e4a1b  (zera: 40, losowych: 24, энтропия: 96 бит)
   ...

## 🔧 GENERATOR MIESZANYCH WZORCÓW 64-ZNAKOWYCH / MIXED PATTERN GENERATOR 64-CHARACTER / ГЕНЕРАТОР СМЕШАННЫХ ШАБЛОНОВ 64-СИМВОЛЬНЫЙ

### Opis / Description / Описание:

Generator creates **ALL POSSIBLE COMBINATIONS** of given characters (mixed patterns) with length 64.  
Генератор создает **ВСЕ ВОЗМОЖНЫЕ КОМБИНАЦИИ** заданных символов (смешанные шаблоны) длиной 64.  
Генератор створює **ВСІ МОЖЛИВІ КОМБІНАЦІЇ** заданих символів (змішані шаблони) довжиною 64.

**Key feature / Ключевая особенность / Ключова особливість:**  
Generates COMBINATIONS (all permutations) of characters, not random seeds!  
Генерирует КОМБИНАЦИИ (все перестановки) символов, а не случайные сиды!  
Генерує КОМБІНАЦІЇ (всі перестановки) символів, а не випадкові сіди!

### Parametry / Parameters / Параметры:

| Parametr / Parameter / Параметр | Opis / Description / Описание | Default / По умолчанию |
|--------------------------------|-------------------------------|------------------------|
| `-c <chars>` | Character set / Набор символов / Набір символів | `"01"` |
| `-l <length>` | Pattern length / Длина шаблона / Довжина шаблону | `64` |
| `-m <limit>` | Max patterns / Лимит шаблонов / Ліміт шаблонів | `0` (no limit / без лимита) |
| `-f <file>` | Output filename / Имя выходного файла / Ім'я вихідного файлу | `seeds0.txt` |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -O3 -march=native -o nowyprostygenerator nowyprostygenerator.cpp

# Generate first 30 patterns of length 64 using "01" / Сгенерировать первые 30 шаблонов длиной 64 используя "01" / Згенерувати перші 30 шаблонів довжиною 64 використовуючи "01"
./nowyprostygenerator -c 01 -l 64 -m 30

# Generate first 50 patterns using "012" / Сгенерировать первые 50 шаблонов используя "012" / Згенерувати перші 50 шаблонів використовуючи "012"
./nowyprostygenerator -c 012 -l 64 -m 50

# Generate first 30 patterns using "abc" / Сгенерировать первые 30 шаблонов используя "abc" / Згенерувати перші 30 шаблонів використовуючи "abc"
./nowyprostygenerator -c abc -l 64 -m 30

# Generate first 100 patterns using hex characters / Сгенерировать первые 100 шаблонов используя hex-символы / Згенерувати перші 100 шаблонів використовуючи hex-символи
./nowyprostygenerator -c 0123456789abcdef -l 64 -m 100

# Save to custom file / Сохранить в свой файл / Зберегти у власний файл
./nowyprostygenerator -f my_patterns.txt -c 01 -l 64 -m 30

======================================================================
GENERATOR MIESZANYCH WZORCÓW 64-ZNAKOWYCH
MIXED PATTERN GENERATOR 64-CHARACTER
ГЕНЕРАТОР СМЕШАННЫХ ШАБЛОНОВ 64-СИМВОЛЬНЫЙ
======================================================================

🚀 Generating MIXED patterns of length 64
🚀 Генерирую СМЕШАННЫЕ шаблоны длиной 64
🚀 Генерую ЗМІШАНІ шаблони довжиною 64
   Characters: 01 (2 chars) / Символы: 01 (2 символа) / Символи: 01 (2 символи)
   All possible: 2^64 = 18,446,744,073,709,551,616
   Limit: 30 / Лимит: 30 / Ліміт: 30

⚠️  WARNING / ПРЕДУПРЕЖДЕНИЕ / ПОПЕРЕДЖЕННЯ:
   For 3 chars and length 64 = 3^64 patterns!
   Для 3 символов и длины 64 = 3^64 шаблонов!
   Для 3 символів і довжини 64 = 3^64 шаблонів!
   ALWAYS use -m limit! / ВСЕГДА используйте -m лимит!

📊 15 / 30  |  1250/s
📊 30 / 30  |  1250/s

✅ Saved 30 patterns in 0.02 s / Сохранено 30 шаблонов за 0.02 с
   Average speed: 1250 patterns/s / Средняя скорость: 1250 шаблонов/с

📁 File: seeds0.txt
   Size: 2 KB

## 🔧 GENERATOR SŁABYCH SEEDÓW / WEAK SEED GENERATOR / ГЕНЕРАТОР СЛАБЫХ СИДОВ

### Opis / Description / Описание:

Generator creates 64-character hex seeds using **10 different weak random number generators**.  
Генератор создает 64-символьные hex-сиды используя **10 различных слабых генераторов случайных чисел**.  
Генератор створює 64-символьні hex-сіди використовуючи **10 різних слабких генераторів випадкових чисел**.

**Key feature / Ключевая особенность / Ключова особливість:**  
All seeds **LOOK** like 256-bit secure keys but have **LOW ENTROPY (15-48 bits)**!  
Все сиды **ВЫГЛЯДЯТ** как безопасные 256-битные ключи, но имеют **НИЗКУЮ ЭНТРОПИЮ (15-48 бит)**!  
Всі сіди **ВИГЛЯДАЮТЬ** як безпечні 256-бітні ключі, але мають **НИЗЬКУ ЕНТРОПІЮ (15-48 біт)**!

### Generators / Генераторы / Генератори:

| # | Name / Название | Entropy / Энтропия | Description / Описание |
|---|-----------------|-------------------|-----------------------|
| 1 | `LCG 16-bit` | 16 bits / бит | Wygląda na 256-bit ale ma TYLKO 16 bitów! / Выглядит как 256-битный, но имеет ТОЛЬКО 16 бит! |
| 2 | `glibc rand()` | 31 bits / бит | Standardowy rand() z glibc / Стандартный rand() из glibc |
| 3 | `MSVC rand()` | 15 bits / бит | Najgorszy - tylko 15 bitów! / Худший - только 15 бит! |
| 4 | `MINSTD` | 31 bits / бит | Minimalny standard / Минимальный стандарт |
| 5 | `RANDU` | 31 bits / бит | Słynnie zły - wzór widoczny! / Печально известный плохой - виден паттерн! |
| 6 | `Java Random` | 48 bits / бит | Random z Javy / Random из Java |
| 7 | `Mersenne Twister` | 32 bits / бита | **Milk Sad** - CVE-2023-31290 / **Milk Sad** - уязвимость CVE-2023-31290 |
| 8 | `time(NULL)` | 32 bits / бита | Czas jako seed / Время как сид |
| 9 | `PID + time` | 32 bits / бита | PID + czas / PID + время |
| 10 | `Weak /dev/urandom` | 32 bits / бита | Słaby RNG / Слабый RNG |

### Usage / Использование / Використання:

```bash
# Compilation / Компиляция / Компіляція
g++ -O3 -march=native -o popularneslaberng popularneslaberng.cpp

# Generate 10 seeds from each generator / Сгенерировать 10 сидов из каждого генератора / Згенерувати 10 сідів з кожного генератора
./popularneslaberng -n 10

# Generate 100 seeds from each generator / Сгенерировать 100 сидов из каждого генератора / Згенерувати 100 сідів з кожного генератора
./popularneslaberng -n 100

# Generate 1000 seeds and save to custom file / Сгенерировать 1000 сидов и сохранить в свой файл / Згенерувати 1000 сідів і зберегти у власний файл
./popularneslaberng -n 1000 -f my_weak_seeds.txt

# Show help / Показать помощь / Показати допомогу
./popularneslaberng -h

================================================================================
GENERATOR SŁABYCH SEEDÓW - PRAWIDŁOWY
WEAK SEED GENERATOR - PROPER
ГЕНЕРАТОР СЛАБЫХ СИДОВ - ПРАВИЛЬНЫЙ
================================================================================

📁 Zapisuję do: weak_seeds.txt
📁 Сохраняю в: weak_seeds.txt
📊 Generuję 10 seedów na generator
📊 Генерирую 10 сидов на генератор

🔹 LCG 16-bit
   Tylko 16 bitów entropii (ale wygląda na 256-bit)
   Только 16 бит энтропии (но выглядит как 256-битный)
   Przykład: 30393039303930393039303930393039...
   Пример:   30393039303930393039303930393039...

🔹 glibc rand()
   Tylko 31 bitów entropii
   Только 31 бит энтропии
   Przykład: 7a6b3c4d5e6f7a8b9c0d1e2f3a4b5c6d...
   Пример:   7a6b3c4d5e6f7a8b9c0d1e2f3a4b5c6d...

🔹 MSVC rand()
   Tylko 15 bitów entropii (NAJGORSZY!)
   Только 15 бит энтропии (ХУДШИЙ!)
   Przykład: 12345678123456781234567812345678...
   Пример:   12345678123456781234567812345678...

🔹 Mersenne Twister (Milk Sad)
   Tylko 32 bity entropii
   Только 32 бита энтропии
   Przykład: a6c9a6c9a6c9a6c9a6c9a6c9a6c9a6c9...
   Пример:   a6c9a6c9a6c9a6c9a6c9a6c9a6c9a6c9...

✅ Wygenerowano 100 seedów do pliku: weak_seeds.txt
✅ Сгенерировано 100 сидов в файл: weak_seeds.txt

⚠️  OSTRZEŻENIE / ПРЕДУПРЕЖДЕНИЕ:
   Te seedy WYGLĄDAJĄ na 256-bitowe
   Эти сиды ВЫГЛЯДЯТ как 256-битные
   ale mają MAŁĄ ENTROPIĘ (15-48 bitów)!
   но имеют МАЛУЮ ЭНТРОПИЮ (15-48 бит)!
