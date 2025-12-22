#ifndef HEAP_H
#define HEAP_H

#define MAX_WORD_LENGTH 64

typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
} HeapNode;

typedef struct {
    HeapNode *data;
    int size;
    int capacity;
} MinHeap;

MinHeap* heap_create(int capacity);
void heap_insert(MinHeap *heap, const char *word, int frequency);
void heap_free(MinHeap *heap);
void heap_push(MinHeap *heap, HeapNode node);

#endif
