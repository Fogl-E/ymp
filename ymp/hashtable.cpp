#include "hashtable.h"
#include <fstream>
#include <iostream>

HashEntry::HashEntry(const Token& t, int idx)
    : token(t), index(idx), next(nullptr) {
}
HashTable::HashTable() : currentIndex(0) {
    table = new HashEntry * [TABLE_SIZE];
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = nullptr;
    }
}
HashTable::~HashTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry* entry = table[i];
        while (entry != nullptr) {
            HashEntry* temp = entry;
            entry = entry->next;
            delete temp;
        }
    }
    delete[] table;
}
int HashTable::hash(const std::string& value) const {
    int hash = 0;
    for (char c : value) {
        hash = (hash * 257 + c) % TABLE_SIZE;
    }
    return hash;
}
int HashTable::insert(const Token& token) {
    int hashValue = hash(token.value);
    HashEntry* entry = table[hashValue];
    while (entry != nullptr) {
        if (entry->token.value == token.value && entry->token.type == token.type) {
            return entry->index;
        }
        entry = entry->next;
    }
    HashEntry* newEntry = new HashEntry(token, currentIndex);
    newEntry->next = table[hashValue];
    table[hashValue] = newEntry;
    return currentIndex++;
}

