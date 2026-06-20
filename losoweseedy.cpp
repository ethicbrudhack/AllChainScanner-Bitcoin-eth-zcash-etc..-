// ============================================================
// GENERATOR SEEDÓW - DOWOLNE ZNAKI (poprawiony!)
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <random>

using namespace std;

static const size_t BUFFER_SIZE = 1024 * 1024 * 100; // 100 MB bufor

void generate_seeds(const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "❌ Nie można otworzyć pliku\n";
        return;
    }

    // ========== TU USTAWIA SZ PARAMETRY ==========
    const uint64_t TARGET_SIZE = 1024ULL * 1024 * 1024 * 10; // 10 GB
    // const uint64_t TARGET_SIZE = const uint64_t TARGET_SIZE = 1024ULL * 1024 * 1024 * 10
    const int SEED_LENGTH = 64;
    const string CHARS = "01"; // <-- ZMIEŃ TU ZNAKI
    // =============================================
    
    string buffer;
    buffer.reserve(BUFFER_SIZE);
    
    uint64_t written_bytes = 0;
    uint64_t seed_count = 0;
    
    auto start = chrono::steady_clock::now();
    
    cout << "\n🚀 Generuję " << (TARGET_SIZE / (1024*1024)) << " MB seedów\n";
    cout << "   Długość: " << SEED_LENGTH << " znaków\n";
    cout << "   Znaki: " << CHARS << " (" << CHARS.size() << " znaków)\n";
    cout << "   Każdy seed: 64 znaki + newline = 65 bajtów\n";
    cout << "   Potrzebne seedów: ~" << (TARGET_SIZE / 65) << "\n\n";
    
    // Inicjalizacja generatora liczb losowych
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, CHARS.size() - 1); // <-- WAŻNE!
    
    uint64_t last_progress_mb = 0;
    const uint64_t target_mb = TARGET_SIZE / (1024*1024);
    
    while (written_bytes < TARGET_SIZE) {
        // Generuj jeden losowy seed
        string seed;
        seed.reserve(SEED_LENGTH);
        for (int i = 0; i < SEED_LENGTH; ++i) {
            seed += CHARS[dist(gen)]; // <-- TU WYBIERA ZNAK Z CHARS!
        }
        
        buffer += seed + "\n";
        written_bytes += SEED_LENGTH + 1;
        ++seed_count;
        
        if (buffer.size() >= BUFFER_SIZE) {
            file.write(buffer.c_str(), buffer.size());
            buffer.clear();
            
            // Status co 1 MB
            uint64_t current_mb = written_bytes / (1024*1024);
            if (current_mb > last_progress_mb) {
                last_progress_mb = current_mb;
                auto now = chrono::steady_clock::now();
                double elapsed = chrono::duration<double>(now - start).count();
                double speed_mb = (written_bytes / elapsed) / (1024 * 1024);
                double progress = (double)written_bytes / TARGET_SIZE * 100;
                
                cout << "\r📊 " << fixed << setprecision(2) << progress << "%"
                     << "  |  " << current_mb << " MB / " << target_mb << " MB"
                     << "  |  " << fixed << setprecision(0) << speed_mb << " MB/s"
                     << "  |  Seedów: " << seed_count
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
    double speed_mb = (written_bytes / seconds) / (1024 * 1024);
    
    cout << "\n\n✅ Wygenerowano " << seed_count << " seedów w "
         << fixed << setprecision(2) << seconds << " s\n";
    cout << "   Średnia prędkość: " << fixed << setprecision(0) << speed_mb << " MB/s\n";
    cout << "   Rozmiar pliku: " << (written_bytes / (1024*1024)) << " MB\n";
}

int main(int argc, char* argv[]) {
    cout << string(70, '=') << endl;
    cout << "GENERATOR SEEDÓW - DOWOLNE ZNAKI" << endl;
    cout << string(70, '=') << endl;
    
    string filename = "seeds01.txt";
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0 && i+1 < argc) {
            filename = argv[++i];
        }
    }
    
    generate_seeds(filename);
    
    // Podgląd pierwszych 5 seedów
    cout << "\n📋 Podgląd pierwszych 5 wygenerowanych seedów:\n";
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        int count = 0;
        while (getline(file, line) && count < 5) {
            cout << "   " << line.substr(0, 30) << "..." << line.substr(44) << "\n";
            count++;
        }
        file.close();
    }
    
    return 0;
}