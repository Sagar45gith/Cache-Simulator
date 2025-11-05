#include <iostream>
#include <string>
#include <vector>
#include <list>         // For LRU and LFU
#include <queue>        // For FIFO
#include <unordered_map> // For LRU, LFU
#include <unordered_set> // For FIFO
#include <memory>       // For smart pointers
#include <iomanip>      // For std::setprecision
#include <limits>       // For main.cpp

// ========================================================================
// 1. THE INTERFACE (Abstract Base Class)
// ========================================================================
/**
 * @class ICachePolicy
 * @brief An "interface" or "contract" for all cache policies.
 */
class ICachePolicy {
public:
    virtual ~ICachePolicy() {}
    virtual void access(const std::string& key) = 0;
    virtual void print_stats() const = 0;
    virtual void print_cache_state() const = 0;

protected:
    size_t capacity;
    int hits;
    int misses;
    ICachePolicy(size_t cap) : capacity(cap), hits(0), misses(0) {
        if (capacity == 0) capacity = 1;
    }
};

// ========================================================================
// 2. LRU (Least Recently Used) IMPLEMENTATION
// ========================================================================
/**
 * @class LRUCache
 * @brief Implements the LRU policy using a list and a map.
 */
class LRUCache : public ICachePolicy {
private:
    std::list<std::string> lru_list; 
    std::unordered_map<std::string, std::list<std::string>::iterator> cache_map;

public:
    LRUCache(size_t cap) : ICachePolicy(cap) {}

    void access(const std::string& key) override {
        std::cout << "Accessing: " << key;
        auto it = cache_map.find(key);

        if (it != cache_map.end()) {
            // --- CACHE HIT ---
            hits++;
            std::cout << " -> HIT" << std::endl;
            lru_list.splice(lru_list.begin(), lru_list, it->second);
            it->second = lru_list.begin();
        } else {
            // --- CACHE MISS ---
            misses++;
            std::cout << " -> MISS" << std::endl;
            if (lru_list.size() >= capacity) {
                std::string lru_key = lru_list.back();
                std::cout << "  Cache full. Evicting (LRU): " << lru_key << std::endl;
                cache_map.erase(lru_key);
                lru_list.pop_back();
            }
            lru_list.push_front(key);
            cache_map[key] = lru_list.begin();
        }
        print_cache_state();
    }

    void print_stats() const override {
        std::cout << "\n--- [LRU] Simulation Complete ---" << std::endl;
        std::cout << "Total Accesses: " << (hits + misses) << std::endl;
        std::cout << "Cache Hits:     " << hits << std::endl;
        std::cout << "Cache Misses:   " << misses << std::endl;
        double hit_rate = (hits + misses > 0) ? (static_cast<double>(hits) / (hits + misses) * 100.0) : 0.0;
        std::cout << "Hit Rate:       " << std::fixed << std::setprecision(2) << hit_rate << "%" << std::endl;
    }

    void print_cache_state() const override {
        std::cout << "  Cache (MRU -> LRU): [ ";
        for (const auto& key : lru_list) {
            std::cout << key << " ";
        }
        std::cout << "]" << std::endl;
    }
};

// ========================================================================
// 3. FIFO (First-In, First-Out) IMPLEMENTATION
// ========================================================================
/**
 * @class FIFOCache
 * @brief Implements the FIFO policy using a queue and a set.
 */
class FIFOCache : public ICachePolicy {
private:
    std::queue<std::string> fifo_queue;
    std::unordered_set<std::string> cache_set;

public:
    FIFOCache(size_t cap) : ICachePolicy(cap) {}

    void access(const std::string& key) override {
        std::cout << "Accessing: " << key;
        if (cache_set.find(key) != cache_set.end()) {
            // --- CACHE HIT ---
            hits++;
            std::cout << " -> HIT" << std::endl;
            // No reordering for FIFO on hit
        } else {
            // --- CACHE MISS ---
            misses++;
            std::cout << " -> MISS" << std::endl;
            if (fifo_queue.size() >= capacity) {
                std::string fifo_key = fifo_queue.front();
                std::cout << "  Cache full. Evicting (FIFO): " << fifo_key << std::endl;
                fifo_queue.pop();
                cache_set.erase(fifo_key);
            }
            fifo_queue.push(key);
            cache_set.insert(key);
        }
        print_cache_state();
    }

