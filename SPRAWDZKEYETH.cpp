// ===============================
// FAST ETH SCANNER - TYLKO ETHEREUM!
// Z 24-BITOWYM INDEKSEM (128 MB) - SZYBSZY!
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
#include <map>
#include <sstream>
#include <arpa/inet.h>
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
#include "keccak_fixed.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// ===============================
// PARAMETRY
// ===============================
static const int THREAD_COUNT = 60;
static const size_t QUEUE_SIZE = 200000;

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
// HASH FUNCTIONS
// ===============================
inline void sha256_once(const unsigned char* d, size_t n, unsigned char out[32]) {
    SHA256_CTX c;
    SHA256_Init(&c);
    SHA256_Update(&c, d, n);
    SHA256_Final(out, &c);
}

inline void ripemd160_once(const unsigned char* d, size_t n, unsigned char out[20]) {
    RIPEMD160_CTX r;
    RIPEMD160_Init(&r);
    RIPEMD160_Update(&r, d, n);
    RIPEMD160_Final(out, &r);
}

// ===============================
// MAPOWANIE PLIKU
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
            throw std::runtime_error("invalid bin file");

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

// ===============================
// 24-BITOWY INDEKS PREFIKSOWY (128 MB)
// ===============================
class PrefixIndex24 {
private:
    const MMapFile& mm;
    std::vector<uint64_t> index;
    bool ready;

    static inline uint32_t get_prefix24(const unsigned char* p) {
        return (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8) | uint32_t(p[2]);
    }

public:
    PrefixIndex24(const MMapFile& m) : mm(m), ready(false) {
        index.resize(16777217);
        build();
    }

    void build() {
        const unsigned char* base = mm.ptr();
        uint64_t count = mm.count();
        
        std::cout << "📦 Budowanie 24-bitowego indeksu dla " << count << " adresów...\n";
        std::cout << "📊 Rozmiar indeksu: ~" << (index.size() * sizeof(uint64_t)) / (1024*1024) << " MB\n";
        
        auto start_time = std::chrono::steady_clock::now();

        uint64_t pos = 0;
        uint64_t buckets_found = 0;

        while (pos < count) {
            const unsigned char* rec = base + pos * 20;
            uint32_t p = get_prefix24(rec);
            
            index[p] = pos;
            
            uint64_t lo = pos;
            uint64_t hi = count;
            
            while (lo + 1 < hi) {
                uint64_t mid = (lo + hi) / 2;
                const unsigned char* mid_rec = base + mid * 20;
                uint32_t mp = get_prefix24(mid_rec);
                
                if (mp <= p)
                    lo = mid;
                else
                    hi = mid;
            }
            
            index[p + 1] = lo + 1;
            
            pos = lo + 1;
            buckets_found++;
            
            if (buckets_found % 1000 == 0) {
                std::cout << "\r   Przetworzono: " << pos << "/" << count 
                          << " rekordów | Znaleziono: " << buckets_found 
                          << " prefiksów" << std::flush;
            }
        }
        
        uint64_t last_start = 0;
        uint64_t last_end = count;
        
        for (int i = 16777215; i >= 0; i--) {
            if (index[i] != 0 || i == 0) {
                last_start = index[i];
                last_end = index[i + 1];
            } else {
                index[i] = last_start;
                index[i + 1] = last_end;
            }
        }

        auto end_time = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration<double>(end_time - start_time).count();
        
        std::cout << "\n✅ Indeks zbudowany: " << buckets_found << "/16777216 prefiksów używanych\n";
        std::cout << "⏱️  Czas budowy: " << std::fixed << std::setprecision(1) << seconds << " s\n";
        std::cout << "📊 Pamięć indeksu: ~" << (index.size() * sizeof(uint64_t)) / (1024*1024) << " MB\n";
        
        ready = true;
    }

