Poniżej masz poprawioną wersję z właściwymi nazwami plików i opisami:

AllChainScanner - Multi-Coin BIP32 Scanner Suite

🔐 Advanced Cryptocurrency Address Scanner & Private Key Generator

A comprehensive suite of tools for cryptocurrency address generation, scanning, and private key generation. Designed for educational and security research purposes only.

⚠️ IMPORTANT: This software is for authorized security testing and educational research only. Do not use for unauthorized wallet access.

📦 PACKAGE CONTENTS

## File                                   Description

PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp
Main multi-coin BIP32 scanner
supporting multiple cryptocurrencies.

SPRAWDZPRIVKEY.cpp                     Optimized Bitcoin-only scanner
that checks only addresses
starting with "1" (P2PKH).

AddressConverter.cpp                   Converts text addresses into
binary database format (.bin).

generator_hex.cpp                      Generates random private keys
with entropy between 60 and
130 bits.

nowyprostygenerator.cpp                Sequential private key generator.
Generates one private key after
another in ascending order.

🚀 MAIN SCANNER - PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp

Features

• 6 Cryptocurrencies:
Bitcoin, Litecoin, Dogecoin, Dash, Ethereum, Zcash

• Multiple Address Formats:
P2PKH, P2SH, SegWit (Bech32), Ethereum 0x...

• Compressed & Uncompressed Keys:
Both formats checked

• Multi-threaded:
60 threads for maximum performance

• Memory-mapped I/O:
Fast binary file access

• Binary Search:
O(log n) lookups

Performance

• Speed: ~2-5 Mkeys/s (CPU dependent)
• ~50+ address variants per private key

Usage

./allchainsanner addresses.bin keys.txt

⚡ BITCOIN-ONLY SCANNER - SPRAWDZPRIVKEY.cpp

Features

• Checks only Bitcoin P2PKH addresses
• Addresses starting with "1"
• Lightweight implementation
• Minimal processing overhead
• Focused single-purpose scanner

Performance

• Speed: 0.5+ Mkeys/s
• Optimized for Bitcoin-only searches

Supported Paths

direct_compressed     - private key (compressed)
direct_uncompressed   - private key (uncompressed)
m/0-1                 - BIP32 derivation
m/44'/0'/0'/0/0-1     - BIP44 derivation

Usage

./fastbip32 addresses.bin seeds.txt

📂 ADDRESS CONVERTER - AddressConverter.cpp

Purpose

Converts cryptocurrency addresses from text format into
a binary database for high-speed scanning.

Supported Formats

• Bitcoin (1..., 3..., bc1...)
• Litecoin (L..., M..., ltc1...)
• Dogecoin (D...)
• Dash (X...)
• Ethereum (0x...)
• Zcash (t1..., t3...)

Output

• adresy.bin
• Sorted binary database
• Optimized for binary search
• Memory-mapped access

Usage

./address_converter

Input:
wszystkieadresy_unique.txt

Output:
adresy.bin

🔑 PRIVATE KEY GENERATORS

1. generator_hex.cpp

Generates random private keys with configurable entropy.

Features

• Entropy range: 60-130 bits
• Random private key generation
• High-speed generation
• Bulk generation support

Usage

./generator_hex -min 60 -max 130 -n 10000
./generator_hex -min 60 -max 60 -n 1000
./generator_hex -f seeds.txt -min 60 -max 130 -n 100000

Options

-min    Minimum entropy bits
-max    Maximum entropy bits
-n      Number of keys
-f      Output file

Example Output

0000000000000000000000000000000000000000000000000000000000a3f5c1
00000000000000000000000000000000000000000000000000001f3a8b9c2d
0000000000000000000000000000000000000000000000000a4b8c2d1e3f

2. nowyprostygenerator.cpp

Sequential private key generator.

Features

• Generates private keys sequentially
• One key after another
• Predictable ascending generation
• Lightweight implementation
• Suitable for controlled testing

Usage

./nowyprostygenerator

Example

0000000000000000000000000000000000000000000000000000000000000001
0000000000000000000000000000000000000000000000000000000000000002
0000000000000000000000000000000000000000000000000000000000000003

🔧 COMPILATION

Prerequisites

Ubuntu / Debian

sudo apt-get install libssl-dev libsecp256k1-dev build-essential

macOS

brew install openssl secp256k1

Compile All Tools

# Main Scanner

g++ -std=c++17 -O3 -pthread -lssl -lcrypto -lsecp256k1 
PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp 
-o allchainsanner

# Bitcoin Scanner

g++ -std=c++17 -O3 -pthread -lssl -lcrypto -lsecp256k1 
SPRAWDZPRIVKEY.cpp 
-o fastbip32

# Address Converter

g++ -std=c++17 -O3 -pthread -lssl -lcrypto 
AddressConverter.cpp 
-o address_converter

# Random Key Generator

g++ -std=c++17 -O3 -pthread 
generator_hex.cpp 
-o generator_hex

# Sequential Generator

g++ -std=c++17 -O3 -pthread 
nowyprostygenerator.cpp 
-o nowyprostygenerator

📊 PERFORMANCE COMPARISON

## Tool                                 Purpose

PROTOTYPGENEROWANIEZTXTWSZYSTKIERODZAJECOIN.cpp
Multi-coin scanner

SPRAWDZPRIVKEY.cpp                   Bitcoin-only scanner

generator_hex.cpp                    Random key generation

nowyprostygenerator.cpp              Sequential generation

🔍 WORKFLOW EXAMPLE

Step 1: Generate Private Keys

./generator_hex -min 60 -max 130 -n 1000000 -f keys.txt

Step 2: Prepare Address Database

./address_converter

Step 3: Run Scanner

./fastbip32 adresy.bin keys.txt

or

./allchainsanner adresy.bin keys.txt

Step 4: Check Results

cat found.txt

📁 OUTPUT FORMAT

PRIV:
0000000000000000000000000000000000000000000000000000000000a3f5c1

PATH:
direct_compressed

ADDR:
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH

---

⚠️ LEGAL DISCLAIMER

FOR EDUCATIONAL AND SECURITY RESEARCH PURPOSES ONLY

• This software is provided for authorized testing only
• Do not use to access wallets you do not own
• Unauthorized access to cryptocurrency wallets may be illegal
• The author assumes no responsibility for misuse
• Users are responsible for complying with local laws

🤝 CONTRIBUTING

Contributions are welcome.
Submit a Pull Request or open an Issue.

📝 LICENSE

MIT License

📞 CONTACT

Author: ethicbrudhack

Project: AllChainScanner

⭐ SUPPORT

If you find this project useful for research,
please give it a star!

Built for the cryptocurrency security research community 🔒
