#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

struct Book {
    int id;
    std::string title;
    std::string path;
};

std::vector<Book> books_db;

std::unordered_set<std::string> extractKeywords(const std::string& text) {
    std::unordered_set<std::string> words;
    std::string current;

    for (unsigned char ch : text) {
        if (std::isalpha(ch)) {
            current += static_cast<char>(std::tolower(ch));
        } else {
            if (current.size() >= 3) {
                words.insert(current);
            }
            current.clear();
        }
    }

    if (current.size() >= 3) {
        words.insert(current);
    }

    return words;
}

struct HashNode {
    std::string key;
    std::unordered_set<int> ids;
    HashNode* next;

    HashNode(const std::string& k, int id) : key(k), next(nullptr) {
        ids.insert(id);
    }
};

class LibraryHashTable {
private:
    int cap;
    int used;
    std::vector<HashNode*> table;
    const float maxLoad = 0.7f;

    size_t hashKey(const std::string& key) const {
        return std::hash<std::string>{}(key) % cap;
    }

    void clear() {
        for (HashNode* head : table) {
            while (head) {
                HashNode* temp = head;
                head = head->next;
                delete temp;
            }
        }
    }

    void grow() {
        std::vector<HashNode*> oldTable = table;
        int oldCap = cap;

        cap *= 2;
        table.assign(cap, nullptr);
        used = 0;

        for (int i = 0; i < oldCap; ++i) {
            HashNode* node = oldTable[i];
            while (node) {
                for (int id : node->ids) {
                    insert(node->key, id);
                }
                HashNode* temp = node;
                node = node->next;
                delete temp;
            }
        }
    }

public:
    explicit LibraryHashTable(int initialCap = 8) : cap(initialCap), used(0), table(initialCap, nullptr) {}

    ~LibraryHashTable() {
        clear();
    }

    void insert(const std::string& key, int id) {
        if (static_cast<float>(used) / cap >= maxLoad) {
            grow();
        }

        size_t idx = hashKey(key);
        HashNode* node = table[idx];

        if (!node) {
            table[idx] = new HashNode(key, id);
            ++used;
            return;
        }

        while (node) {
            if (node->key == key) {
                node->ids.insert(id);
                return;
            }
            if (!node->next) break;
            node = node->next;
        }

        node->next = new HashNode(key, id);
        ++used;
    }

    std::unordered_set<int> search(const std::string& key) const {
        size_t idx = hashKey(key);
        HashNode* node = table[idx];

        while (node) {
            if (node->key == key) {
                return node->ids;
            }
            node = node->next;
        }

        return {};
    }
};

class BinarySearchEngine {
private:
    std::vector<std::pair<std::string, int>> index;
    bool ready = false;

public:
    void insert(const std::string& key, int id) {
        index.push_back({key, id});
        ready = false;
    }

    void build() {
        std::sort(index.begin(), index.end());
        ready = true;
    }

    std::unordered_set<int> search(const std::string& key) {
        if (!ready) {
            build();
        }

        std::unordered_set<int> result;
        auto range = std::equal_range(
            index.begin(), index.end(),
            std::make_pair(key, 0),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            }
        );

        for (auto it = range.first; it != range.second; ++it) {
            result.insert(it->second);
        }

        return result;
    }
};

void printResults(const std::unordered_set<int>& results, const std::string& engine, long long time_ns) {
    std::cout << "\n+------------------------------------------------------------------------------+\n";
    std::cout << "| Engine: " << std::left << std::setw(20) << engine
              << "| Time: " << std::setw(15) << time_ns << " ns |\n";
    std::cout << "+------------------------------------------------------------------------------+\n";
    std::cout << "| " << std::left << std::setw(5) << "ID"
              << "| " << std::setw(28) << "Title"
              << "| " << std::setw(40) << "File Path" << "|\n";
    std::cout << "+------------------------------------------------------------------------------+\n";

    if (results.empty()) {
        std::cout << "| " << std::left << std::setw(78) << "No results found." << "|\n";
        std::cout << "+------------------------------------------------------------------------------+\n";
        return;
    }

    for (int id : results) {
        const Book& book = books_db[id - 1];
        std::cout << "| " << std::left << std::setw(5) << book.id
                  << "| " << std::setw(28) << book.title.substr(0, 27)
                  << "| " << std::setw(40) << book.path.substr(0, 39) << "|\n";
    }

    std::cout << "+------------------------------------------------------------------------------+\n";
}

bool loadBooks(const std::string& booksDir, LibraryHashTable& hashEngine, BinarySearchEngine& binaryEngine) {
    if (!fs::exists(booksDir) || !fs::is_directory(booksDir)) {
        std::cerr << "Error: books directory not found. Add a folder named 'books' with .txt files.\n";
        return false;
    }

    int nextId = 1;

    for (const auto& entry : fs::directory_iterator(booksDir)) {
        if (entry.path().extension() != ".txt") {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            continue;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        Book book{nextId, entry.path().stem().string(), entry.path().string()};
        books_db.push_back(book);

        auto words = extractKeywords(content);
        for (const auto& word : words) {
            hashEngine.insert(word, nextId);
            binaryEngine.insert(word, nextId);
        }

        std::cout << "Loaded: " << book.title << " (" << words.size() << " words)\n";
        ++nextId;
    }

    return !books_db.empty();
}

std::string toLowerCopy(std::string text) {
    for (unsigned char& ch : reinterpret_cast<std::basic_string<unsigned char>&>(text)) {
        ch = static_cast<unsigned char>(std::tolower(ch));
    }
    return text;
}

std::string normalizeSearchTerm(std::string term) {
    for (char& ch : term) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return term;
}

int main(int argc, char* argv[]) {
    std::string searchTerm;
    std::string engineType = "hash";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--search" && i + 1 < argc) {
            searchTerm = argv[++i];
        } else if (arg == "--engine" && i + 1 < argc) {
            engineType = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: ./engine --search <word> [--engine <hash|binary|all>]\n";
            return 0;
        }
    }

    if (searchTerm.empty()) {
        std::cerr << "Please enter a search keyword using --search <word>.\n";
        return 1;
    }

    searchTerm = normalizeSearchTerm(searchTerm);

    LibraryHashTable hashEngine;
    BinarySearchEngine binaryEngine;

    if (!loadBooks("books", hashEngine, binaryEngine)) {
        std::cerr << "No .txt files found in the books folder.\n";
        return 1;
    }

    binaryEngine.build();
    std::cout << "Search term: " << searchTerm << "\n";

    if (engineType == "hash" || engineType == "all") {
        auto start = std::chrono::high_resolution_clock::now();
        auto results = hashEngine.search(searchTerm);
        auto end = std::chrono::high_resolution_clock::now();
        auto time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        printResults(results, "Hash Table", time_ns);
    }

    if (engineType == "binary" || engineType == "all") {
        auto start = std::chrono::high_resolution_clock::now();
        auto results = binaryEngine.search(searchTerm);
        auto end = std::chrono::high_resolution_clock::now();
        auto time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        printResults(results, "Binary Search", time_ns);
    }

    return 0;
}
