#include "cache_simulator.h" // Include our class definitions
#include <iostream>
#include <string>
#include <vector>
#include <limits> 

/**
 * @brief Main entry point for the command-line application.
 *
 * Now we will run the *same* access sequence on *both*
 * cache policies to compare their performance.
 */
int main() {

    // 1. Define the cache capacity
    size_t capacity = 4;

    // 2. Define the *same* sequence of memory accesses for both tests
    std::vector<std::string> accesses = {
        "A", "B", "C", "D", // Misses, [D, C, B, A] (LRU) | [A, B, C, D] (FIFO)
        "A",                 // Hit (LRU), Hit (FIFO)
        "E",                 // Miss, evicts B (LRU) | evicts A (FIFO)
        "A",                 // Hit (LRU), Miss, evicts B (FIFO)
        "B",                 // Miss, evicts C (LRU) | Miss, evicts C (FIFO)
        "A",                 // Hit (LRU), Hit (FIFO)
        "C",                 // Miss, evicts D (LRU) | Miss, evicts D (FIFO)
        "D",                 // Miss, evicts E (LRU) | Miss, evicts E (FIFO)
        "E",
        "D",
        "C"                 // Miss, evicts B (LRU) | Miss, evicts A (FIFO)
    };

    // 3. Create and run the LRU simulator
    CacheSimulator sim_lru(capacity, accesses, "LRU");
    sim_lru.run();

    // 4. Create and run the FIFO simulator
    CacheSimulator sim_fifo(capacity, accesses, "FIFO");
    sim_fifo.run();

    CacheSimulator sim_lfu(capacity, accesses, "LFU");
    sim_lfu.run();

    // 5. Check the console output! You can now directly compare
    //    the Hit Rates of the two policies for the same workload.

    return 0;
}


/*
int main() {
    // 1. Prompt for cache capacity
    size_t capacity = 4; // Default capacity
    std::cout << "Enter the cache capacity (e.g., 4): ";
    std::cin >> capacity;

    // --- Input validation for capacity ---
    // This loop handles cases where the user types letters instead of numbers
    while (std::cin.fail()) {
        std::cout << "Invalid input. Please enter a number: ";
        std::cin.clear(); // Clear the error flag
        // Ignore the rest of the bad input line
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> capacity;
    }
    
    // Clear the newline character left in the buffer after cin >> capacity
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


    // 2. Prompt for the access sequence
    std::vector<std::string> accesses;
    std::string key;

    std::cout << "\nEnter your memory accesses one by one (e.g., A, B, C)." << std::endl;
    std::cout << "Type 'RUN' when you are finished:" << std::endl;

    // Loop to get user input
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, key); // Read the whole line

        // Simple trim for any accidental whitespace (optional but good)
        // Note: This is a simple version.
        // For production code, you'd trim more robustly.
        key.erase(0, key.find_first_not_of(" \t\n\r"));
        key.erase(key.find_last_not_of(" \t\n\r") + 1);

        // Check for the stop command
        if (key == "RUN" || key == "run") {
            break;
        }

        // Don't add empty strings if user just hits enter
        if (key.empty()) {
            continue;
        }

        // Add the valid key to our list
        accesses.push_back(key);
    }

    if (accesses.empty()) {
        std::cout << "No accesses provided. Exiting." << std::endl;
        return 0;
    }

    std::cout << "\n--- Running simulations with " << accesses.size() 
              << " accesses ---" << std::endl;

    // 3. Create and run the LRU simulator
    CacheSimulator sim_lru(capacity, accesses, "LRU");
    sim_lru.run();

    // 4. Create and run the FIFO simulator
    CacheSimulator sim_fifo(capacity, accesses, "FIFO");
    sim_fifo.run();

    // 5. Check the console output! You can now directly compare
    //    the Hit Rates of the two policies for your own workload.

    return 0;
}
*/


