Nazwa
PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp

Cel programu
Skrypt służy do generowania i sprawdzania adresów kryptowalutowych z pliku zawierającego klucze prywatne. Jest to narzędzie do skanowania wielu kryptowalut jednocześnie.

Główne funkcje
Wczytywanie danych

Czyta plik adresy.bin z posortowanymi 20-bajtowymi hashami adresów (RIPEMD-160)

Czyta plik tekstowy z kluczami prywatnymi (64 znaki hex w każdej linii)

Generowanie adresów
Dla każdego klucza prywatnego generuje:

Kompresowany i niekompresowany klucz publiczny

Adresy dla 6 kryptowalut:

Bitcoin (P2PKH, P2SH, SegWit)

Litecoin (P2PKH, P2SH, SegWit)

Dogecoin (P2PKH)

Dash (P2PKH)

Ethereum (0x...)

Zcash (t1..., t3...)

Szybkie wyszukiwanie

Używa binary search na posortowanym pliku

Plik jest mapowany do pamięci (mmap) dla szybkiego dostępu

Wielowątkowość

60 wątków konsumenckich

Kolejka zadań na 200,000 kluczy

Monitor prędkości w tle

Wydajność
⚠️ Niska prędkość z powodu:

Generowania ~50+ wariantów adresów na każdy klucz

Obsługi wielu kryptowalut jednocześnie

Obliczeń kryptograficznych (SHA, RIPEMD, secp256k1)

Typowa prędkość: 2-5 Mkeys/s (zależy od CPU)

Obsługiwane kryptowaluty
Moneta	Formaty adresów
Bitcoin	1..., 3..., bc1...
Litecoin	L..., M..., ltc1...
Dogecoin	D...
Dash	X...
Ethereum	0x...
Zcash	t1..., t3...
Kompilacja
bash
g++ -std=c++17 -O3 -pthread -lssl -lcrypto -lsecp256k1 PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp -o scanner
Użycie
bash
./scanner adresy.bin klucze.txt
Przykład działania
Program najpierw wykonuje test dla klucza prywatnego = 1, pokazując wszystkie wygenerowane adresy i porównując je z oczekiwanymi wartościami. Następnie przechodzi do właściwego skanowania.

Zalety
✅ Obsługa wielu kryptowalut
✅ Zarówno kompresowane jak i niekompresowane klucze
✅ Efektywne użycie pamięci (mmap)
✅ Równoległe przetwarzanie
✅ Szybkie wyszukiwanie binarne

Wady
❌ Wolniejsze niż skanery pojedynczych monet
❌ Wymaga dużej ilości RAM dla dużych plików
❌ Złożona konfiguracja

Pliki wyjściowe
found.txt - znalezione adresy z kluczami prywatnymi

Na ekranie - statystyki prędkości i postępu

To narzędzie jest idealne dla osób, które chcą sprawdzić swoje klucze prywatne w wielu blockchainach jednocześnie, ale muszą być świadome kompromisu między szybkością a funkcjonalnością.

This response is AI-generated, for reference only.
a angielski i ruski
ENGLISH
Multi-Coin BIP32 Scanner - Description
Program Name
PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp

Purpose
A cryptocurrency address scanner that processes private keys from a text file and checks them against multiple blockchain address databases simultaneously.

Key Features
Data Loading

Reads sorted adresy.bin file with 20-byte RIPEMD-160 address hashes

Reads text file with private keys (64 hex characters per line)

Address Generation
For each private key, generates:

Compressed and uncompressed public keys

Addresses for 6 cryptocurrencies:

Bitcoin (P2PKH, P2SH, SegWit)

Litecoin (P2PKH, P2SH, SegWit)

Dogecoin (P2PKH)

Dash (P2PKH)

Ethereum (0x...)

Zcash (t1..., t3...)

Fast Lookup

Uses binary search on sorted file

Memory-mapped file (mmap) for quick access

Multi-threading

60 consumer threads

Task queue of 200,000 keys

Background speed monitor

Performance
⚠️ Lower speed due to:

Generating ~50+ address variants per key

Supporting multiple coins simultaneously

Cryptographic computations (SHA, RIPEMD, secp256k1)

Typical speed: 2-5 Mkeys/s (CPU dependent)

Supported Coins
Coin	Address Formats
Bitcoin	1..., 3..., bc1...
Litecoin	L..., M..., ltc1...
Dogecoin	D...
Dash	X...
Ethereum	0x...
Zcash	t1..., t3...
Compilation
bash
g++ -std=c++17 -O3 -pthread -lssl -lcrypto -lsecp256k1 PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp -o scanner
Usage
bash
./scanner addresses.bin keys.txt
Output Files
found.txt - discovered addresses with private keys

Console - speed statistics and progress

Advantages
✅ Multi-coin support
✅ Compressed and uncompressed keys
✅ Memory efficient (mmap)
✅ Parallel processing
✅ Binary search for O(log n) lookups

Disadvantages
❌ Slower than single-coin scanners
❌ Higher RAM requirements
❌ Complex setup

РУССКИЙ
Мульти-монетный BIP32 сканер - Описание
Имя программы
PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp

Назначение
Сканер криптовалютных адресов, который обрабатывает приватные ключи из текстового файла и проверяет их по базам адресов нескольких блокчейнов одновременно.

Основные возможности
Загрузка данных

Читает отсортированный файл adresy.bin с 20-байтовыми хэшами RIPEMD-160

Читает текстовый файл с приватными ключами (64 hex символа в строке)

Генерация адресов
Для каждого приватного ключа генерирует:

Сжатые и несжатые публичные ключи

Адреса для 6 криптовалют:

Bitcoin (P2PKH, P2SH, SegWit)

Litecoin (P2PKH, P2SH, SegWit)

Dogecoin (P2PKH)

Dash (P2PKH)

Ethereum (0x...)

Zcash (t1..., t3...)

Быстрый поиск

Использует бинарный поиск по отсортированному файлу

Файл отображается в память (mmap) для быстрого доступа

Многопоточность

60 потребительских потоков

Очередь задач на 200,000 ключей

Фоновый монитор скорости

Производительность
⚠️ Низкая скорость из-за:

Генерации ~50+ вариантов адресов на каждый ключ

Поддержки множества монет одновременно

Криптографических вычислений (SHA, RIPEMD, secp256k1)

Типичная скорость: 2-5 Mключей/с (зависит от CPU)

Поддерживаемые монеты
Монета	Форматы адресов
Bitcoin	1..., 3..., bc1...
Litecoin	L..., M..., ltc1...
Dogecoin	D...
Dash	X...
Ethereum	0x...
Zcash	t1..., t3...
Компиляция
bash
g++ -std=c++17 -O3 -pthread -lssl -lcrypto -lsecp256k1 PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp -o scanner
Использование
bash
./scanner addresses.bin keys.txt
Выходные файлы
found.txt - найденные адреса с приватными ключами

Консоль - статистика скорости и прогресс

Преимущества
✅ Поддержка множества монет
✅ Сжатые и несжатые ключи
✅ Эффективное использование памяти (mmap)
✅ Параллельная обработка
✅ Бинарный поиск для O(log n) проверок

Недостатки
❌ Медленнее сканеров для одной монеты
❌ Требует больше RAM
❌ Сложная настройка

Summary / Резюме
This tool is ideal for users who want to check their private keys across multiple blockchains simultaneously, but they must be aware of the trade-off between speed and functionality.

Этот инструмент идеален для пользователей, которые хотят проверить свои приватные ключи в нескольких блокчейнах одновременно, но они должны осознавать компромисс между скоростью и функциональностью.
