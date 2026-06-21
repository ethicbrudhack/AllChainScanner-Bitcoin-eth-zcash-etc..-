#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/evp.h>

const char* BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// ============================================================
// STRUKTURA DLA COINÓW
// ============================================================
struct CoinInfo {
    std::string name;
    std::string prefix;
    std::vector<uint8_t> version_bytes;  // Może być 1 lub 2 bajty
    bool is_zcash_style;  // Zcash ma 2-bajtową wersję
    bool is_bech32;
    std::string bech32_hrp;
};

std::vector<CoinInfo> COINS = {
    // Bitcoin
    {"Bitcoin_P2PKH", "1", {0x00}, false, false, ""},
    {"Bitcoin_P2SH", "3", {0x05}, false, false, ""},
    {"Bitcoin_Bech32", "bc1", {}, false, true, "bc"},
    
    // Litecoin
    {"Litecoin_P2PKH", "L", {0x30}, false, false, ""},
    {"Litecoin_P2SH", "M", {0x32}, false, false, ""},
    {"Litecoin_Bech32", "ltc1", {}, false, true, "ltc"},
    
    // Dogecoin
    {"Dogecoin_P2PKH", "D", {0x1E}, false, false, ""},
    {"Dogecoin_P2SH", "A", {0x16}, false, false, ""},  // Dogecoin P2SH to "A" lub "9"
    {"Dogecoin_Bech32", "doge1", {}, false, true, "doge"},
    
    // Dash
    {"Dash_P2PKH", "X", {0x4C}, false, false, ""},
    {"Dash_P2SH", "7", {0x10}, false, false, ""},
    
    // Zcash (2-bajtowa wersja!)
    {"Zcash_P2PKH", "t1", {0x1C, 0xB8}, true, false, ""},
    {"Zcash_P2SH", "t3", {0x1C, 0xBD}, true, false, ""},
    
    // Ethereum (specjalne)
    {"Ethereum", "0x", {}, false, false, ""}
};

// ============================================================
// SHA256d - do weryfikacji checksum Base58
// ============================================================
bool verify_checksum(const std::vector<unsigned char>& data) {
    if (data.size() < 4) return false;
    
    unsigned char hash1[SHA256_DIGEST_LENGTH];
    unsigned char hash2[SHA256_DIGEST_LENGTH];
    
    SHA256(data.data(), data.size() - 4, hash1);
    SHA256(hash1, SHA256_DIGEST_LENGTH, hash2);
    
    for (int i = 0; i < 4; i++) {
        if (hash2[i] != data[data.size() - 4 + i]) {
            return false;
        }
    }
    return true;
}

// ============================================================
// DEKODOWANIE BASE58 - UNIWERSALNE
// ============================================================
int base58_char_value(char c) {
    const char* p = strchr(BASE58_ALPHABET, c);
    return p ? p - BASE58_ALPHABET : -1;
}

