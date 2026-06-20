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

const char* BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

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
// DEKODOWANIE BASE58
// ============================================================
int base58_char_value(char c) {
    const char* p = strchr(BASE58_ALPHABET, c);
    return p ? p - BASE58_ALPHABET : -1;
}

bool base58_decode_20(const std::string& input, std::vector<unsigned char>& output) {
    // 1. Oblicz leading zeros
    int leading_zeros = 0;
    for (char c : input) {
        if (c == '1') leading_zeros++;
        else break;
    }
    
    // 2. Dekoduj Base58
    std::vector<unsigned char> temp(32, 0);
    for (char c : input) {
        int carry = base58_char_value(c);
        if (carry == -1) return false;
        
        for (int i = 31; i >= 0; --i) {
            carry += 58 * temp[i];
            temp[i] = carry & 0xFF;
            carry >>= 8;
        }
        if (carry != 0) return false;
    }
    
    // 3. Znajdź pierwszy niezerowy bajt
    size_t start = 0;
    while (start < temp.size() && temp[start] == 0) start++;
    
    // 4. Określ wersję (Zcash ma 2 bajty)
    int version_bytes = 1;
    bool is_zcash = false;
    if (input.size() > 1 && input[0] == 't' && (input[1] == '1' || input[1] == '3')) {
        is_zcash = true;
        version_bytes = 2;
    }
    
    // 5. Zbuduj payload
    std::vector<unsigned char> payload;
    payload.reserve(leading_zeros + (temp.size() - start));
    
    for (int i = 0; i < leading_zeros; i++) {
        payload.push_back(0x00);
    }
    for (size_t i = start; i < temp.size(); i++) {
        payload.push_back(temp[i]);
    }
    
    // 6. Sprawdź długość
    size_t expected_size = version_bytes + 20 + 4;
    if (payload.size() < expected_size) return false;
    
    // 7. Weź OSTATNIE 25 (lub 26) bajtów
    std::vector<unsigned char> decoded_payload(
        payload.end() - expected_size,
        payload.end()
    );
    
    // 8. Sprawdź checksum
    if (!verify_checksum(decoded_payload)) return false;
    
    // 9. Wyciągnij 20 bajtów hash
    output.assign(
        decoded_payload.begin() + version_bytes,
        decoded_payload.begin() + version_bytes + 20
    );
    
    return true;
}

// ============================================================
// DEKODOWANIE BECH32
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
    
    // Tylko P2WPKH (20 bajtów)
    return output.size() == 20;
}

// ============================================================
// DEKODOWANIE ETH
// ============================================================
bool eth_decode(const std::string& addr, std::vector<unsigned char>& output) {
    if (addr.size() != 42) return false;
    if (!(addr.rfind("0x", 0) == 0 || addr.rfind("0X", 0) == 0)) return false;
    
    std::string hex = addr.substr(2);
    if (hex.size() != 40) return false;
    
    output.resize(20);
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
        output[i] = (h << 4) | l;
    }
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
    
    if (addr.substr(0, 3) == "bc1" || 
        addr.substr(0, 4) == "ltc1" ||
        addr.substr(0, 3) == "tb1") {
        return TYPE_BECH32;
    }
    
    return TYPE_BASE58;
}

// ============================================================
// PRZETWARZANIE CHUNKÓW - TYLKO BIN
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
    
    for (size_t i = start_idx; i < end_idx; ++i) {
        const std::string& line = lines[i];
        if (line.empty()) continue;
        
        AddressType type = detect_type(line);
        bool success = false;
        
        switch (type) {
            case TYPE_BASE58:
                success = base58_decode_20(line, hash160);
                break;
            case TYPE_BECH32:
                success = bech32_decode(line, hash160);
                break;
            case TYPE_ETH:
                success = eth_decode(line, hash160);
                break;
            case TYPE_UNKNOWN:
                success = false;
                break;
        }
        
        if (!success || hash160.size() != 20) continue;
        
        buffer_bin.insert(buffer_bin.end(), hash160.begin(), hash160.end());
        found++;
    }
    
    if (!buffer_bin.empty()) {
        std::lock_guard<std::mutex> lock(fout_mutex);
        fout_bin.write(reinterpret_cast<const char*>(buffer_bin.data()), buffer_bin.size());
    }
    
    processed += (end_idx - start_idx);
    
    if (processed % 1000000 == 0) {
        std::cout << "\rProcessed: " << processed.load() / 1000000.0 << " M lines"
                  << " | Found: " << found.load()
                  << "     " << std::flush;
    }
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::ifstream fin("wszystkieadresy_unique.txt");
    if (!fin) {
        std::cerr << "Cannot open input file: wszystkieadresy_unique.txt" << std::endl;
        return 1;
    }
    
    std::ofstream fout_bin("adresy.bin", std::ios::binary);
    if (!fout_bin) {
        std::cerr << "Cannot create adresy.bin" << std::endl;
        return 1;
    }
    
    const size_t CHUNK_LINES = 500000;
    std::vector<std::string> lines;
    lines.reserve(CHUNK_LINES);
    
    std::mutex fout_mutex;
    std::atomic<uint64_t> processed(0);
    std::atomic<uint64_t> found(0);
    std::string line;
    
    std::cout << "=== ADDRESS CONVERTER (BIN ONLY) ===" << std::endl;
    std::cout << "Output: adresy.bin (20 bytes per address)" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "Starting conversion..." << std::endl;
    
    while (true) {
        lines.clear();
        for (size_t i = 0; i < CHUNK_LINES && std::getline(fin, line); ++i) {
            if (!line.empty()) lines.push_back(std::move(line));
        }
        if (lines.empty()) break;
        
        int num_threads = std::min(4, (int)std::thread::hardware_concurrency());
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
    std::cout << "Saved to: adresy.bin" << std::endl;
    
    return 0;
}
