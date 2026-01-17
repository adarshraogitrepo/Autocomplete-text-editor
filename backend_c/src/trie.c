#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"
#include "heap.h"

TrieNode* trie_create_node(void) {
    TrieNode *node = (TrieNode*)malloc(sizeof(TrieNode));
    if (!node) {
        perror("Failed to allocate TrieNode");
        exit(EXIT_FAILURE);
    }

    node->is_end_of_word = false;
    node->frequency = 0;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }

    return node;
}

void trie_insert(TrieNode *root, const char *word) {
    TrieNode *current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) continue;

        char c = tolower(word[i]);
        int index = c - 'a';

        if (index < 0 || index >= ALPHABET_SIZE) continue;

        if (current->children[index] == NULL) {
            current->children[index] = trie_create_node();
        }

        current = current->children[index];
    }

    if (!current->is_end_of_word) {
        current->is_end_of_word = true;
        current->frequency = 100;  
    }
}


bool trie_search(const TrieNode *root, const char *word) {
    const TrieNode *current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) continue;

        char c = tolower(word[i]);
        int index = c - 'a';

        if (index < 0 || index >= ALPHABET_SIZE) return false;
        if (current->children[index] == NULL) return false;

        current = current->children[index];
    }

    return current->is_end_of_word;
}

TrieNode* trie_find_prefix(const TrieNode *root, const char *prefix) {
    TrieNode *current = (TrieNode*)root;

    for (int i = 0; prefix[i] != '\0'; i++) {
        if (!isalpha(prefix[i])) continue;

        char c = tolower(prefix[i]);
        int index = c - 'a';

        if (index < 0 || index >= ALPHABET_SIZE) return NULL;
        if (current->children[index] == NULL) return NULL;

        current = current->children[index];
    }

    return current;
}

bool trie_increment_frequency(TrieNode *root, const char *word) {
    TrieNode *current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) continue;

        char c = tolower(word[i]);
        int index = c - 'a';

        if (index < 0 || index >= ALPHABET_SIZE) return false;
        if (current->children[index] == NULL) return false;

        current = current->children[index];
    }

    if (current->is_end_of_word) {
        current->frequency++;
        return true;
    }

    return false;
}

void trie_free(TrieNode *root) {
    if (!root) return;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        trie_free(root->children[i]);
    }

    free(root);
}

#include <string.h>
#include "heap.h"

void trie_collect_words(TrieNode *node,
                        char *buffer,
                        int depth,
                        MinHeap *heap) {
    if (!node) return;

    if (node->is_end_of_word) {
        HeapNode hn;
        strcpy(hn.word, buffer);
        hn.frequency = node->frequency;
        heap_push(heap, hn);
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            buffer[depth] = 'a' + i;
            buffer[depth + 1] = '\0';
            trie_collect_words(node->children[i],
                               buffer,
                               depth + 1,
                               heap);
        }
    }
}

MinHeap* autocomplete_top_k(TrieNode *root,
                            const char *prefix,
                            int k) {
    TrieNode *prefix_node = trie_find_prefix(root, prefix);
    if (!prefix_node)
        return NULL;

    MinHeap *heap = heap_create(k);

    char buffer[64];
    strncpy(buffer, prefix, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    trie_collect_words(prefix_node,
                       buffer,
                       strlen(prefix),
                       heap);

    return heap;
}

bool trie_insert_word(TrieNode *root, const char *word) {
    TrieNode *current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) continue;

        char c = tolower(word[i]);
        int index = c - 'a';

        if (index < 0 || index >= ALPHABET_SIZE)
            return false;

        if (current->children[index] == NULL) {
            current->children[index] = trie_create_node();
        }

        current = current->children[index];
    }

    if (!current->is_end_of_word) {
        current->is_end_of_word = true;
        current->frequency = 1;
    }

    return true;
}

bool trie_delete_word(TrieNode *root, const char *word) {
    TrieNode *current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) continue;

        char c = tolower(word[i]);
        int index = c - 'a';

        if (index < 0 || index >= ALPHABET_SIZE)
            return false;

        if (current->children[index] == NULL)
            return false;

        current = current->children[index];
    }

    if (!current->is_end_of_word)
        return false;

    current->is_end_of_word = false;
    current->frequency = 0;

    return true;
}

void trie_to_json(TrieNode *node, char *buffer, int *offset) {
    *offset += snprintf(
        buffer + *offset,
        8192 - *offset,
        "{ \"end\": %s, \"freq\": %d, \"children\": {",
        node->is_end_of_word ? "true" : "false",
        node->frequency
    );

    int first = 1;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            if (!first)
                *offset += snprintf(buffer + *offset, 8192 - *offset, ",");
            first = 0;

            *offset += snprintf(
                buffer + *offset,
                8192 - *offset,
                "\"%c\":",
                'a' + i
            );

            trie_to_json(node->children[i], buffer, offset);
        }
    }

    *offset += snprintf(buffer + *offset, 8192 - *offset, "} }");
}