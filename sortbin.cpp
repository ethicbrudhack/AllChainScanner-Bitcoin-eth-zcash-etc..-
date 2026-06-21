#include <algorithm>
#include <atomic>
#include <fcntl.h>
#include <filesystem>
#include <future>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <queue>

static const size_t RECORD = 20;
static const size_t CHUNK_BYTES = 512ull * 1024 * 1024; // 512 MB (BEZPIECZNIEJSZE!)
static const size_t MAX_THREADS = std::min(std::thread::hardware_concurrency(), 16u); // Max 16
static const size_t WRITE_BUFFER_SIZE = 64 * 1024 * 1024; // 64 MB bufor

std::atomic<size_t> total_processed(0);
size_t total_size = 0;

inline bool cmp20(const uint8_t* a, const uint8_t* b) {
    return memcmp(a, b, RECORD) < 0;
}

void sort_chunk(std::string infile, off_t offset, size_t bytes, std::string outfile) {
    int fd = open(infile.c_str(), O_RDONLY);
    if (fd < 0) { perror("open"); exit(1); }

    void* map = mmap(nullptr, bytes, PROT_READ, MAP_PRIVATE, fd, offset);
    if (map == MAP_FAILED) { perror("mmap"); exit(1); }

    uint8_t* base = (uint8_t*)map;
    size_t n = bytes / RECORD;

    std::vector<uint8_t> buf(bytes);
    memcpy(buf.data(), base, bytes);

    munmap(map, bytes);
    close(fd);

    uint8_t* ptr = buf.data();
    std::vector<uint8_t*> index(n);
    for (size_t i = 0; i < n; i++) index[i] = ptr + i * RECORD;

    std::sort(index.begin(), index.end(),
        [](const uint8_t* a, const uint8_t* b) {
            return memcmp(a, b, RECORD) < 0;
        }
    );

    int ofd = open(outfile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (ofd < 0) { perror("open out"); exit(1); }

    // ===============================
    // OPTYMALIZACJA: resize() zamiast reserve() + insert()
    // ===============================
    std::vector<uint8_t> write_buffer(WRITE_BUFFER_SIZE);
    size_t pos = 0;

    for (auto p : index) {
        memcpy(write_buffer.data() + pos, p, RECORD);
        pos += RECORD;
        
        if (pos >= WRITE_BUFFER_SIZE) {
            ssize_t w = write(ofd, write_buffer.data(), pos);
            if (w != (ssize_t)pos) {
                perror("write");
                close(ofd);
                exit(1);
            }
            pos = 0;
        }

        size_t old = total_processed.fetch_add(RECORD);
        if ((old / (50ULL << 20)) != ((old + RECORD) / (50ULL << 20))) {
            double pct = (old + RECORD) * 100.0 / total_size;
            std::cout << "\r[CHUNK] " << pct << "% ("
                      << ((old + RECORD) >> 20) << " MB / "
                      << (total_size >> 20) << " MB)"
                      << std::flush;
        }
    }

    // Zapisz pozostałe dane
    if (pos > 0) {
        ssize_t w = write(ofd, write_buffer.data(), pos);
        if (w != (ssize_t)pos) {
            perror("write");
            close(ofd);
            exit(1);
        }
    }

    close(ofd);
}

void merge_chunks(std::vector<std::string> parts, std::string out) {
    struct Node {
        const uint8_t* ptr;
        const uint8_t* end;
        int idx;

        bool operator<(Node const& other) const {
            return memcmp(ptr, other.ptr, RECORD) > 0;
        }
    };

    std::vector<int> fds(parts.size());
    std::vector<uint8_t*> maps(parts.size());
    std::vector<size_t> sizes(parts.size());

    for (int i = 0; i < parts.size(); i++) {
        fds[i] = open(parts[i].c_str(), O_RDONLY);
        if (fds[i] < 0) { perror("open"); exit(1); }

        struct stat st{};
        if (fstat(fds[i], &st) < 0) { perror("fstat"); exit(1); }
        sizes[i] = st.st_size;

        maps[i] = (uint8_t*)mmap(nullptr, sizes[i], PROT_READ, MAP_PRIVATE, fds[i], 0);
        if (maps[i] == MAP_FAILED) { perror("mmap"); exit(1); }
    }

    int ofd = open(out.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (ofd < 0) { perror("open out"); exit(1); }

    // ===============================
    // OPTYMALIZACJA: resize() zamiast reserve() + insert()
    // ===============================
    std::vector<uint8_t> write_buffer(WRITE_BUFFER_SIZE);
    size_t pos = 0;

    std::priority_queue<Node> pq;
    for (int i = 0; i < parts.size(); i++) {
        pq.push({maps[i], maps[i] + sizes[i], i});
    }

    while (!pq.empty()) {
        Node n = pq.top(); pq.pop();
        
        memcpy(write_buffer.data() + pos, n.ptr, RECORD);
        pos += RECORD;
        
        if (pos >= WRITE_BUFFER_SIZE) {
            ssize_t w = write(ofd, write_buffer.data(), pos);
            if (w != (ssize_t)pos) {
                perror("write");
                close(ofd);
                exit(1);
            }
            pos = 0;
        }

        size_t old = total_processed.fetch_add(RECORD);
        if ((old / (50ULL << 20)) != ((old + RECORD) / (50ULL << 20))) {
            double pct = (old + RECORD) * 100.0 / total_size;
            std::cout << "\r[MERGE] " << pct << "% ("
                      << ((old + RECORD) >> 20) << " MB / "
                      << (total_size >> 20) << " MB)"
                      << std::flush;
        }

        const uint8_t* next = n.ptr + RECORD;
        if (next < n.end)
            pq.push({next, n.end, n.idx});
    }

    // Zapisz pozostałe dane
    if (pos > 0) {
        ssize_t w = write(ofd, write_buffer.data(), pos);
        if (w != (ssize_t)pos) {
            perror("write");
            close(ofd);
            exit(1);
        }
    }

    close(ofd);

    for (int i = 0; i < parts.size(); i++) {
        munmap(maps[i], sizes[i]);
        close(fds[i]);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: ./sorter <input.bin> <output.bin>\n";
        return 1;
    }

    std::string in = argv[1];
    std::string out = argv[2];

    struct stat st{};
    if (stat(in.c_str(), &st) < 0) {
        perror("stat");
        return 1;
    }
    total_size = st.st_size;

    std::cout << "📁 Input file: " << in << " (" << (total_size / 1e9) << " GB)\n";
    std::cout << "🚀 Sorting chunks with " << MAX_THREADS << " threads...\n";
    std::cout << "📦 Chunk size: " << (CHUNK_BYTES / (1024*1024)) << " MB\n";
    std::cout << "💾 RAM usage estimate: ~" 
              << (MAX_THREADS * (CHUNK_BYTES / (1024*1024*1024) * 1.4)) 
              << " GB (for " << MAX_THREADS << " threads)\n";

    std::vector<std::future<void>> futures;
    std::vector<std::string> chunks;

    size_t num_chunks = 0;
    for (off_t offset = 0; offset < total_size; offset += CHUNK_BYTES) {
        while (futures.size() >= MAX_THREADS) {
            futures.front().get();
            futures.erase(futures.begin());
        }

        size_t size = std::min<size_t>(CHUNK_BYTES, total_size - offset);
        std::string part = "chunk_" + std::to_string(++num_chunks) + ".bin";
        chunks.push_back(part);

        futures.push_back(
            std::async(std::launch::async, sort_chunk, in, offset, size, part)
        );
    }

    for (auto& f : futures) f.get();

    // ===============================
    // POPRAWA: Reset counter przed merge
    // ===============================
    total_processed.store(0);

    std::cout << "\n🔄 Merging " << chunks.size() << " chunks...\n";
    merge_chunks(chunks, out);

    std::cout << "\n✅ DONE! Sorted file saved to: " << out << "\n";

    // Clean up temporary chunk files
    for (const auto& chunk : chunks) {
        std::filesystem::remove(chunk);
    }

    std::cout << "🧹 Temporary chunk files removed.\n";
    return 0;
}
