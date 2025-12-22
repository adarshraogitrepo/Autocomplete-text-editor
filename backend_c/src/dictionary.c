#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dictionary.h"

#define MAX_WORD_LEN 128

int load_dictionary(const char *filename, TrieNode *root) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open dictionary file");
        return -1;
    }

    char buffer[MAX_WORD_LEN];
    int count = 0;

    while (fgets(buffer, sizeof(buffer), file)) {

        buffer[strcspn(buffer, "\r\n")] = '\0';


        if (buffer[0] == '\0')
            continue;

        trie_insert(root, buffer);
        count++;
    }

    fclose(file);
    return count;
}
