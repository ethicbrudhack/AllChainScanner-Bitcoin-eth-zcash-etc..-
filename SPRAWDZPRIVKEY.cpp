// ===============================
// FAST BIP32 SCANNER v15 - WSZYSTKIE MONETY
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
// COIN DEFINITIONS
// ===============================
enum CoinType {
    COIN_BITCOIN = 0,
    COIN_LITECOIN = 2,
    COIN_DOGECOIN = 3,
    COIN_DASH = 5,
    COIN_ETHEREUM = 60,
    COIN_ZCASH = 133
};

struct CoinInfo {
    std::string name;
    uint32_t bip44_type;
    uint8_t p2pkh_version;
    uint8_t p2sh_version;
    std::string bech32_hrp;
    bool uses_ethereum_format;
    bool uses_zcash_format;
    bool only_p2pkh;
};

std::map<CoinType, CoinInfo> coin_map = {
    {COIN_BITCOIN, {"Bitcoin", 0, 0x00, 0x05, "bc", false, false, false}},
    {COIN_LITECOIN, {"Litecoin", 2, 0x30, 0x32, "ltc", false, false, false}},
    {COIN_DOGECOIN, {"Dogecoin", 3, 0x1E, 0x16, "doge", false, false, true}},
    {COIN_DASH, {"Dash", 5, 0x4C, 0x10, "dash", false, false, true}},
    {COIN_ETHEREUM, {"Ethereum", 60, 0x00, 0x00, "", true, false, false}},
    {COIN_ZCASH, {"Zcash", 133, 0x1C, 0x1D, "zcash", false, true, false}}
};

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

inline void pubkey_hash160(const unsigned char* pub, size_t len, unsigned char out[20]) {
    unsigned char sh[32];
    sha256_once(pub, len, sh);
    ripemd160_once(sh, 32, out);
}

// ===============================
// BASE58
// ===============================
static const char* BASE58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string base58_encode(const std::vector<unsigned char>& in) {
    BIGNUM* bn = BN_new();
    BN_bin2bn(in.data(), in.size(), bn);

    BIGNUM *dv = BN_new(), *rem = BN_new(), *b58 = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    BN_set_word(b58, 58);

    std::string out;
    while (!BN_is_zero(bn)) {
        BN_div(dv, rem, bn, b58, ctx);
        out.insert(out.begin(), BASE58[BN_get_word(rem)]);
        BN_copy(bn, dv);
    }

    for (unsigned char c : in)
        if (c == 0x00) out.insert(out.begin(), '1');
        else break;

    BN_free(bn); BN_free(dv); BN_free(rem); BN_free(b58); BN_CTX_free(ctx);
    return out;
}

std::string base58_check_encode(const unsigned char* data, size_t len) {
    std::vector<unsigned char> extended(data, data + len);
    unsigned char hash[32];
    sha256_once(data, len, hash);
    sha256_once(hash, 32, hash);
    extended.insert(extended.end(), hash, hash + 4);
    return base58_encode(extended);
}

std::string ripemd_to_base58(const unsigned char ripe[20], uint8_t version) {
    std::vector<unsigned char> ext;
    ext.push_back(version);
    ext.insert(ext.end(), ripe, ripe+20);
    return base58_check_encode(ext.data(), ext.size());
}

std::string zcash_to_base58(const unsigned char ripe[20], uint8_t version1, uint8_t version2) {
    std::vector<unsigned char> ext;
    ext.push_back(version1);
    ext.push_back(version2);
    ext.insert(ext.end(), ripe, ripe+20);
    return base58_check_encode(ext.data(), ext.size());
}

// ===============================
// BECH32
// ===============================
const std::string BECH32_CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

uint32_t bech32_polymod(const std::vector<uint8_t>& values) {
    uint32_t chk = 1;
    const uint32_t GEN[] = {0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3};
    
    for (uint8_t v : values) {
        uint32_t top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ v;
        for (size_t i = 0; i < 5; ++i) {
            if ((top >> i) & 1) {
                chk ^= GEN[i];
            }
        }
    }
    return chk;
}

std::vector<uint8_t> bech32_hrp_expand(const std::string& hrp) {
    std::vector<uint8_t> ret;
    for (char c : hrp) {
        ret.push_back((uint8_t)(c >> 5));
    }
    ret.push_back(0);
    for (char c : hrp) {
        ret.push_back((uint8_t)(c & 0x1f));
    }
    return ret;
}

