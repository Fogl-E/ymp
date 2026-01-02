#include "token.h"

struct HashEntry {
    Token token;
    int index;
    HashEntry* next;
    HashEntry(const Token& t, int idx);
};

class HashTable {
private:
    static const int TABLE_SIZE = 100;
    HashEntry** table;
    int currentIndex;
    int hash(const std::string& value) const;
public:
    HashTable();
    ~HashTable();
    int insert(const Token& token);
};

