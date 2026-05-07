//dcache_3c.cc tracks 3C miss classification (compulsory, capacity, conflict) for the dcache.
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <cstdio>
#include <cstdlib>

// Tracks every line address ever seen by the dcache (for compulsory detection)
static std::unordered_set<unsigned long long> seen_addresses;

// Shadow fully-associative LRU cache of the same SIZE as the real dcache.
// If a real-cache miss WOULD have hit here, it's a conflict miss.
// If it would have missed here too, it's a capacity miss.
static int shadow_size = 0;
static std::list<unsigned long long> lru_list;
static std::unordered_map<unsigned long long,
    std::list<unsigned long long>::iterator> lru_map;

static unsigned long long compulsory_count = 0;
static unsigned long long capacity_count   = 0;
static unsigned long long conflict_count   = 0;
static unsigned long long total_misses     = 0;
static unsigned long long total_hits       = 0;

static int initialized = 0;

extern "C" {

static void dcache_3c_dump(void) {
    FILE* f = fopen("dcache_3c.csv", "w");
    if (!f) return;
    fprintf(f, "metric,value\n");
    fprintf(f, "compulsory,%llu\n", compulsory_count);
    fprintf(f, "capacity,%llu\n",   capacity_count);
    fprintf(f, "conflict,%llu\n",   conflict_count);
    fprintf(f, "total_misses,%llu\n", total_misses);
    fprintf(f, "total_hits,%llu\n",   total_hits);
    fclose(f);
}

void dcache_3c_init(int num_lines) {
    if (initialized) return;
    shadow_size = num_lines;
    atexit(dcache_3c_dump);
    initialized = 1;
}

// Returns 1 if shadow cache had it (hit), 0 otherwise. Updates LRU.
static int shadow_access(unsigned long long line_addr) {
    auto it = lru_map.find(line_addr);
    if (it != lru_map.end()) {
        lru_list.erase(it->second);
        lru_list.push_front(line_addr);
        lru_map[line_addr] = lru_list.begin();
        return 1;
    } else {
        if ((int)lru_list.size() >= shadow_size) {
            unsigned long long evicted = lru_list.back();
            lru_list.pop_back();
            lru_map.erase(evicted);
        }
        lru_list.push_front(line_addr);
        lru_map[line_addr] = lru_list.begin();
        return 0;
    }
}

void dcache_3c_record_miss(unsigned long long line_addr) {
    total_misses++;
    int first_time = (seen_addresses.find(line_addr) == seen_addresses.end());
    int shadow_hit = shadow_access(line_addr);
    if (first_time) {
        seen_addresses.insert(line_addr);
        compulsory_count++;
    } else if (!shadow_hit) {
        capacity_count++;
    } else {
        conflict_count++;
    }
}

void dcache_3c_record_hit(unsigned long long line_addr) {
    total_hits++;
    seen_addresses.insert(line_addr);
    shadow_access(line_addr);
}

} // extern "C"