std::vector<uint8_t> convert_bits(const uint8_t* data, size_t len, 
                                  size_t from_bits, size_t to_bits, 
                                  bool pad) {
    std::vector<uint8_t> ret;
    uint64_t acc = 0;
    size_t bits = 0;
    const uint64_t max_mask = (1 << to_bits) - 1;
    
    for (size_t i = 0; i < len; ++i) {
        acc = (acc << from_bits) | data[i];
        bits += from_bits;
        while (bits >= to_bits) {
            bits -= to_bits;
            ret.push_back((acc >> bits) & max_mask);
        }
    }
    
    if (pad && bits > 0) {
        ret.push_back((acc << (to_bits - bits)) & max_mask);
    }
    
    return ret;
}

std::string bech32_encode(const std::string& hrp, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> values;
    auto hrp_exp = bech32_hrp_expand(hrp);
    values.insert(values.end(), hrp_exp.begin(), hrp_exp.end());
    values.insert(values.end(), data.begin(), data.end());
    
    for (int i = 0; i < 6; ++i) {
        values.push_back(0);
    }
    
    uint32_t polymod = bech32_polymod(values) ^ 1;
    
    std::vector<uint8_t> data_with_checksum = data;
    for (int i = 0; i < 6; ++i) {
        data_with_checksum.push_back((polymod >> (5 * (5 - i))) & 0x1F);
    }
    
    std::string result = hrp + "1";
    for (uint8_t v : data_with_checksum) {
        result += BECH32_CHARSET[v];
    }
    
    return result;
}

// ===============================
// BIP32 FUNKCJE
// ===============================
struct ExtendedKey {
    unsigned char priv[32];
    unsigned char chain[32];
    uint32_t depth;
    uint32_t fingerprint;
    uint32_t index;
};

void hmac_sha512(const unsigned char* key, size_t key_len,
                 const unsigned char* data, size_t data_len,
                 unsigned char out[64]) {
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key, key_len, EVP_sha512(), NULL);
    HMAC_Update(ctx, data, data_len);
    HMAC_Final(ctx, out, NULL);
    HMAC_CTX_free(ctx);
}

void bip32_master_from_seed(const unsigned char seed[64], ExtendedKey& key) {
    unsigned char hmac_out[64];
    hmac_sha512((const unsigned char*)"Bitcoin seed", 12, seed, 64, hmac_out);
    memcpy(key.priv, hmac_out, 32);
    memcpy(key.chain, hmac_out + 32, 32);
    key.depth = 0;
    key.fingerprint = 0;
    key.index = 0;
}