bool base58_decode_20(const std::string& input, std::vector<unsigned char>& output, CoinInfo& detected_coin) {
    if (input.empty()) return false;
    
    // 1. Wykryj coin po prefiksie
    detected_coin = {"Unknown", "", {0x00}, false, false, ""};
    bool found = false;
    
    for (const auto& coin : COINS) {
        if (coin.is_bech32) continue;
        if (coin.prefix.empty()) continue;
        
        if (input.compare(0, coin.prefix.length(), coin.prefix) == 0) {
            detected_coin = coin;
            found = true;
            break;
        }
    }
    
    if (!found) {
        // Spróbuj wykryć po pierwszym znaku
        std::string first_char = input.substr(0, 1);
        for (const auto& coin : COINS) {
            if (coin.is_bech32) continue;
            if (coin.prefix == first_char) {
                detected_coin = coin;
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        // Dla nieznanych - spróbuj jako Bitcoin
        detected_coin = COINS[0]; // Bitcoin_P2PKH
    }
    
    // 2. Policz leading '1'
    int leading_zeros = 0;
    while (leading_zeros < input.size() && input[leading_zeros] == '1') {
        leading_zeros++;
    }
    
    // 3. Dekoduj Base58
    std::vector<unsigned char> temp(32, 0);
    for (char c : input) {
        int carry = base58_char_value(c);
        if (carry == -1) return false;
        
        for (int i = 31; i >= 0; i--) {
            carry += 58 * temp[i];
            temp[i] = carry & 0xFF;
            carry >>= 8;
        }
        if (carry != 0) return false;
    }
    
    // 4. Znajdź pierwszy niezerowy
    int start = 0;
    while (start < temp.size() && temp[start] == 0) start++;
    
    // 5. Zbuduj payload
    std::vector<unsigned char> payload;
    payload.reserve(leading_zeros + (temp.size() - start));
    
    for (int i = 0; i < leading_zeros; i++) {
        payload.push_back(0x00);
    }
    for (int i = start; i < temp.size(); i++) {
        payload.push_back(temp[i]);
    }
    
    // 6. Określ rozmiar wersji
    int version_bytes = detected_coin.is_zcash_style ? 2 : 1;
    int expected_size = version_bytes + 20 + 4; // wersja + hash + checksum
    
    if (payload.size() < expected_size) return false;
    
    // 7. Pobierz OSTATNIE expected_size bajtów
    std::vector<unsigned char> decoded(
        payload.end() - expected_size,
        payload.end()
    );
    
    // 8. Sprawdź checksum
    if (!verify_checksum(decoded)) return false;
    
    // 9. Wyciągnij 20 bajtów hash (pomijając wersję)
    output.assign(decoded.begin() + version_bytes, decoded.begin() + version_bytes + 20);
    return output.size() == 20;
}

// ============================================================
// DEKODOWANIE BECH32 - UNIWERSALNE
// ============================================================
const std::string CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

int bech32_polymod_step(int pre) {
    int b = pre >> 25;
    return ((pre & 0x1FFFFFF) << 5) ^
        (-((b >> 0) & 1) & 0x3b6a57b2UL) ^
        (-((b >> 1) & 1) & 0x26508e6dUL) ^
        (-((b >> 2) & 1) & 0x1ea119faUL) ^
        (-((b >> 3) & 1) & 0x3d4233ddUL) ^
        (-((b >> 4) & 1) & 0x2a1462b3UL);
}

bool bech32_decode(const std::string& addr, std::vector<unsigned char>& output) {
    size_t pos = addr.find('1');
    if (pos == std::string::npos || pos == 0 || pos + 1 >= addr.size()) return false;
    
    std::string hrp = addr.substr(0, pos);
    std::string data = addr.substr(pos + 1);
    
    if (data.size() < 6) return false;
    
    // Walidacja znaków
    for (char c : data) {
        if (CHARSET.find(c) == std::string::npos) return false;
    }
    
    // Konwersja na wartości
    std::vector<int> values;
    values.reserve(data.size());
    for (char c : data) {
        size_t idx = CHARSET.find(c);
        if (idx == std::string::npos) return false;
        values.push_back(idx);
    }
    
    // Sprawdzenie checksum
    int chk = 1;
    for (char c : hrp) {
        chk = bech32_polymod_step(chk) ^ (c >> 5);
    }
    chk = bech32_polymod_step(chk);
    for (char c : hrp) {
        chk = bech32_polymod_step(chk) ^ (c & 31);
    }
    for (int v : values) {
        chk = bech32_polymod_step(chk) ^ v;
    }
    
    if (chk != 1) return false;
    
    // Usuń checksum
    values.resize(values.size() - 6);
    if (values.empty()) return false;
    
    // Pomijamy witness version
    std::vector<int> prog(values.begin() + 1, values.end());
    
    // Konwersja 5→8 bitów
    output.clear();
    int bits = 0;
    int acc = 0;
    for (int v : prog) {
        acc = (acc << 5) | v;
        bits += 5;
        while (bits >= 8) {
            bits -= 8;
            output.push_back((acc >> bits) & 0xFF);
        }
    }
    if (bits >= 5) return false;
    
    // Akceptuj P2WPKH (20) i P2WSH (32)
    return output.size() == 20 || output.size() == 32;
}

// ============================================================
// DEKODOWANIE ETH
// ============================================================
bool eth_decode(const std::string& addr, std::vector<unsigned char>& output) {
    if (addr.size() != 42) return false;
    if (!(addr.rfind("0x", 0) == 0 || addr.rfind("0X", 0) == 0)) return false;
    
    std::string hex = addr.substr(2);
    if (hex.size() != 40) return false;
    
    std::vector<unsigned char> raw(20);
    for (size_t i = 0; i < 20; i++) {
        char hi = hex[i * 2];
        char lo = hex[i * 2 + 1];
        
        auto cvt = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        
        int h = cvt(hi);
        int l = cvt(lo);
        if (h < 0 || l < 0) return false;
        raw[i] = (h << 4) | l;
    }
    
    // Konwertuj na hash160 (SHA256 + RIPEMD160)
    unsigned char sha_hash[SHA256_DIGEST_LENGTH];
    SHA256(raw.data(), 20, sha_hash);
    
    unsigned char ripemd_hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160(sha_hash, SHA256_DIGEST_LENGTH, ripemd_hash);
    
    output.assign(ripemd_hash, ripemd_hash + 20);
    return true;
}

// ============================================================
// ROZPOZNAWANIE TYPU
// ============================================================
enum AddressType {
    TYPE_BASE58,
    TYPE_BECH32,
    TYPE_ETH,
    TYPE_UNKNOWN
};

AddressType detect_type(const std::string& addr) {
    if (addr.empty()) return TYPE_UNKNOWN;
    
    if (addr.size() == 42 && 
        (addr.rfind("0x", 0) == 0 || addr.rfind("0X", 0) == 0)) {
        return TYPE_ETH;
    }
    
    // Wykryj Bech32 dla wszystkich coinów
    if (addr.find('1') != std::string::npos) {
        for (const auto& coin : COINS) {
            if (coin.is_bech32 && addr.find(coin.bech32_hrp) == 0) {
                return TYPE_BECH32;
            }
        }
    }
    
    return TYPE_BASE58;
}

// ============================================================
// DEKODOWANIE Z WYKRYCIEM COINA
// ============================================================
bool decode_address(const std::string& addr, std::vector<unsigned char>& output, std::string& coin_name) {
    AddressType type = detect_type(addr);
    CoinInfo detected_coin;
    bool success = false;
    
    switch (type) {
        case TYPE_BASE58:
            success = base58_decode_20(addr, output, detected_coin);
            if (success) coin_name = detected_coin.name;
            break;
        case TYPE_BECH32:
            success = bech32_decode(addr, output);
            if (success) {
                // Znajdź nazwę coina
                for (const auto& coin : COINS) {
                    if (coin.is_bech32 && addr.find(coin.bech32_hrp) == 0) {
                        coin_name = coin.name;
                        break;
                    }
                }
            }
            break;
        case TYPE_ETH:
            success = eth_decode(addr, output);
            if (success) coin_name = "Ethereum";
            break;
        case TYPE_UNKNOWN:
            success = false;
            break;
    }
    
    return success && output.size() == 20;
}

// ============================================================
// PRZETWARZANIE CHUNKÓW
// ============================================================
void process_chunk(const std::vector<std::string>& lines,
                   size_t start_idx,
                   size_t end_idx,
                   std::ofstream& fout_bin,
                   std::mutex& fout_mutex,
                   std::atomic<uint64_t>& processed,
                   std::atomic<uint64_t>& found)
{
    std::vector<unsigned char> buffer_bin;
    buffer_bin.reserve((end_idx - start_idx) * 20);
    
    std::vector<unsigned char> hash160;
    std::string coin_name;
    
    for (size_t i = start_idx; i < end_idx; ++i) {
        const std::string& line = lines[i];
        if (line.empty()) continue;
        
        if (decode_address(line, hash160, coin_name)) {
            if (hash160.size() == 20) {
                buffer_bin.insert(buffer_bin.end(), hash160.begin(), hash160.end());
                found++;
            }
        }
    }
    
    if (!buffer_bin.empty()) {
        std::lock_guard<std::mutex> lock(fout_mutex);
        fout_bin.write(reinterpret_cast<const char*>(buffer_bin.data()), buffer_bin.size());
    }
    
    processed += (end_idx - start_idx);
    
    if (processed % 10000 == 0) {
        std::cout << "\rProcessed: " << processed.load() 
                  << " | Found: " << found.load()
                  << " | Rate: " << (double)found.load() / processed.load() * 100 << "%   "
                  << std::flush;
    }
}

// ============================================================
// MAIN
// ============================================================
int main(int argc, char* argv[]) {
    std::string input_file = "wszystkieadresy_unique.txt";
    std::string output_file = "adresy.bin";
    
    if (argc >= 2) input_file = argv[1];
    if (argc >= 3) output_file = argv[2];
    
    std::ifstream fin(input_file);
    if (!fin) {
        std::cerr << "Cannot open input file: " << input_file << std::endl;
        return 1;
    }
    
    std::ofstream fout_bin(output_file, std::ios::binary);
    if (!fout_bin) {
        std::cerr << "Cannot create " << output_file << std::endl;
        return 1;
    }
    
    const size_t CHUNK_LINES = 500000;
    std::vector<std::string> lines;
    lines.reserve(CHUNK_LINES);
    
    std::mutex fout_mutex;
    std::atomic<uint64_t> processed(0);
    std::atomic<uint64_t> found(0);
    std::string line;
    
    std::cout << "=== UNIVERSAL ADDRESS CONVERTER ===" << std::endl;
    std::cout << "Input: " << input_file << std::endl;
    std::cout << "Output: " << output_file << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "Supported coins:" << std::endl;
    for (const auto& coin : COINS) {
        if (!coin.prefix.empty() && !coin.is_bech32) {
            std::string version_str;
            for (uint8_t v : coin.version_bytes) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%02X", v);
                version_str += buf;
            }
            std::cout << "  - " << coin.name << " (prefix: " << coin.prefix 
                      << ", version: 0x" << version_str << ")" << std::endl;
        } else if (coin.is_bech32) {
            std::cout << "  - " << coin.name << " (hrp: " << coin.bech32_hrp << "1)" << std::endl;
        }
    }
    std::cout << "======================================" << std::endl;
    std::cout << "Starting conversion..." << std::endl;
    
    while (true) {
        lines.clear();
        for (size_t i = 0; i < CHUNK_LINES && std::getline(fin, line); ++i) {
            if (!line.empty()) lines.push_back(line);
        }
        if (lines.empty()) break;
        
        int num_threads = std::min(8, (int)std::thread::hardware_concurrency());
        if (num_threads == 0) num_threads = 4;
        
        size_t per_thread = (lines.size() + num_threads - 1) / num_threads;
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            size_t start = i * per_thread;
            size_t end = std::min(lines.size(), start + per_thread);
            if (start >= end) break;
            threads.emplace_back(process_chunk, 
                               std::ref(lines), start, end,
                               std::ref(fout_bin),
                               std::ref(fout_mutex),
                               std::ref(processed), std::ref(found));
        }
        
        for (auto& t : threads) t.join();
    }
    
    std::cout << "\n\n=== CONVERSION FINISHED ===" << std::endl;
    std::cout << "Total lines processed: " << processed.load() << std::endl;
    std::cout << "Valid addresses found: " << found.load() << std::endl;
    std::cout << "Success rate: " << (double)found.load() / processed.load() * 100 << "%" << std::endl;
    std::cout << "Saved to: " << output_file << std::endl;
    
    return 0;
}