    // ==========================================
    // contains() z 24-bitowym indeksem - SZYBSZY!
    // ==========================================
    bool contains(const unsigned char addr20[20]) const {
        uint32_t p = get_prefix24(addr20);
        
        uint64_t lo = index[p];
        uint64_t hi = index[p + 1];
        
        if (lo >= hi) {
            return false;
        }
        
        const unsigned char* base = mm.ptr();
        
        while (lo < hi) {
            uint64_t mid = (lo + hi) / 2;
            const unsigned char* midp = base + mid * 20;

            int cmp = memcmp(midp, addr20, 20);
            if (cmp == 0) return true;
            if (cmp < 0) lo = mid + 1;
            else hi = mid;
        }
        return false;
    }
};

// ===============================
// TEST DLA PRIVATE KEY = 1 (ETH)
// ===============================
void test_eth_private_key_one() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     🔑 TEST ETH DLA PRIVATE KEY = 1                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    
    unsigned char priv[32] = {0};
    priv[31] = 1;
    
    std::cout << "Private Key (hex): ";
    for (int i = 0; i < 32; i++) {
        printf("%02x", priv[i]);
    }
    printf("\n\n");
    
    secp256k1_pubkey pub;
    if (!secp256k1_ec_pubkey_create(ctx, &pub, priv)) {
        std::cerr << "Failed to create pubkey\n";
        secp256k1_context_destroy(ctx);
        return;
    }
    
    unsigned char pub_ser_uncomp[65];
    size_t pub_len = 65;
    secp256k1_ec_pubkey_serialize(ctx, pub_ser_uncomp, &pub_len, &pub, SECP256K1_EC_UNCOMPRESSED);
    
    unsigned char keccak_hash[32];
    keccak_256_fixed(pub_ser_uncomp + 1, 64, keccak_hash);
    
    std::cout << "💰 Ethereum (0x...): 0x";
    for (int i = 12; i < 32; i++) {
        printf("%02x", keccak_hash[i]);
    }
    printf("\n");
    std::cout << "  Oczekiwany: 0x7E5F4552091A69125d5DfCb7b8C2659029395Bdf\n";
    
    char eth_addr[43];
    strcpy(eth_addr, "0x");
    for (int i = 12; i < 32; i++) {
        char hex[3];
        sprintf(hex, "%02x", keccak_hash[i]);
        strcat(eth_addr, hex);
    }
    std::cout << "  " << (strcasecmp(eth_addr, "0x7E5F4552091A69125d5DfCb7b8C2659029395Bdf") == 0 ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    secp256k1_context_destroy(ctx);
}

// ===============================
// GENEROWANIE ETH ADRESU
// ===============================
inline std::string generate_eth_address(secp256k1_context* ctx, const unsigned char priv[32]) {
    secp256k1_pubkey pub;
    if (!secp256k1_ec_pubkey_create(ctx, &pub, priv)) {
        return "";
    }
    
    unsigned char pub_ser[65];
    size_t pub_len = 65;
    secp256k1_ec_pubkey_serialize(ctx, pub_ser, &pub_len, &pub, SECP256K1_EC_UNCOMPRESSED);
    
    unsigned char keccak_hash[32];
    keccak_256_fixed(pub_ser + 1, 64, keccak_hash);
    
    std::string result = "0x";
    for (int i = 12; i < 32; i++) {
        char hex[3];
        sprintf(hex, "%02x", keccak_hash[i]);
        result += hex;
    }
    return result;
}

// ===============================
// SKANOWANIE - TYLKO ETH! Z 24-BITOWYM INDEKSEM
// ===============================
void scan_seed_eth(secp256k1_context* ctx, const PrefixIndex24& idx, 
                    const unsigned char priv[32], const std::string& priv_hex, 
                    std::ofstream& found) {
    
    secp256k1_pubkey pub;
    if (!secp256k1_ec_pubkey_create(ctx, &pub, priv)) {
        return;
    }
    
    // ==========================================
    // ETH - TYLKO JEDEN HASH!
    // ==========================================
    unsigned char pub_eth[65];
    size_t pub_len = 65;
    secp256k1_ec_pubkey_serialize(ctx, pub_eth, &pub_len, &pub, SECP256K1_EC_UNCOMPRESSED);
    
    unsigned char keccak_hash[32];
    keccak_256_fixed(pub_eth + 1, 64, keccak_hash);
    
    unsigned char eth_hash[20];
    memcpy(eth_hash, keccak_hash + 12, 20);
    total_keys++;
    
    // ==========================================
    // SPRAWDŹ TYLKO ETH! (używając indeksu)
    // ==========================================
    if (idx.contains(eth_hash)) {
        std::string eth_addr = "0x";
        for (int i = 12; i < 32; i++) {
            char hex[3];
            sprintf(hex, "%02x", keccak_hash[i]);
            eth_addr += hex;
        }
        
        std::lock_guard<std::mutex> lock(log_mutex);
        found << "PRIV: " << priv_hex << "\n";
        found << "ETH: " << eth_addr << "\n---\n";
        found.flush();
        total_found++;
        std::cout << "\n✅ ZNALEZIONO ETH: " << eth_addr << std::endl;
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
    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.length() != 64) continue;
        if (line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) continue;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&]{ return seed_queue.size() < QUEUE_SIZE; });
            seed_queue.push(line);
        }
        cv.notify_all();
    }
    
    finished = true;
    cv.notify_all();
}

