#ifndef HASHTABLE_H
#define HASHTABLE_H


typedef struct Entry {
  bool occupied;
  int fullhash;
  int score;
  int depth;
} Entry;

typedef struct HashTable {
  Entry* entries;
  int size_pow;
  int count;
} HashTable;

void init_hashtable(HashTable* table);
void free_hashtable(HashTable* table);
void grow_hashtable(HashTable* table);

Entry* lookup_hashtable(HashTable* table, const Board* board);
void insert_hashtable(HashTable* table, const Board* board, int score, int depth);
#endif
