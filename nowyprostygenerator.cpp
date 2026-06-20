// nowyprostygenerator.cpp - DŁUGOŚĆ 64 BEZ OGRANICZEŃ
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <algorithm>

using namespace std;

static const size_t BUFFER_SIZE = 1024 * 1024 * 10;

class MixedGenerator {
private:
    string chars;
    ofstream* file;
    string buffer;
    uint64_t written;
    uint64_t max_patterns;
    chrono::steady_clock::time_point start;
    int length;
    
    void generate(int depth, string& current) {
        if (max_patterns > 0 && written >= max_patterns) return;
        
        if (depth == length) {
            buffer += current;
            buffer += '\n';
            
            if (buffer.size() >= BUFFER_SIZE) {
                file->write(buffer.c_str(), buffer.size());
                buffer.clear();
            }
            
            ++written;
            
            auto now = chrono::steady_clock::now();
            double elapsed = chrono::duration<double>(now - start).count();
            if (elapsed > 0 && (int)elapsed % 1 == 0) {
                cout << "\r📊 " << written << " / " << max_patterns
                     << "  |  " << fixed << setprecision(0) << (written/elapsed) << "/s"
                     << flush;
            }
            return;
        }
        
        for (char c : chars) {
            if (max_patterns > 0 && written >= max_patterns) return;
            current.push_back(c);
            generate(depth + 1, current);
            current.pop_back();
        }
    }

public:
    MixedGenerator(const string& charset) : chars(charset) {
        sort(chars.begin(), chars.end());
        chars.erase(unique(chars.begin(), chars.end()), chars.end());
    }

    void generate_and_save(const string& filename, int len, uint64_t limit = 0) {
        file = new ofstream(filename, ios::binary);
        if (!file->is_open()) {
            cerr << "❌ Nie można otworzyć pliku\n";
            return;
        }

        buffer.reserve(BUFFER_SIZE);
        written = 0;
        max_patterns = limit;
        length = len;
        start = chrono::steady_clock::now();

        uint64_t total = 1;
        for (int i = 0; i < length; i++) {
            total *= chars.size();
            if (total > UINT64_MAX / chars.size()) {
                total = UINT64_MAX;
                break;
            }
        }

        cout << "\n🚀 Generuję MIESZANE wzorce długości " << length << "\n";
        cout << "   Znaki: " << chars << " (" << chars.size() << " znaków)\n";
        cout << "   Wszystkich możliwych: " << total << "\n";
        if (limit > 0) cout << "   Limit: " << limit << "\n";
        cout << "\n⚠️  UWAGA: Dla 3 znaków i długości 64 to 3^64 wzorców!\n";
        cout << "   Używaj limitu -m aby ograniczyć liczbę!\n\n";

        string current;
        current.reserve(length);
        generate(0, current);

        if (!buffer.empty()) {
            file->write(buffer.c_str(), buffer.size());
            buffer.clear();
        }
        
        file->close();
        delete file;

        auto end = chrono::steady_clock::now();
        double seconds = chrono::duration<double>(end - start).count();

        cout << "\n\n✅ Zapisano " << written << " wzorców w "
             << fixed << setprecision(2) << seconds << " s\n";
        if (seconds > 0) {
            cout << "   Średnia prędkość: "
                 << fixed << setprecision(0) << (written / seconds) << " wzorców/s\n";
        }
    }
};

void print_help() {
    cout << "\nGenerator MIESZANYCH wzorców 64-znakowych\n"
         << "Generuje WSZYSTKIE możliwe kombinacje znaków (mieszane)!\n\n"
         << "Użycie:\n"
         << "  ./nowyprostygenerator [opcje]\n\n"
         << "Opcje:\n"
         << "  -c <znaki>   Zestaw znaków (domyślnie: 01)\n"
         << "  -l <liczba>  Długość wzorca (domyślnie: 64)\n"
         << "  -m <liczba>  Limit wzorców (WAŻNE!)\n"
         << "  -f <nazwa>   Plik wyjściowy (domyślnie: seeds0.txt)\n"
         << "  -h           Ta pomoc\n\n"
         << "Przykłady:\n"
         << "  ./nowyprostygenerator -c 01 -l 64 -m 30\n"
         << "  ./nowyprostygenerator -c 012 -l 64 -m 50\n"
         << "  ./nowyprostygenerator -c abc -l 64 -m 30\n"
         << "  ./nowyprostygenerator -c 0123456789abcdef -l 64 -m 100\n\n"
         << "⚠️  ZAWSZE używaj -m z limitem! Dla 3 znaków to 3^64 wzorców!\n";
}

int main(int argc, char* argv[]) {
    cout << string(70, '=') << endl;
    cout << "GENERATOR MIESZANYCH WZORCÓW 64-ZNAKOWYCH" << endl;
    cout << string(70, '=') << endl;

    string filename = "seeds0.txt";
    string charset = "01";
    int length = 64;  // DOMYŚLNIE 64!
    uint64_t max_patterns = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0 && i+1 < argc) filename = argv[++i];
        else if (strcmp(argv[i], "-c") == 0 && i+1 < argc) charset = argv[++i];
        else if (strcmp(argv[i], "-l") == 0 && i+1 < argc) {
            length = stoi(argv[++i]);
            if (length < 1) length = 1;
            // NIE OGRANICZAMY! Może być 64
        }
        else if (strcmp(argv[i], "-m") == 0 && i+1 < argc) max_patterns = stoull(argv[++i]);
        else if (strcmp(argv[i], "-h") == 0) { print_help(); return 0; }
        else {
            cerr << "❌ Nieznana opcja: " << argv[i] << "\n";
            print_help();
            return 1;
        }
    }

    MixedGenerator gen(charset);
    gen.generate_and_save(filename, length, max_patterns);

    // Sprawdź długość
    ifstream file(filename, ios::binary | ios::ate);
    if (file.is_open()) {
        size_t size = file.tellg();
        file.close();
        cout << "\n📁 Plik: " << filename << "\n";
        if (size < 1024) {
            cout << "   Rozmiar: " << size << " B\n";
        } else if (size < 1024 * 1024) {
            cout << "   Rozmiar: " << size / 1024 << " KB\n";
        } else if (size < 1024 * 1024 * 1024) {
            cout << "   Rozmiar: " << size / (1024 * 1024) << " MB\n";
        } else {
            cout << "   Rozmiar: " << size / (1024 * 1024 * 1024) << " GB\n";
        }
    }

    return 0;
}