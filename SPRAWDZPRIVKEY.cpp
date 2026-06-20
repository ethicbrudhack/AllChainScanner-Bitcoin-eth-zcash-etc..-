// ===============================
// FAST BIP32 SCANNER v15 - TYLKO WYBRANE ŚCIEŻKI (OPTIMAL)
// ===============================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <cctype>
#include <queue>
#include <condition_variable>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <secp256k1.h>

// ===============================
// PARAMETRY
// ===============================
static const int THREAD_COUNT = 30;
static const size_t QUEUE_SIZE = 100000;

std::mutex log_mutex;
std::mutex queue_mutex;
std::condition_variable cv;
std::queue<std::string> seed_queue;
std::atomic<bool> finished{false};
std::atomic<uint64_t> total_processed{0};
std::atomic<uint64_t> total_found{0};
std::atomic<uint64_t> total_keys{0};
auto start_time = std::chrono::steady_clock::now();
std::atomic<bool> monitor_stop{false};

// ===============================
// BASE58 - DOKŁADNIE TAK SAMO JAK W ORYGINALE
// ===============================
static const char* BASE58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string base58_encode(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> digits(data.size() * 138 / 100 + 1);
    int digitslen = 1;
    
    for (size_t i = 0; i < data.size(); i++) {
        unsigned int carry = data[i];
        for (int j = 0; j < digitslen; j++) {
            carry += (unsigned int)(digits[j]) << 8;
            digits[j] = (unsigned char)(carry % 58);
            carry /= 58;
        }
        while (carry) {
            digits[digitslen++] = (unsigned char)(carry % 58);
            carry /= 58;
        }
    }
    
    std::string result;
    for (int i = digitslen - 1; i >= 0; i--) {
        result += BASE58[digits[i]];
    }
    
    for (size_t i = 0; i < data.size() && data[i] == 0; i++) {
        result = '1' + result;
    }
    
    return result;
}

std::string base58check_encode(const std::vector<unsigned char>& data) {
    unsigned char hash1[32], hash2[32];
    SHA256(data.data(), data.size(), hash1);
    SHA256(hash1, 32, hash2);
    
    std::vector<unsigned char> extended = data;
    extended.insert(extended.end(), hash2, hash2 + 4);
    
    return base58_encode(extended);
}

// ===============================
// HASH160 - DOKŁADNIE TAK SAMO
// ===============================
inline void hash160(const unsigned char* data, size_t len, unsigned char out[20]) {
    unsigned char sha[32];
    SHA256(data, len, sha);
    RIPEMD160(sha, 32, out);
}

// ===============================
// MAPOWANIE PLIKU BINARNEGO
// ===============================
class MMapFile {
public:
    MMapFile(const char* path) {
        fd = ::open(path, O_RDONLY);
        if (fd < 0) throw std::runtime_error(std::string("open: ") + strerror(errno));

        struct stat st{};
        if (fstat(fd, &st) != 0)
            throw std::runtime_error(std::string("fstat: ") + strerror(errno));

        size = st.st_size;
        if (size == 0 || size % 20 != 0)
            throw std::runtime_error("invalid bin file (size must be multiple of 20)");

        data = (const unsigned char*) mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED)
            throw std::runtime_error(std::string("mmap: ") + strerror(errno));
    }

    ~MMapFile() {
        if (data) munmap((void*)data, size);
        if (fd >= 0) close(fd);
    }

    const unsigned char* ptr() const { return data; }
    size_t length() const { return size; }
    size_t count() const { return size / 20; }

private:
    int fd;
    const unsigned char* data;
    size_t size;
};

// Binary search
inline bool contains_address(const MMapFile& mm, const unsigned char addr20[20]) {
    const unsigned char* base = mm.ptr();
    size_t count = mm.count();

    size_t lo = 0, hi = count;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        const unsigned char* midp = base + mid * 20;
        int cmp = memcmp(midp, addr20, 20);
        if (cmp == 0) return true;
        if (cmp < 0) lo = mid + 1;
        else hi = mid;
    }
    return false;
}

// ===============================
// SZYBKI KONTEXT PUBKEY
// ===============================
struct FastPubCtx {
    secp256k1_context* ctx;
    secp256k1_pubkey pub;
};

inline bool fast_load_priv(FastPubCtx& pc, const unsigned char priv[32]) {
    return secp256k1_ec_pubkey_create(pc.ctx, &pc.pub, priv) == 1;
}