    void print_stats() const override {
        std::cout << "\n--- [FIFO] Simulation Complete ---" << std::endl;
        std::cout << "Total Accesses: " << (hits + misses) << std::endl;
        std::cout << "Cache Hits:     " << hits << std::endl;
        std::cout << "Cache Misses:   " << misses << std::endl;
        double hit_rate = (hits + misses > 0) ? (static_cast<double>(hits) / (hits + misses) * 100.0) : 0.0;
        std::cout << "Hit Rate:       " << std::fixed << std::setprecision(2) << hit_rate << "%" << std::endl;
    }

    void print_cache_state() const override {
        std::cout << "  Cache (Front/Oldest -> Back/Newest): [ ";
        std::queue<std::string> temp_q = fifo_queue;
        while (!temp_q.empty()) {
            std::cout << temp_q.front() << " ";
            temp_q.pop();
        }
        std::cout << "]" << std::endl;
    }
};

// ========================================================================
// 4. LFU (Least Frequently Used) IMPLEMENTATION
// ========================================================================
/**
 * @class LFUCache
 * @brief Implements the LFU policy.
 *
 * This is a very complex implementation that requires three data structures
 * for O(1) average time complexity for both access and eviction.
 *
 * 1. cache_map: (The main directory)
 * - Type: unordered_map<string, ...>
 * - Job: O(1) lookup of any key.
 * - Value: A pair containing {frequency, iterator to the key's position
 * in the freq_map's list}.
 *
 * 2. freq_map: (The frequency groups)
 * - Type: unordered_map<int, list<string>>
 * - Job: Groups all keys by their frequency count.
 * - Value: A std::list of keys, kept in MRU-to-LRU order. This
 * list is used for the LRU tie-breaker.
 *
 * 3. min_frequency:
 * - Type: int
 * - Job: Tracks the lowest frequency currently in the cache,
 * which tells us which list in freq_map to evict from.
 */
class LFUCache : public ICachePolicy {
private:
    // We need to store an iterator AND the frequency in our main map
    using list_iterator = std::list<std::string>::iterator;
    using cache_entry = std::pair<int, list_iterator>;
    
    // 1. The Main Directory
    std::unordered_map<std::string, cache_entry> cache_map;
    
    // 2. The Frequency Groups
    //    map< frequency, list_of_keys_with_that_frequency >
    std::unordered_map<int, std::list<std::string>> freq_map;
    
    // 3. The Eviction Pointer
    int min_frequency;

    /**
     * @brief Internal helper to update a key's frequency.
     * This is the core logic of LFU.
     * It moves a key from its old frequency list to the new one.
     */
    void update_frequency(const std::string& key, cache_entry& entry) {
        int old_freq = entry.first;
        int new_freq = old_freq + 1;
        
        // 1. Remove the key from the OLD frequency list
        freq_map[old_freq].erase(entry.second);
        
        // 2. Add the key to the FRONT (MRU) of the NEW frequency list
        freq_map[new_freq].push_front(key);
        
        // 3. Update the cache_map entry with the new frequency and new list iterator
        entry.first = new_freq;
        entry.second = freq_map[new_freq].begin();
        
        // 4. Update min_frequency if the old list is now empty
        //    and it was the minimum.
        if (freq_map[old_freq].empty() && old_freq == min_frequency) {
            min_frequency = new_freq;
        }
    }

public:
    LFUCache(size_t cap) : ICachePolicy(cap), min_frequency(0) {}

