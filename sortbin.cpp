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

// ============================================================
// CHUNK_BYTES - ZAWSZE WIELOKROTNOŚĆ 20
// ============================================================
static const size_t CHUNK_BYTES = ((512ull * 1024 * 1024) / RECORD) * RECORD;

// ============================================================
// MAX_THREADS - BEZPIECZNE!
// ============================================================
static const size_t MAX_THREADS = std::max(1u,
    std::min(std::thread::hardware_concurrency(), 16u));

static const size_t WRITE_BUFFER_SIZE = 64 * 1024 * 1024;

std::atomic<size_t> total_processed(0);
size_t total_size = 0;

inline bool cmp20(const uint8_t* a, const uint8_t* b) {
    return memcmp(a, b, RECORD) < 0;
}

void sort_chunk(std::string infile, off_t offset, size_t bytes, std::string outfile) {
    if (bytes == 0) return;
    
    int fd = open(infile.c_str(), O_RDONLY);
    if (fd < 0) { perror("open"); exit(1); }

    size_t page_size = sysconf(_SC_PAGESIZE);
    off_t aligned_offset = (offset / page_size) * page_size;
    size_t offset_diff = offset - aligned_offset;
    size_t map_size = bytes + offset_diff;

    void* map = mmap(nullptr, map_size, PROT_READ, MAP_SHARED, fd, aligned_offset);
    if (map == MAP_FAILED) { perror("mmap"); exit(1); }

    uint8_t* base = (uint8_t*)map + offset_diff;
    size_t n = bytes / RECORD;

    std::vector<uint8_t> buf(bytes);
    memcpy(buf.data(), base, bytes);

    munmap(map, map_size);
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

    std::vector<uint8_t> write_buffer(WRITE_BUFFER_SIZE);
    size_t pos = 0;
    size_t written = 0;

    for (auto p : index) {
        memcpy(write_buffer.data() + pos, p, RECORD);
        pos += RECORD;
        written++;
        
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

    if (pos > 0) {
        ssize_t w = write(ofd, write_buffer.data(), pos);
        if (w != (ssize_t)pos) {
            perror("write");
            close(ofd);
            exit(1);
        }
    }

    if (written != n) {
        std::cerr << "ERROR: Wrote " << written << " records, expected " << n << "\n";
    }

    close(ofd);
}

bool verify_sorted(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "Cannot open " << filename << " for verification\n";
        return false;
    }

    uint8_t prev[RECORD];
    uint8_t cur[RECORD];
    
    ssize_t n = read(fd, prev, RECORD);
    if (n != RECORD) {
        close(fd);
        return true;
    }

    bool ok = true;
    size_t record_num = 1;
    while (read(fd, cur, RECORD) == RECORD) {
        if (memcmp(prev, cur, RECORD) > 0) {
            std::cerr << "❌ SORT ERROR at record " << record_num << "\n";
            ok = false;
            break;
        }
        memcpy(prev, cur, RECORD);
        record_num++;
    }

    close(fd);
    return ok;
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

    for (size_t i = 0; i < parts.size(); i++) {
        fds[i] = open(parts[i].c_str(), O_RDONLY);
        if (fds[i] < 0) { perror("open"); exit(1); }

        struct stat st{};
        if (fstat(fds[i], &st) < 0) { perror("fstat"); exit(1); }
        sizes[i] = st.st_size;

        if (sizes[i] > 0) {
            maps[i] = (uint8_t*)mmap(nullptr, sizes[i], PROT_READ, MAP_SHARED, fds[i], 0);
            if (maps[i] == MAP_FAILED) { perror("mmap"); exit(1); }
        } else {
            maps[i] = nullptr;
        }
    }

    int ofd = open(out.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (ofd < 0) { perror("open out"); exit(1); }

    std::vector<uint8_t> write_buffer(WRITE_BUFFER_SIZE);
    size_t pos = 0;
    size_t written = 0;

    std::priority_queue<Node> pq;
    for (size_t i = 0; i < parts.size(); i++) {
        if (sizes[i] > 0) {
            pq.push({maps[i], maps[i] + sizes[i], (int)i});
        }
    }

    while (!pq.empty()) {
        Node n = pq.top(); pq.pop();
        
        memcpy(write_buffer.data() + pos, n.ptr, RECORD);
        pos += RECORD;
        written++;
        
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

    if (pos > 0) {
        ssize_t w = write(ofd, write_buffer.data(), pos);
        if (w != (ssize_t)pos) {
            perror("write");
            close(ofd);
            exit(1);
        }
    }

    size_t total_records = total_size / RECORD;
    if (written != total_records) {
        std::cerr << "ERROR: Merge wrote " << written << " records, expected " << total_records << "\n";
    }

    close(ofd);

    for (size_t i = 0; i < parts.size(); i++) {
        if (maps[i]) munmap(maps[i], sizes[i]);
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

    if (total_size % RECORD != 0) {
        std::cerr << "\n========================================\n";
        std::cerr << "ERROR: File size is not a multiple of 20!\n";
        std::cerr << "  Size: " << total_size << " bytes\n";
        std::cerr << "  Mod 20: " << (total_size % 20) << "\n";
        std::cerr << "========================================\n\n";
        std::cerr << "This means adresy.bin is corrupted!\n";
        std::cerr << "Please regenerate with adrestobin\n";
        return 1;
    }

    if (total_size == 0) {
        std::cerr << "ERROR: Input file is empty!\n";
        return 1;
    }

    std::cout << "📁 Input file: " << in << " (" << (total_size / 1e9) << " GB)\n";
    std::cout << "📊 Records: " << (total_size / RECORD) << "\n";
    std::cout << "✅ File size OK (multiple of 20)\n";
    std::cout << "📦 Chunk size: " << (CHUNK_BYTES / (1024*1024)) << " MB (multiple of 20!)\n";
    std::cout << "🧵 Threads: " << MAX_THREADS << "\n";
    std::cout << "🚀 Sorting chunks...\n";

    std::vector<std::future<void>> futures;
    std::vector<std::string> chunks;

    size_t num_chunks = 0;
    for (off_t offset = 0; offset < (off_t)total_size; offset += CHUNK_BYTES) {
        while (futures.size() >= MAX_THREADS) {
            futures.front().get();
            futures.erase(futures.begin());
        }

        size_t size = std::min<size_t>(CHUNK_BYTES, total_size - offset);
        
        // ============================================================
        // TYLKO WSZYSTKIE CHUNKI OPRÓCZ OSTATNIEGO WYRÓWNUJEMY!
        // ============================================================
        if (offset + size < total_size) {
            size -= size % RECORD;
        }
        
        if (size == 0) break;
        
        std::string part = "chunk_" + std::to_string(++num_chunks) + ".bin";
        chunks.push_back(part);

        futures.push_back(
            std::async(std::launch::async, sort_chunk, in, offset, size, part)
        );
    }

    for (auto& f : futures) f.get();

    total_processed.store(0);

    std::cout << "\n🔄 Merging " << chunks.size() << " chunks...\n";
    merge_chunks(chunks, out);

    struct stat out_st{};
    if (stat(out.c_str(), &out_st) == 0) {
        std::cout << "\n📊 Output file size: " << out_st.st_size << " bytes\n";
        if (out_st.st_size % RECORD != 0) {
            std::cerr << "❌ ERROR: Output file is corrupted! Not multiple of 20!\n";
        } else {
            std::cout << "✅ Output file size OK (multiple of 20)\n";
            
            std::cout << "🔍 Verifying sort order...\n";
            if (verify_sorted(out)) {
                std::cout << "✅ Sort order verified OK!\n";
            } else {
                std::cerr << "❌ Sort order verification FAILED!\n";
            }
        }
    }

    std::cout << "\n✅ DONE! Sorted file saved to: " << out << "\n";
    std::cout << "📊 Records in output: " << (total_size / RECORD) << "\n";

    for (const auto& chunk : chunks) {
        std::filesystem::remove(chunk);
    }

    std::cout << "🧹 Temporary chunk files removed.\n";
    return 0;
}