// Generuje adres i hash160 - dokładnie tak samo jak privkey_to_address()
inline std::string fast_priv_to_address(FastPubCtx& pc, bool compressed, unsigned char out_h160[20]) {
    unsigned char pubkey[65];
    size_t len = compressed ? 33 : 65;
    if (secp256k1_ec_pubkey_serialize(pc.ctx, pubkey, &len, &pc.pub, 
        compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED) != 1) {
        return "";
    }
    
    hash160(pubkey, len, out_h160);
    
    std::vector<unsigned char> data;
    data.push_back(0x00);
    data.insert(data.end(), out_h160, out_h160 + 20);
    
    return base58check_encode(data);
}

// ===============================
// BIP32 FUNKCJE - DOKŁADNIE TAK SAMO JAK W ORYGINALE
// ===============================
inline void bip32_master(const unsigned char* seed, size_t seed_len, unsigned char out_key[32], unsigned char out_chain[32]) {
    unsigned char hmac[64];
    unsigned int hmac_len = 64;
    HMAC(EVP_sha512(), "Bitcoin seed", 12, seed, seed_len, hmac, &hmac_len);
    memcpy(out_key, hmac, 32);
    memcpy(out_chain, hmac + 32, 32);
}

inline bool bip32_derive_normal(secp256k1_context* ctx, const unsigned char parent_key[32], const unsigned char parent_chain[32],
                         uint32_t child_num, unsigned char child_key[32], unsigned char child_chain[32]) {
    secp256k1_pubkey parent_pub;
    if (secp256k1_ec_pubkey_create(ctx, &parent_pub, parent_key) != 1) {
        return false;
    }
    
    unsigned char pubkey[33];
    size_t pubkey_len = 33;
    if (secp256k1_ec_pubkey_serialize(ctx, pubkey, &pubkey_len, &parent_pub, SECP256K1_EC_COMPRESSED) != 1) {
        return false;
    }
    
    unsigned char data[37];
    memcpy(data, pubkey, 33);
    data[33] = (child_num >> 24) & 0xFF;
    data[34] = (child_num >> 16) & 0xFF;
    data[35] = (child_num >> 8) & 0xFF;
    data[36] = child_num & 0xFF;
    
    unsigned char hmac[64];
    unsigned int hmac_len = 64;
    HMAC(EVP_sha512(), parent_chain, 32, data, 37, hmac, &hmac_len);
    
    memcpy(child_key, hmac, 32);
    memcpy(child_chain, hmac + 32, 32);
    
    return true;
}

inline bool bip32_derive_hardened(const unsigned char parent_key[32], const unsigned char parent_chain[32],
                           uint32_t child_num, unsigned char child_key[32], unsigned char child_chain[32]) {
    unsigned char data[37];
    data[0] = 0;
    memcpy(data + 1, parent_key, 32);
    uint32_t hardened_num = child_num + 0x80000000;
    data[33] = (hardened_num >> 24) & 0xFF;
    data[34] = (hardened_num >> 16) & 0xFF;
    data[35] = (hardened_num >> 8) & 0xFF;
    data[36] = hardened_num & 0xFF;
    
    unsigned char hmac[64];
    unsigned int hmac_len = 64;
    HMAC(EVP_sha512(), parent_chain, 32, data, 37, hmac, &hmac_len);
    
    memcpy(child_key, hmac, 32);
    memcpy(child_chain, hmac + 32, 32);
    
    return true;
}

// ===============================
// SPRAWDZANIE CZY KLUCZ JEST POPRAWNY
// ===============================
inline bool is_valid_private_key(const unsigned char priv[32]) {
    bool all_zero = true;
    for (int i = 0; i < 32; i++) {
        if (priv[i] != 0) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) return false;
    
    static const unsigned char N[32] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
        0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
        0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
    };
    
    for (int i = 0; i < 32; i++) {
        if (priv[i] < N[i]) return true;
        if (priv[i] > N[i]) return false;
    }
    return false;
}

