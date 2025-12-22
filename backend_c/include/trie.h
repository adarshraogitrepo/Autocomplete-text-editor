#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

#define ALPHABET_SIZE 26

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    bool is_end_of_word;
    int frequency;              
} TrieNode;


TrieNode* trie_create_node(void); 
void trie_insert(TrieNode *root, const char *word); 
bool trie_search(const TrieNode *root, const char *word); 
TrieNode* trie_find_prefix(const TrieNode *root, const char *prefix); 
bool trie_increment_frequency(TrieNode *root, const char *word);
void trie_free(TrieNode *root);
bool trie_delete_word(TrieNode *root, const char *word);


#include "heap.h"

void trie_collect_words(TrieNode *node,
                        char *buffer,
                        int depth,
                        MinHeap *heap);

MinHeap* autocomplete_top_k(TrieNode *root,
                            const char *prefix,
                            int k);

bool trie_insert_word(TrieNode *root, const char *word);

void trie_to_json(TrieNode *node, char *buffer, int *offset);



#endif