// ===============================
// KONSUMENT
// ===============================
void consumer(const PrefixIndex24& idx, const std::string& output_file) {
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    std::ofstream found(output_file, std::ios::app);
    
    while (true) {
        std::string priv_hex;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&]{ return !seed_queue.empty() || finished; });
            
            if (seed_queue.empty() && finished) break;
            if (seed_queue.empty()) continue;
            
            priv_hex = seed_queue.front();
            seed_queue.pop();
        }
        cv.notify_all();
        
        unsigned char priv[32];
        for (int i = 0; i < 32; i++) {
            unsigned int byte;
            sscanf(priv_hex.substr(i*2, 2).c_str(), "%02x", &byte);
            priv[i] = (unsigned char)byte;
        }
        
        scan_seed_eth(ctx, idx, priv, priv_hex, found);
        total_processed++;
    }
    
    secp256k1_context_destroy(ctx);
}

// ===============================
// SPEED MONITOR
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
// MAIN
// ===============================
int main(int argc, char* argv[]) {
    
    // ==========================================
    // ZAWSZE NA POCZĄTKU - TEST PRIVATE KEY = 1 (ETH)
    // ==========================================
    test_eth_private_key_one();
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        ✅ KONIEC TESTU                                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    // ==========================================
    // NORMALNE SKANOWANIE (JEŚLI PODANO ARGUMENTY)
    // ==========================================
    if (argc < 3) {
        std::cout << "📖 Użycie: ./ethscan adresy.bin keys.txt\n";
        std::cout << "   keys.txt - lista kluczy prywatnych (64 hex, po jednym w linii)\n";
        return 0;
    }
    
    try {
        MMapFile mm(argv[1]);
        std::cout << "📁 Wczytano " << mm.count() << " adresów z " << argv[1] << std::endl;
        
        // ==========================================
        // 24-BITOWY INDEKS (128 MB) - SZYBSZY!
        // ==========================================
        PrefixIndex24 idx(mm);
        
        std::cout << "🧵 Uruchamiam " << THREAD_COUNT << " wątków" << std::endl;
        std::cout << "\n🔍 Obsługiwana moneta: TYLKO ETHEREUM!" << std::endl;
        std::cout << "⚡ Używam 24-bitowego indeksu (128 MB) - SZYBSZY!\n";
        std::cout << "\n🚀 Rozpoczynam skanowanie..." << std::endl;
        
        std::thread monitor(speed_monitor);
        std::thread prod(producer, std::string(argv[2]));
        
        std::vector<std::thread> consumers;
        for (int i = 0; i < THREAD_COUNT; i++) {
            consumers.emplace_back(consumer, std::cref(idx), std::string("found.txt"));
        }
        
        prod.join();
        for (auto& t : consumers) {
            t.join();
        }
        
        monitor_stop = true;
        monitor.join();
        
        std::cout << "\n\n✅ Skanowanie zakończone!" << std::endl;
        std::cout << "   Przetworzono: " << total_processed << " kluczy" << std::endl;
        std::cout << "   Wygenerowano: " << total_keys << " adresów" << std::endl;
        std::cout << "   Znaleziono: " << total_found << " trafień" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Błąd: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}