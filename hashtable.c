#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "grubchess.h"

#include "hashtable.h"

int pow_to_size(int size_pow) {
  return 1 << size_pow;
}

void init_hashtable(HashTable* table) {
  table->size_pow = 21;
  table->entries = calloc(pow_to_size(table->size_pow), sizeof(Entry));
  table->count = 0;
}

void free_hashtable(HashTable* table) {
  table->size_pow = 0;
  table->count = 0;
  free(table->entries);
}

#define FNV1_OFFSET_BASIS 14695981039346656037ull
#define FNV1_PRIME 1099511628211
uint64_t FNV1Hash(const char* data, int len) {
  uint64_t hash = FNV1_OFFSET_BASIS;
  for(int i=0; i<len; i++) {
    hash ^= data[i];
    hash *=FNV1_PRIME;
  }
  return hash;
}

uint64_t hash_board(const Board* board) {
  return FNV1Hash((const char*)board, sizeof(Board));
}

int get_mask(const HashTable* table) {
  return (1 << table->size_pow) - 1;
}

int hash_to_bucket(const HashTable* table, uint64_t fullhash) {
  return fullhash & get_mask(table);
}

int next_bucket(const HashTable* table, int bucket) {
  return (bucket+1) & get_mask(table);  
}

void do_insert(HashTable* table, uint64_t hash, int score, int depth) {
  int bucket = hash_to_bucket(table, hash);
  // Linear Probing
  while(table->entries[bucket].occupied) {
    bucket = next_bucket(table, bucket);
  }
  table->entries[bucket] = (Entry) {true, hash, score, depth};
}

Entry* lookup_hashtable(HashTable* table, const Board* board) {
  int fullhash = hash_board(board);
  int bucket = hash_to_bucket(table, fullhash);
  while(table->entries[bucket].occupied) {
    if(table->entries[bucket].fullhash == fullhash) {
      return &table->entries[bucket];
    } else {
      bucket = next_bucket(table, bucket);
    }
  }
  return NULL;
}

void insert_hashtable(HashTable* table, const Board* board, int score, int depth) {
  if(++table->count > pow_to_size(table->size_pow)/2) { // Resize at 50% capacity.
    grow_hashtable(table);
  }
  do_insert(table, hash_board(board), score, depth);
}

void grow_hashtable(HashTable* table) {
  int old_size = pow_to_size(table->size_pow);
  int new_size_pow = table->size_pow + 1;
  HashTable newtable = {calloc(pow_to_size(new_size_pow), sizeof(Entry)), new_size_pow, table->count};
  for(int i=0; i<old_size; i++) {
    Entry* entry = table->entries + i;
    if(entry->occupied) {
      do_insert(&newtable, entry->fullhash, entry->score, entry->depth);
    }
  }
  free_hashtable(table);
  *table = newtable;
}