// ===============================
// SKANOWANIE JEDNEGO SEEDA - TYLKO WYBRANE ŚCIEŻKI (0-1)
// ===============================
void scan_seed_fast(const MMapFile& mm, const unsigned char seed[32], const std::string& seed_hex, std::ofstream& found) {
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    
    unsigned char h160[20];
    FastPubCtx fctx;
    fctx.ctx = ctx;
    
    // ==================================================
    // 0. SEED BEZPOŚREDNIO JAKO KLUCZ
    // ==================================================
    if (fast_load_priv(fctx, seed)) {
        // Compressed
        std::string addr_comp = fast_priv_to_address(fctx, true, h160);
        if (contains_address(mm, h160)) {
            std::lock_guard<std::mutex> lock(log_mutex);
            found << "SEED: " << seed_hex << "\nPRIV KEY: " << seed_hex << "\nPATH: direct_compressed\nADDR: " << addr_comp << "\n---\n";
            found.flush();
            total_found++;
            std::cout << "\n✅ ZNALEZIONO! direct compressed -> " << addr_comp << std::endl;
        }
        
        // Uncompressed
        std::string addr_uncomp = fast_priv_to_address(fctx, false, h160);
        if (contains_address(mm, h160)) {
            std::lock_guard<std::mutex> lock(log_mutex);
            found << "SEED: " << seed_hex << "\nPRIV KEY: " << seed_hex << "\nPATH: direct_uncompressed\nADDR: " << addr_uncomp << "\n---\n";
            found.flush();
            total_found++;
            std::cout << "\n✅ ZNALEZIONO! direct uncompressed -> " << addr_uncomp << std::endl;
        }
    }
    
    // ==================================================
    // BIP32 MASTER KEY
    // ==================================================
    unsigned char master_key[32], master_chain[32];
    bip32_master(seed, 32, master_key, master_chain);
    
    if (!is_valid_private_key(master_key)) {
        secp256k1_context_destroy(ctx);
        return;
    }
    
    // ==================================================
    // 1. m/0-1 (TYLKO 0 i 1)
    // ==================================================
    for (uint32_t child = 0; child <= 1; child++) {
        unsigned char child_key[32], child_chain[32];
        if (bip32_derive_normal(ctx, master_key, master_chain, child, child_key, child_chain)) {
            if (fast_load_priv(fctx, child_key)) {
                std::string addr = fast_priv_to_address(fctx, true, h160);
                if (contains_address(mm, h160)) {
                    std::lock_guard<std::mutex> lock(log_mutex);
                    found << "SEED: " << seed_hex << "\nPATH: m/" << child << "\nADDR: " << addr << "\n---\n";
                    found.flush();
                    total_found++;
                    std::cout << "\n✅ ZNALEZIONO! m/" << child << " -> " << addr << std::endl;
                }
            }
        }
    }
    
    // ==================================================
    // 2. BIP44: m/44'/0'/0'/0/0-1 (TYLKO 0 i 1)
    // ==================================================
    unsigned char bip44_key1[32], bip44_chain1[32];
    if (bip32_derive_hardened(master_key, master_chain, 44, bip44_key1, bip44_chain1)) {
        unsigned char bip44_key2[32], bip44_chain2[32];
        if (bip32_derive_hardened(bip44_key1, bip44_chain1, 0, bip44_key2, bip44_chain2)) {
            unsigned char bip44_key3[32], bip44_chain3[32];
            if (bip32_derive_hardened(bip44_key2, bip44_chain2, 0, bip44_key3, bip44_chain3)) {
                unsigned char bip44_key4[32], bip44_chain4[32];
                if (bip32_derive_normal(ctx, bip44_key3, bip44_chain3, 0, bip44_key4, bip44_chain4)) {
                    for (uint32_t child = 0; child <= 1; child++) {
                        unsigned char child_key[32], child_chain[32];
                        if (bip32_derive_normal(ctx, bip44_key4, bip44_chain4, child, child_key, child_chain)) {
                            if (fast_load_priv(fctx, child_key)) {
                                std::string addr = fast_priv_to_address(fctx, true, h160);
                                if (contains_address(mm, h160)) {
                                    std::lock_guard<std::mutex> lock(log_mutex);
                                    found << "SEED: " << seed_hex << "\nPATH: m/44'/0'/0'/0/" << child << "\nADDR: " << addr << "\n---\n";
                                    found.flush();
                                    total_found++;
                                    std::cout << "\n✅ ZNALEZIONO! m/44'/0'/0'/0/" << child << " -> " << addr << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // ==================================================
    // LICZBA KLUCZY: 
    // direct: 2 (C+U)
    // m/0-1: 2
    // BIP44: 2
    // RAZEM: 2 + 2 + 2 = 6 kluczy na seed
    // ==================================================
    total_keys += 6;
    
    secp256k1_context_destroy(ctx);
}

// ===============================
// MONITOR SZYBKOŚCI
// ===============================
void speed_monitor() {
    uint64_t last_keys = 0;
    uint64_t last_seeds = 0;
    
    while (!monitor_stop.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        uint64_t current_keys = total_keys.load();
        uint64_t current_seeds = total_processed.load();
        
        uint64_t keys_diff = current_keys - last_keys;
        double keys_sec = keys_diff / 1000000.0;
        
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "\r⚡ " << std::fixed << std::setprecision(2) << keys_sec << " Mkeys/s"
                  << " | seeds: " << current_seeds
                  << " | keys: " << current_keys
                  << " | found: " << total_found
                  << std::flush;
        
        last_keys = current_keys;
        last_seeds = current_seeds;
    }
}

// ===============================
// PRODUCENT
// ===============================
void producer(const std::string& seed_file) {
    std::ifstream file(seed_file);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć " << seed_file << std::endl;
        finished = true;
        cv.notify_all();
        return;
    }
    
    std::string line;
    uint64_t line_count = 0;
    
    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.length() != 64) continue;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&]{ return seed_queue.size() < QUEUE_SIZE; });
            seed_queue.push(line);
            line_count++;
        }
        cv.notify_all();
    }
    
    finished = true;
    cv.notify_all();
    std::cout << "\n📖 Wczytano " << line_count << " seedów z pliku" << std::endl;
}

// ===============================
// KONSUMENT
// ===============================
void consumer(const MMapFile& mm, const std::string& output_file, int thread_id) {
    std::ofstream found(output_file, std::ios::app);
    
    while (true) {
        std::string seed_hex;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&]{ return !seed_queue.empty() || finished; });
            
            if (seed_queue.empty() && finished) break;
            if (seed_queue.empty()) continue;
            
            seed_hex = seed_queue.front();
            seed_queue.pop();
        }
        cv.notify_all();
        
        unsigned char seed[32];
        for (int i = 0; i < 32; i++) {
            unsigned int byte;
            sscanf(seed_hex.substr(i*2, 2).c_str(), "%02x", &byte);
            seed[i] = (unsigned char)byte;
        }
        
        scan_seed_fast(mm, seed, seed_hex, found);
        total_processed++;
    }
}

