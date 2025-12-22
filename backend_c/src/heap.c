#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heap.h"

static void swap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

static void heapify_up(MinHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->data[parent].frequency <= heap->data[index].frequency)
            break;

        swap(&heap->data[parent], &heap->data[index]);
        index = parent;
    }
}

static void heapify_down(MinHeap *heap, int index) {
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < heap->size &&
            heap->data[left].frequency < heap->data[smallest].frequency) {
            smallest = left;
        }

        if (right < heap->size &&
            heap->data[right].frequency < heap->data[smallest].frequency) {
            smallest = right;
        }

        if (smallest == index)
            break;

        swap(&heap->data[index], &heap->data[smallest]);
        index = smallest;
    }
}

MinHeap* heap_create(int capacity) {
    MinHeap *heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) {
        perror("Failed to allocate heap");
        exit(EXIT_FAILURE);
    }

    heap->data = (HeapNode*)malloc(sizeof(HeapNode) * capacity);
    if (!heap->data) {
        perror("Failed to allocate heap data");
        exit(EXIT_FAILURE);
    }

    heap->size = 0;
    heap->capacity = capacity;

    return heap;
}

void heap_push(MinHeap *heap, HeapNode node) {
    if (heap->size < heap->capacity) {
        heap->data[heap->size] = node;
        int i = heap->size;
        heap->size++;

        while (i > 0) {
            int parent = (i - 1) / 2;

            if (heap->data[parent].frequency > heap->data[i].frequency)
                break;

            if (heap->data[parent].frequency == heap->data[i].frequency &&
                strcmp(heap->data[parent].word, heap->data[i].word) < 0)
                break;

            HeapNode temp = heap->data[parent];
            heap->data[parent] = heap->data[i];
            heap->data[i] = temp;

            i = parent;
        }
    }
}


void heap_insert(MinHeap *heap, const char *word, int frequency) {
    if (heap->capacity == 0)
        return;

    if (heap->size < heap->capacity) {
        strncpy(heap->data[heap->size].word, word, MAX_WORD_LENGTH - 1);
        heap->data[heap->size].word[MAX_WORD_LENGTH - 1] = '\0';
        heap->data[heap->size].frequency = frequency;

        heapify_up(heap, heap->size);
        heap->size++;
    }

    else if (frequency > heap->data[0].frequency) {
        strncpy(heap->data[0].word, word, MAX_WORD_LENGTH - 1);
        heap->data[0].word[MAX_WORD_LENGTH - 1] = '\0';
        heap->data[0].frequency = frequency;

        heapify_down(heap, 0);
    }
}

void heap_free(MinHeap *heap) {
    if (!heap) return;
    free(heap->data);
    free(heap);
}