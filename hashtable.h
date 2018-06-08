/*
Copyright 2018 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

typedef struct Entry {
  bool occupied;
  uint64_t fullhash;
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