// ===============================
// MAIN
// ===============================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Użycie: ./fastbip32 adresy.bin seeds.txt" << std::endl;
        return 1;
    }
    
    try {
        MMapFile mm(argv[1]);
        std::cout << "📁 Wczytano " << mm.count() << " adresów z " << argv[1] << std::endl;
        std::cout << "🧵 Uruchamiam " << THREAD_COUNT << " wątków konsumenckich" << std::endl;
        std::cout << "📦 Kolejka: " << QUEUE_SIZE << " seedów" << std::endl;
        std::cout << "\n🔍 Szukane ścieżki:" << std::endl;
        std::cout << "   - direct (seed as key) compressed/uncompressed" << std::endl;
        std::cout << "   - m/0-1" << std::endl;
        std::cout << "   - BIP44: m/44'/0'/0'/0/0-1" << std::endl;
        std::cout << "\n🚀 Rozpoczynam skanowanie...\n" << std::endl;
        
        start_time = std::chrono::steady_clock::now();
        
        // Uruchom monitor
        monitor_stop = false;
        std::thread monitor(speed_monitor);
        
        // Uruchom producenta
        std::thread prod(producer, std::string(argv[2]));
        
        // Uruchom konsumentów
        std::vector<std::thread> consumers;
        for (int i = 0; i < THREAD_COUNT; i++) {
            consumers.emplace_back(consumer, std::cref(mm), std::string("found.txt"), i);
        }
        
        prod.join();
        for (auto& t : consumers) {
            t.join();
        }
        
        monitor_stop = true;
        monitor.join();
        
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        double seconds = std::chrono::duration<double>(elapsed).count();
        
        std::cout << "\n\n✅ Skanowanie zakończone!" << std::endl;
        std::cout << "   Przetworzono: " << total_processed << " seedów" << std::endl;
        std::cout << "   Wygenerowano: " << total_keys << " kluczy" << std::endl;
        std::cout << "   Znaleziono: " << total_found << " adresów" << std::endl;
        std::cout << "   Czas: " << std::fixed << std::setprecision(2) << seconds << " s" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Błąd: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
