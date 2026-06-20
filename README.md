Jasne — jeśli chcesz mieć ten opis w formie zwykłego tekstu do wklejenia do konsoli/terminala lub pliku README, możesz skopiować poniższy fragment:

```text
AllChainScanner - Multi-Coin BIP32 Address Scanner

🚀 High-Performance Cryptocurrency Address Scanner for 6+ Blockchains

This project demonstrates a multi-threaded cryptocurrency address scanner that generates addresses from private keys and checks them against a pre-sorted binary database. It supports compressed and uncompressed public keys across 6 major cryptocurrencies.

⚠️ Educational & Security Research Tool Only

This program is not for unauthorized wallet recovery or brute-forcing.
Use it only for controlled research, security testing, or educational purposes.

📋 OVERVIEW

This tool continuously processes private keys from a text file, derives addresses according to BIP32 standards, and compares them against a memory-mapped binary database of known addresses.

When a generated address exists in the database, the private key and corresponding address are saved to a results file.

Supported features:
- Multi-threaded parallel processing (60 threads)
- 6 cryptocurrencies simultaneously
- Compressed & uncompressed public key formats
- Memory-mapped I/O for ultra-fast lookups
- Binary search for O(log n) comparisons

✨ FEATURES

- Multi-Coin Support:
  Bitcoin, Litecoin, Dogecoin, Dash, Ethereum, Zcash

- Address Formats:
  P2PKH, P2SH, SegWit (Bech32), Ethereum 0x..., Zcash t1/t3

- Compressed/Uncompressed:
  Both public key formats supported

- Parallel Processing:
  60 concurrent threads for maximum CPU utilization

- Memory Mapping:
  mmap for fast binary file access

- Binary Search:
  O(log n) database lookups

- Speed Monitor:
  Real-time performance statistics

- Test Mode:
  Built-in test for private key = 1

SUPPORTED CRYPTOCURRENCIES

Bitcoin (BTC):
  P2PKH, P2SH, SegWit
  Prefixes: 1..., 3..., bc1...

Litecoin (LTC):
  P2PKH, P2SH, SegWit
  Prefixes: L..., M..., ltc1...

Dogecoin (DOGE):
  P2PKH
  Prefix: D...

Dash:
  P2PKH
  Prefix: X...

Ethereum (ETH):
  Prefix: 0x...

Zcash (ZEC):
  P2PKH, P2SH
  Prefixes: t1..., t3...

📊 PERFORMANCE

Processing Speed:
  2-5 Mkeys/s (CPU dependent)

Threads:
  60

Queue Size:
  200,000 keys

Address Variants per Key:
  ~50+

🔧 INSTALLATION

Ubuntu / Debian:

sudo apt-get install libssl-dev libsecp256k1-dev build-essential

macOS:

brew install openssl secp256k1

COMPILATION

g++ -std=c++17 -O3 -pthread -lssl -lcrypto -lsecp256k1 \
PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp \
-o allchainsanner

📖 USAGE

./allchainsanner <addresses.bin> <keys.txt>

Example:

./allchainsanner database.bin private_keys.txt

Input file format (keys.txt):

0000000000000000000000000000000000000000000000000000000000000001
0000000000000000000000000000000000000000000000000000000000000002

📁 OUTPUT

Console:

📁 Loaded 1,234,567 addresses from database.bin
🧵 Starting 60 threads
⚡ 3.45 Mkeys/s | seeds: 15234 | keys: 762,345 | found: 42

found.txt:

PRIV: 0000000000000000000000000000000000000000000000000000000000000001
PATH: direct_compressed
ADDR: 1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
---

PRIV: 0000000000000000000000000000000000000000000000000000000000000002
PATH: direct_uncompressed
ADDR: 1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm
---

🧪 TEST MODE

🔑 TEST FOR PRIVATE KEY = 1 (COMPRESSED + UNCOMPRESSED)

Bitcoin P2PKH (COMPRESSED):
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH

Bitcoin P2PKH (UNCOMPRESSED):
1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm

Bitcoin P2SH:
3JvL6Ymt8MVWiCNHC7oWU6nLeHNJKLZGLN

Bitcoin SegWit:
bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4

📦 DEPENDENCIES

- OpenSSL
- secp256k1
- pthread
- keccak_fixed.h

⚠️ DISCLAIMER

FOR EDUCATIONAL AND SECURITY RESEARCH PURPOSES ONLY

- This tool is not intended for illegal activities
- Do not use to access wallets you don't own
- The author takes no responsibility for misuse
- Always comply with local laws and regulations

📝 LICENSE

MIT License

🤝 CONTRIBUTING

Contributions are welcome.
Feel free to submit a Pull Request.

📧 CONTACT

Author: ethicbrudhack

GitHub: AllChainScanner

⭐ If you find this project useful, please give it a star!

DONATE: bc1qps62cyk9f9unmdkc9k3ccj9e2h8ywfhg2j53ec

Built with ❤️ for the crypto research community.
```