void bip32_derive_child(secp256k1_context* ctx,
                        const ExtendedKey& parent,
                        uint32_t index,
                        ExtendedKey& child) {
    unsigned char data[37];
    unsigned char hmac_out[64];
    
    if (index & 0x80000000) {
        data[0] = 0x00;
        memcpy(data + 1, parent.priv, 32);
        uint32_t be_index = htonl(index);
        memcpy(data + 33, &be_index, 4);
    } else {
        secp256k1_pubkey pub;
        if (!secp256k1_ec_pubkey_create(ctx, &pub, parent.priv)) {
            memset(&child, 0, sizeof(child));
            return;
        }
        unsigned char pub_ser[33];
        size_t pub_len = 33;
        secp256k1_ec_pubkey_serialize(ctx, pub_ser, &pub_len, &pub,
                                      SECP256K1_EC_COMPRESSED);
        memcpy(data, pub_ser, 33);
        uint32_t be_index = htonl(index);
        memcpy(data + 33, &be_index, 4);
    }
    
    hmac_sha512(parent.chain, 32, data, 37, hmac_out);
    
    BIGNUM* bn_il = BN_new();
    BIGNUM* bn_n = BN_new();
    BN_hex2bn(&bn_n, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
    BN_bin2bn(hmac_out, 32, bn_il);
    
    if (BN_cmp(bn_il, bn_n) >= 0 || BN_is_zero(bn_il)) {
        memset(&child, 0, sizeof(child));
        BN_free(bn_il);
        BN_free(bn_n);
        return;
    }
    
    BIGNUM* bn_parent = BN_new();
    BIGNUM* bn_result = BN_new();
    BN_CTX* bn_ctx = BN_CTX_new();
    
    BN_bin2bn(parent.priv, 32, bn_parent);
    BN_add(bn_parent, bn_parent, bn_il);
    BN_mod(bn_result, bn_parent, bn_n, bn_ctx);
    
    if (BN_is_zero(bn_result)) {
        memset(&child, 0, sizeof(child));
        BN_free(bn_parent);
        BN_free(bn_result);
        BN_CTX_free(bn_ctx);
        BN_free(bn_il);
        BN_free(bn_n);
        return;
    }
    
    memset(child.priv, 0, 32);
    BN_bn2binpad(bn_result, child.priv, 32);
    memcpy(child.chain, hmac_out + 32, 32);
    
    child.depth = parent.depth + 1;
    child.index = index;
    
    secp256k1_pubkey pub;
    if (secp256k1_ec_pubkey_create(ctx, &pub, parent.priv)) {
        unsigned char pub_ser[33];
        size_t pub_len = 33;
        secp256k1_ec_pubkey_serialize(ctx, pub_ser, &pub_len, &pub,
                                      SECP256K1_EC_COMPRESSED);
        unsigned char hash160[20];
        pubkey_hash160(pub_ser, pub_len, hash160);
        child.fingerprint = (hash160[0] << 24) | (hash160[1] << 16) | 
                            (hash160[2] << 8) | hash160[3];
    } else {
        child.fingerprint = 0;
    }
    
    BN_free(bn_parent);
    BN_free(bn_result);
    BN_CTX_free(bn_ctx);
    BN_free(bn_il);
    BN_free(bn_n);
}

ExtendedKey derive_path(secp256k1_context* ctx,
                        const ExtendedKey& master,
                        const std::string& path) {
    ExtendedKey current = master;
    std::vector<std::string> segments;
    std::stringstream ss(path);
    std::string segment;
    
    while (std::getline(ss, segment, '/')) {
        if (!segment.empty() && segment != "m") {
            bool hardened = (segment.back() == '\'');
            uint32_t index;
            if (hardened) {
                index = std::stoul(segment.substr(0, segment.length() - 1)) + 0x80000000;
            } else {
                index = std::stoul(segment);
            }
            ExtendedKey child;
            bip32_derive_child(ctx, current, index, child);
            current = child;
        }
    }
    return current;
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

bool contains_address(const MMapFile& mm, const unsigned char addr20[20]) {
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
// TEST DLA PRIVATE KEY = 1 (WSZYSTKIE MONETY!)
// ===============================
void test_private_key_one() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     🔑 TEST DLA PRIVATE KEY = 1 (WSZYSTKIE MONETY)         ║\n";
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
    secp256k1_ec_pubkey_create(ctx, &pub, priv);
    
    unsigned char pub_ser[33];
    size_t pub_len = 33;
    secp256k1_ec_pubkey_serialize(ctx, pub_ser, &pub_len, &pub, SECP256K1_EC_COMPRESSED);
    
    unsigned char h160[20];
    pubkey_hash160(pub_ser, pub_len, h160);
    
    // ==========================================
    // 1. BITCOIN P2PKH (1...)
    // ==========================================
    std::cout << "💰 Bitcoin P2PKH (1...): ";
    std::string addr = ripemd_to_base58(h160, 0x00);
    std::cout << addr << "\n";
    std::cout << "  Oczekiwany: 1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm\n";
    std::cout << "  " << (addr == "1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 2. BITCOIN P2SH (3...)
    // ==========================================
    unsigned char redeem_script[22];
    redeem_script[0] = 0x00;
    redeem_script[1] = 0x14;
    memcpy(redeem_script + 2, h160, 20);
    unsigned char redeem_hash[20];
    pubkey_hash160(redeem_script, 22, redeem_hash);
    
    std::cout << "💰 Bitcoin P2SH (3...): ";
    std::string addr_p2sh = ripemd_to_base58(redeem_hash, 0x05);
    std::cout << addr_p2sh << "\n";
    std::cout << "  Oczekiwany: 3JvL6Ymt8MVWiCNHC7oWU6nLeHNJKLZGLN\n";
    std::cout << "  " << (addr_p2sh == "3JvL6Ymt8MVWiCNHC7oWU6nLeHNJKLZGLN" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 3. BITCOIN SEGWIT (bc1...)
    // ==========================================
    std::vector<uint8_t> five_bit;
    five_bit.push_back(0);
    auto converted = convert_bits(h160, 20, 8, 5, true);
    five_bit.insert(five_bit.end(), converted.begin(), converted.end());
    std::string addr_segwit = bech32_encode("bc", five_bit);
    std::cout << "💰 Bitcoin SegWit (bc1...): " << addr_segwit << "\n";
    std::cout << "  Oczekiwany: bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4\n";
    std::cout << "  " << (addr_segwit == "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 4. LITECOIN P2PKH (L...)
    // ==========================================
    std::cout << "💰 Litecoin P2PKH (L...): ";
    std::string addr_ltc = ripemd_to_base58(h160, 0x30);
    std::cout << addr_ltc << "\n";
    std::cout << "  Oczekiwany: LYWKqJhtPeGyBAw7WC8R3F7ovxtzAiubdM\n";
    std::cout << "  " << (addr_ltc == "LYWKqJhtPeGyBAw7WC8R3F7ovxtzAiubdM" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 5. LITECOIN P2SH (M...)
    // ==========================================
    std::cout << "💰 Litecoin P2SH (M...): ";
    std::string addr_ltc_p2sh = ripemd_to_base58(redeem_hash, 0x32);
    std::cout << addr_ltc_p2sh << "\n";
    std::cout << "  Oczekiwany: MR8UQSBr5ULwWheBHznrHk2jxyxkHQu8vB\n";
    std::cout << "  " << (addr_ltc_p2sh == "MR8UQSBr5ULwWheBHznrHk2jxyxkHQu8vB" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 6. LITECOIN SEGWIT (ltc1...)
    // ==========================================
    std::string addr_ltc_segwit = bech32_encode("ltc", five_bit);
    std::cout << "💰 Litecoin SegWit (ltc1...): " << addr_ltc_segwit << "\n";
    std::cout << "  Oczekiwany: ltc1qw508d6qejxtdg4y5r3zarvary0c5xw7kgmn4n9\n";
    std::cout << "  " << (addr_ltc_segwit == "ltc1qw508d6qejxtdg4y5r3zarvary0c5xw7kgmn4n9" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 7. DOGECOIN (D...)
    // ==========================================
    std::cout << "💰 Dogecoin P2PKH (D...): ";
    std::string addr_doge = ripemd_to_base58(h160, 0x1E);
    std::cout << addr_doge << "\n";
    std::cout << "  Oczekiwany: DJRU7MLhcPwCTNRZ4e8gJzDebtG1H5M7pc\n";
    std::cout << "  " << (addr_doge == "DJRU7MLhcPwCTNRZ4e8gJzDebtG1H5M7pc" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 8. DASH (X...)
    // ==========================================
    std::cout << "💰 Dash P2PKH (X...): ";
    std::string addr_dash = ripemd_to_base58(h160, 0x4C);
    std::cout << addr_dash << "\n";
    std::cout << "  Oczekiwany: XoyDQM3xGhFW5JqYBwTLckjqZ67Q3jZfAL\n";
    std::cout << "  " << (addr_dash == "XoyDQM3xGhFW5JqYBwTLckjqZ67Q3jZfAL" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 9. ZCASH t1...
    // ==========================================
    std::cout << "💰 Zcash P2PKH (t1...): ";
    std::string addr_zec = zcash_to_base58(h160, 0x1C, 0xB8);
    std::cout << addr_zec << "\n";
    std::cout << "  Oczekiwany: t1UYsZVJkLPeMjxEtACvSxfWuNmddpWfxzs\n";
    std::cout << "  " << (addr_zec == "t1UYsZVJkLPeMjxEtACvSxfWuNmddpWfxzs" ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 10. ZCASH t3...
    // ==========================================
    unsigned char zec_redeem[22];
    zec_redeem[0] = 0x00;
    zec_redeem[1] = 0x14;
    memcpy(zec_redeem + 2, h160, 20);
    unsigned char zec_redeem_hash[20];
    pubkey_hash160(zec_redeem, 22, zec_redeem_hash);
    
    std::cout << "💰 Zcash P2SH (t3...): ";
    std::string addr_zec_p2sh = zcash_to_base58(zec_redeem_hash, 0x1C, 0xBD);
    std::cout << addr_zec_p2sh << "\n";
    std::cout << "  Oczekiwany: t3...\n";
    std::cout << "  " << (addr_zec_p2sh == "t3..." ? "✅ OK" : "❌ BŁĄD") << "\n\n";
    
    // ==========================================
    // 11. ETHEREUM
    // ==========================================
    unsigned char pub_ser_uncomp[65];
    pub_len = 65;
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
}// ===============================
// GŁÓWNE SKANOWANIE
// ===============================
void scan_seed_direct(const MMapFile& mm, const unsigned char priv[32], const std::string& priv_hex, std::ofstream& found) {
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    
    secp256k1_pubkey pub;
    if (!secp256k1_ec_pubkey_create(ctx, &pub, priv)) {
        secp256k1_context_destroy(ctx);
        return;
    }
    
    unsigned char pub_ser[33];
    size_t pub_len = 33;
    secp256k1_ec_pubkey_serialize(ctx, pub_ser, &pub_len, &pub, SECP256K1_EC_COMPRESSED);
    
    unsigned char h160[20];
    pubkey_hash160(pub_ser, pub_len, h160);
    
    // Sprawdź P2PKH dla wszystkich coinów
    if (contains_address(mm, h160)) {
        std::string addr = ripemd_to_base58(h160, 0x00);
        std::lock_guard<std::mutex> lock(log_mutex);
        found << "PRIV: " << priv_hex << "\n";
        found << "PATH: direct_compressed\n";
        found << "ADDR: " << addr << "\n---\n";
        found.flush();
        total_found++;
        std::cout << "\n✅ ZNALEZIONO! " << addr << std::endl;
    }
    
    // P2SH
    unsigned char redeem_script[22];
    redeem_script[0] = 0x00;
    redeem_script[1] = 0x14;
    memcpy(redeem_script + 2, h160, 20);
    unsigned char redeem_hash[20];
    pubkey_hash160(redeem_script, 22, redeem_hash);
    
    if (contains_address(mm, redeem_hash)) {
        std::string addr = ripemd_to_base58(redeem_hash, 0x05);
        std::lock_guard<std::mutex> lock(log_mutex);
        found << "PRIV: " << priv_hex << "\n";
        found << "PATH: direct_p2sh\n";
        found << "ADDR: " << addr << "\n---\n";
        found.flush();
        total_found++;
        std::cout << "\n✅ ZNALEZIONO! " << addr << std::endl;
    }
    
    // SegWit
    std::vector<uint8_t> five_bit;
    five_bit.push_back(0);
    auto converted = convert_bits(h160, 20, 8, 5, true);
    five_bit.insert(five_bit.end(), converted.begin(), converted.end());
    std::string segwit_addr = bech32_encode("bc", five_bit);
    
    if (contains_address(mm, h160)) {
        std::lock_guard<std::mutex> lock(log_mutex);
        found << "PRIV: " << priv_hex << "\n";
        found << "PATH: direct_segwit\n";
        found << "ADDR: " << segwit_addr << "\n---\n";
        found.flush();
        total_found++;
        std::cout << "\n✅ ZNALEZIONO! " << segwit_addr << std::endl;
    }
    
    // Ethereum
    unsigned char pub_ser_uncomp[65];
    pub_len = 65;
    secp256k1_ec_pubkey_serialize(ctx, pub_ser_uncomp, &pub_len, &pub, SECP256K1_EC_UNCOMPRESSED);
    
    unsigned char keccak_hash[32];
    keccak_256_fixed(pub_ser_uncomp + 1, 64, keccak_hash);
    
    unsigned char eth_hash[20];
    memcpy(eth_hash, keccak_hash + 12, 20);
    
    if (contains_address(mm, eth_hash)) {
        std::string eth_addr = "0x";
        for (int i = 12; i < 32; i++) {
            char hex[3];
            sprintf(hex, "%02x", keccak_hash[i]);
            eth_addr += hex;
        }
        std::lock_guard<std::mutex> lock(log_mutex);
        found << "PRIV: " << priv_hex << "\n";
        found << "PATH: direct_eth\n";
        found << "ADDR: " << eth_addr << "\n---\n";
        found.flush();
        total_found++;
        std::cout << "\n✅ ZNALEZIONO! " << eth_addr << std::endl;
    }
    
    secp256k1_context_destroy(ctx);
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
void consumer(const MMapFile& mm, const std::string& output_file) {
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
        
        scan_seed_direct(mm, priv, priv_hex, found);
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
    // ZAWSZE NA POCZĄTKU - TEST PRIVATE KEY = 1
    // ==========================================
    test_private_key_one();
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        ✅ KONIEC TESTU                                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    // ==========================================
    // NORMALNE SKANOWANIE (JEŚLI PODANO ARGUMENTY)
    // ==========================================
    if (argc < 3) {
        std::cout << "📖 Użycie: ./program adresy.bin keys.txt\n";
        std::cout << "   keys.txt - lista kluczy prywatnych (64 hex, po jednym w linii)\n";
        return 0;
    }
    
    try {
        MMapFile mm(argv[1]);
        std::cout << "📁 Wczytano " << mm.count() << " adresów z " << argv[1] << std::endl;
        std::cout << "🧵 Uruchamiam " << THREAD_COUNT << " wątków" << std::endl;
        std::cout << "\n🔍 Obsługiwane monety:" << std::endl;
        for (auto& coin_pair : coin_map) {
            std::cout << "   - " << coin_pair.second.name << std::endl;
        }
        std::cout << "\n🚀 Rozpoczynam skanowanie..." << std::endl;
        
        std::thread monitor(speed_monitor);
        std::thread prod(producer, std::string(argv[2]));
        
        std::vector<std::thread> consumers;
        for (int i = 0; i < THREAD_COUNT; i++) {
            consumers.emplace_back(consumer, std::cref(mm), std::string("found.txt"));
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