    void access(const std::string& key) override {
        std::cout << "Accessing: " << key;
        
        auto it = cache_map.find(key);

        if (it != cache_map.end()) {
            // --- CACHE HIT ---
            hits++;
            std::cout << " -> HIT" << std::endl;
            
            // A "hit" means its frequency increases.
            // We call our helper to move it to the next frequency list.
            update_frequency(key, it->second);
            
        } else {
            // --- CACHE MISS ---
            misses++;
            std::cout << " -> MISS" << std::endl;

            // Check if eviction is needed
            if (cache_map.size() >= capacity) {
                // Evict the LFU item.
                // 1. Get the list of least frequent items
                std::list<std::string>& lfu_list = freq_map[min_frequency];
                
                // 2. Get the LRU item from that list (the one at the back)
                std::string lfu_key = lfu_list.back();
                std::cout << "  Cache full. Evicting (LFU): " << lfu_key 
                          << " (freq: " << min_frequency << ")" << std::endl;
                
                // 3. Remove it from the list
                lfu_list.pop_back();
                
                // 4. Remove it from the main cache_map
                cache_map.erase(lfu_key);
            }

            // Add the new item
            // 1. All new items have a frequency of 1
            int new_freq = 1;
            freq_map[new_freq].push_front(key);
            
            // 2. Add it to the main cache_map
            cache_map[key] = {new_freq, freq_map[new_freq].begin()};
            
            // 3. A new item always resets the min_frequency to 1
            min_frequency = 1;
        }
        print_cache_state();
    }

    void print_stats() const override {
        std::cout << "\n--- [LFU] Simulation Complete ---" << std::endl;
        std::cout << "Total Accesses: " << (hits + misses) << std::endl;
        std::cout << "Cache Hits:     " << hits << std::endl;
        std::cout << "Cache Misses:   " << misses << std::endl;
        double hit_rate = (hits + misses > 0) ? (static_cast<double>(hits) / (hits + misses) * 100.0) : 0.0;
        std::cout << "Hit Rate:       " << std::fixed << std::setprecision(2) << hit_rate << "%" << std::endl;
    }

    /**
     * @brief Helper to visualize LFU cache state.
     * This is more complex as it iterates through the frequency map.
     */
    void print_cache_state() const override {
        std::cout << "  Cache (LFU):" << std::endl;
        if (cache_map.empty()) {
            std::cout << "  [ Empty ]" << std::endl;
            return;
        }
        
        // This is complex to print, so we'll just print by frequency
        // We can't easily find the *min* key of a map, so we'll just
        // iterate. This is just for debug, so O(N) is fine.
        int current_freq = min_frequency;
        int items_printed = 0;
        while(items_printed < cache_map.size()) {
            if (freq_map.count(current_freq)) {
                const auto& list = freq_map.at(current_freq);
                if (!list.empty()) {
                    std::cout << "    Freq " << current_freq << " (MRU->LRU): [ ";
                    for (const auto& key : list) {
                        std::cout << key << " ";
                    }
                    std::cout << "]" << std::endl;
                    items_printed += list.size();
                }
            }
            current_freq++;
        }
    }
};


// ========================================================================
// 5. THE SIMULATOR CLASS (Updated)
// ========================================================================
/**
 * @class CacheSimulator
 * @brief Manages the simulation process.
 *
 * Holds a pointer to the ICachePolicy interface, making it
 * flexible to run LRU, FIFO, or LFU.
 */
class CacheSimulator {
private:
    std::unique_ptr<ICachePolicy> cache;
    std::vector<std::string> access_sequence;

public:
    /**
     * @brief Constructor for CacheSimulator.
     * It now takes a 'policy_type' string to decide
     * which cache policy object to create.
     */
    CacheSimulator(size_t capacity, 
                   const std::vector<std::string>& sequence, 
                   const std::string& policy_type)
        : access_sequence(sequence) {
        
        std::cout << "\n--- Initializing " << policy_type 
                  << " simulator with capacity " << capacity << " ---" << std::endl;
                  
        if (policy_type == "LRU") {
            cache = std::make_unique<LRUCache>(capacity);
        } else if (policy_type == "FIFO") {
            cache = std::make_unique<FIFOCache>(capacity);
        } else if (policy_type == "LFU") {
            cache = std::make_unique<LFUCache>(capacity);
        }
        else {
            std::cerr << "Unknown policy type '" << policy_type 
                      << "'. Defaulting to LRU." << std::endl;
            cache = std::make_unique<LRUCache>(capacity);
        }
    }

    /**
     * @brief Runs the simulation.
     * This function doesn't change! It just calls the virtual
     * functions, and polymorphism does the rest.
     */
    void run() {
        for (const auto& key : access_sequence) {
            cache->access(key); 
        }
        cache->print_stats();
    }
};