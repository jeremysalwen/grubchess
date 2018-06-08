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
//2^20
#define QUEUE_CAPACITY 1048576

struct Queue {
  QueueEntry* entries;
  int size;
  int capactiy;
  bool minheap;
};

void create_queue(Queue* queue, int capacity, bool minheap) {
  queue->entries = malloc(capacity * sizeof(QueueEntry));
  queue->size = 0;
  queue->capacity = capacity;
  queue->minheap = minheap;
}

int heap_parent(int index) {
  return (index-1) / 2;
}
void push_up_queue(Queue* queue, int index) {
  if(index == 0) return;
  int parent_index = heap_parent(index);
  if(queue->entries[parent_index].score < queue->entries[index].score) {
    swap_entries(queue->entries + parent_index, queue->entries + index);
    heapify_queue(queue, parent_index);
  }
};

void push_down_queue(Queue* queue, int index) {
  int bestscore = entries[index].score;
  int bestchild=1;
  int child1 = index*2;
  int child2 = index*2+1;
  if(child2<queue->size) {
    bestscore = MAX(bestscore, queue->entries[child2].score);
  }

  if(child1<queue->size) {
    if(queue->entries[child1].score>bestscore) {
      swap_entries(index, child1);
      push_down_queue(queue, child1);
    }
  }
  if(entries[index].score < bestscore) {
    swap_entries(index, child2);
    push_down_queue(queue, child2);
  }
}

void insert_queue(Queue* queue, const QueueEntry* entry) {
  int ind = queue->size++;
  queue->entries[ind] = *entry;
  queue->entries[ind].score *= -1;
  push_up_queue(queue, ind);
}

void pop_queue(Queue* queue) {
  swap_entries(0, queue->size-1);
  queue->size--;
  push_down_queue(queue, 0);
}


void insert_queue_callback(const Board* board, Position from, Position to, void* data) {
  Queue* queue = (Queue*) data;
  QueueEntry entry;
  entry.board = apply_valid_moves(board, from, to);
  entry.score = score(entry.board);
  insert_queue(queue, &entry);
}

void search_n_nodes(const Board* board, int N) {
  Board bestboard;

  Queue queue;
  create_queue(&queue, N*1000, board->move == BLACK);
  QueueEntry entry;
  entry.board = board;
  entry.score = score(board);
  insert_queue(&queue, &entry);
  for(int i=0; i<N; i++) {
    QueueEntry* head = queue.entries;

    valid_moves(board, insert_queue_callback, &queue);

  }
}
