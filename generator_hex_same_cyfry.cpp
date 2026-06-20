// ============================================================
// GENERATOR HEX Z WIODĄCYMI ZERAMI (50-256 bitów) - TYLKO CYFRY
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <random>
#include <sstream>

using namespace std;

static const size_t BUFFER_SIZE = 1024 * 1024 * 100; // 100 MB

// ================== PARAMETRY ==================
static const char CHARS[] = "0123456789";  // TYLKO CYFRY!
const int SEED_LENGTH = 64;        // 64 znaki hex
// ==============================================

static string temp_random_part;
static string temp_result;

string generate_seed_with_entropy(int entropy_bits) {
    // entropy_bits = liczba bitów entropii (np. 50, 256)
    
    static random_device rd;
    static mt19937_64 gen(rd());
    static uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    
    // 1. Oblicz ile znaków potrzeba na entropię
    // Każdy znak to 4 bity (ale tylko cyfry 0-9)
    int chars_needed = (entropy_bits + 3) / 4;
    
    // 2. Wygeneruj losowe cyfry (0-9)
    temp_random_part.clear();
    temp_random_part.reserve(chars_needed);
    
    uint64_t value = dist(gen);
    int bits_generated = 0;
    
    while (temp_random_part.length() < chars_needed) {
        // Bierzemy 4 bity (0-15) ale tylko 0-9 są dozwolone
        int nibble = value & 0xF;
        if (nibble < 10) {  // TYLKO CYFRY 0-9!
            temp_random_part += CHARS[nibble];
        }
        // Jeśli nibble >= 10 (A-F) - pomijamy!
        value >>= 4;
        bits_generated += 4;
        
        if (bits_generated >= 64 && temp_random_part.length() < chars_needed) {
            value = dist(gen);
            bits_generated = 0;
        }
    }
    
    // 3. Oblicz ile zer z przodu
    int zeros_count = SEED_LENGTH - chars_needed;
    
    // 4. Złóż całość: zera + losowe cyfry
    temp_result.clear();
    temp_result.append(zeros_count, '0');
    temp_result += temp_random_part;
    
    // 5. Upewnij się, że długość to 64
    if (temp_result.length() < SEED_LENGTH) {
        temp_result = string(SEED_LENGTH - temp_result.length(), '0') + temp_result;
    }
    
    return temp_result;
}

void generate_seeds(const string& filename, 
                    uint64_t min_bits,
                    uint64_t max_bits,
                    uint64_t count) {
    
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "❌ Nie można otworzyć pliku\n";
        return;
    }

    string buffer;
    buffer.reserve(BUFFER_SIZE);
    
    auto start = chrono::steady_clock::now();
    
    cout << "\n🚀 Generuję " << count << " seedów hex\n";
    cout << "   Długość: 64 znaki\n";
    cout << "   Entropia: " << min_bits << " - " << max_bits << " bitów\n";
    cout << "   Format: [zera] + [losowe CYFRY 0-9]\n";
    cout << "   Losowych cyfr: ~" << (min_bits/4) << " - " << (max_bits/4) << "\n\n";
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> bit_dist(min_bits, max_bits);
    
    uint64_t written = 0;
    uint64_t last_progress = 0;
    
    uint64_t total_entropy = 0;
    uint64_t min_found = 9999;
    uint64_t max_found = 0;
    
    while (written < count) {
        int entropy = bit_dist(gen);
        
        string seed = generate_seed_with_entropy(entropy);
        
        buffer.append(seed);
        buffer.push_back('\n');
        ++written;
        
        total_entropy += entropy;
        min_found = min(min_found, (uint64_t)entropy);
        max_found = max(max_found, (uint64_t)entropy);
        
        if (buffer.size() >= BUFFER_SIZE) {
            file.write(buffer.c_str(), buffer.size());
            buffer.clear();
            
            uint64_t progress = (written * 100) / count;
            if (progress > last_progress) {
                last_progress = progress;
                auto now = chrono::steady_clock::now();
                double elapsed = chrono::duration<double>(now - start).count();
                double speed = written / elapsed;
                
                cout << "\r📊 " << progress << "%"
                     << "  |  " << written << " / " << count
                     << "  |  " << fixed << setprecision(0) << speed << " seeds/s"
                     << "  |  Śr. entropia: " << (total_entropy / written) << " bitów"
                     << flush;
            }
        }
    }
    
    if (!buffer.empty()) {
        file.write(buffer.c_str(), buffer.size());
    }
    file.close();
    
    auto end = chrono::steady_clock::now();
    double seconds = chrono::duration<double>(end - start).count();
    
    cout << "\n\n✅ Wygenerowano " << written << " seedów w "
         << fixed << setprecision(2) << seconds << " s\n";
    cout << "   Średnia prędkość: " << fixed << setprecision(0) 
         << (written / seconds) << " seeds/s\n";
    cout << "   Średnia entropia: " << (total_entropy / written) << " bitów\n";
    cout << "   Min entropia: " << min_found << " bitów\n";
    cout << "   Max entropia: " << max_found << " bitów\n";
}

int main(int argc, char* argv[]) {
    cout << string(70, '=') << endl;
    cout << "GENERATOR HEX Z WIODĄCYMI ZERAMI - TYLKO CYFRY" << endl;
    cout << string(70, '=') << endl;
    
    string filename = "seeds_zeros.txt";
    uint64_t min_bits = 50;
    uint64_t max_bits = 256;
    uint64_t count = 100;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0 && i+1 < argc) {
            filename = argv[++i];
        } else if (strcmp(argv[i], "-min") == 0 && i+1 < argc) {
            min_bits = stoull(argv[++i]);
        } else if (strcmp(argv[i], "-max") == 0 && i+1 < argc) {
            max_bits = stoull(argv[++i]);
        } else if (strcmp(argv[i], "-n") == 0 && i+1 < argc) {
            count = stoull(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            cout << "\nUżycie:\n";
            cout << "  ./generator -min <min> -max <max> -n <liczba>\n";
            cout << "\nPrzykłady:\n";
            cout << "  ./generator -min 50 -max 50 -n 10    # tylko 50-bitowe (same cyfry)\n";
            cout << "  ./generator -min 50 -max 256 -n 100  # mieszane 50-256 bitów\n";
            cout << "  ./generator -f test.txt -min 50 -max 256 -n 1000\n";
            return 0;
        }
    }
    
    generate_seeds(filename, min_bits, max_bits, count);
    
    cout << "\n📋 Podgląd pierwszych 10 seedów:\n";
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        int i = 0;
        while (getline(file, line) && i < 10) {
            int zeros = 0;
            while (zeros < 64 && line[zeros] == '0') zeros++;
            int chars = 64 - zeros;
            int entropy = chars * 4;
            
            cout << "   " << line.substr(0, 20) << "..." << line.substr(44) 
                 << "  (zera: " << zeros << ", cyfr: " << chars 
                 << ", entropia: " << entropy << " bitów)" << endl;
            i++;
        }
        file.close();
    }
    
    return 0;
}
