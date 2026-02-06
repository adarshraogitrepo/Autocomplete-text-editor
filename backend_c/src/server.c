#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "trie.h"
#include "heap.h"
#include "dictionary.h"

#define PORT 8080
#define BUFFER_SIZE 4096

typedef struct {
    int queries;
    int selections;
    int inserts;
    int deletes;
} Stats;

Stats stats = {0, 0, 0, 0};

#define CACHE_SIZE 20

typedef struct {
    char prefix[128];
    int k;
    char response[BUFFER_SIZE];
    int valid;
} CacheEntry;

CacheEntry prefixCache[CACHE_SIZE];
int cachePtr = 0;


int start_server(int port);
void handle_client(SOCKET client, TrieNode *root);
void handle_query(SOCKET client, TrieNode *root, const char *request);
void handle_select(TrieNode *root, const char *request);
void send_response(SOCKET client, const char *body);
char* cache_get(const char *prefix, int k) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (prefixCache[i].valid &&
            prefixCache[i].k == k &&
            strcmp(prefixCache[i].prefix, prefix) == 0) {
            return prefixCache[i].response;
        }
    }
    return NULL;
}

void cache_put(const char *prefix, int k, const char *response) {
    strcpy(prefixCache[cachePtr].prefix, prefix);
    prefixCache[cachePtr].k = k;
    strcpy(prefixCache[cachePtr].response, response);
    prefixCache[cachePtr].valid = 1;

    cachePtr = (cachePtr + 1) % CACHE_SIZE;
}

void cache_clear() {
    memset(prefixCache, 0, sizeof(prefixCache));
}
int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    TrieNode *root = trie_create_node();
    int loaded = load_dictionary("data/words_alpha.txt", root);
    if (loaded < 0) {
        printf("Failed to load dictionary\n");
        return 1;
    }

    SOCKET server = start_server(PORT);
    printf("Server running on port %d with %d words\n", PORT, loaded);

 while (1) {
    SOCKET client = accept(server, NULL, NULL);

    if (client == INVALID_SOCKET) {
        printf("accept failed\n");
        continue;
    }

    printf("Client connected\n");
    handle_client(client, root);
}

    trie_free(root);
    WSACleanup();
    return 0;
}

int start_server(int port) {
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind failed");
    return -1;
}
printf("Bind successful\n");
    listen(server, 10);
    printf("Listening on port 8080...\n");
    return server;
}

void handle_insert(TrieNode *root, const char *request) {
    stats.inserts++;
    char word[128] = {0};

    const char *word_start = strstr(request, "word=");
    if (word_start) {
        word_start += 5; // Skip "word="
        int i = 0;
        while (i < 127 && word_start[i] && word_start[i] != ' ') {
            word[i] = word_start[i];
            i++;
        }
        word[i] = '\0';
    }
    
    if (word[0] == '\0') return;

    trie_insert(root, word);   
    cache_clear();             
}


void handle_delete(TrieNode *root, const char *request) {
    stats.deletes++;
    char word[128] = {0};

    const char *word_start = strstr(request, "word=");
    if (word_start) {
        word_start += 5; // Skip "word="
        int i = 0;
        while (i < 127 && word_start[i] && word_start[i] != ' ') {
            word[i] = word_start[i];
            i++;
        }
        word[i] = '\0';
    }

    if (word[0] == '\0') return;

    trie_delete_word(root, word);
    cache_clear();

}

void handle_trie(SOCKET client, TrieNode *root) {
    const char *msg =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 7\r\n"
        "Connection: close\r\n"
        "\r\n"
        "TRIE_OK";

    send(client, msg, strlen(msg), 0);
}


void handle_client(SOCKET client, TrieNode *root) {
    char buffer[4096];
    int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0) {
        closesocket(client);
        return;
    }

    buffer[bytes] = '\0';
    printf("Request:\n%s\n", buffer);

    if (strncmp(buffer, "GET /query", 10) == 0) {
        handle_query(client, root, buffer);
    }
    else if (strncmp(buffer, "GET /insert", 11) == 0) {
        handle_insert(root, buffer);
        send_response(client, "OK");
    }
    else if (strncmp(buffer, "GET /delete", 11) == 0) {
        handle_delete(root, buffer);
        send_response(client, "OK");
    }
    else if (strncmp(buffer, "GET /select", 11) == 0) {
        handle_select(root, buffer);
        send_response(client, "OK");
    }
    else if (strncmp(buffer, "GET /stats", 10) == 0) {
    handle_stats(client);
    }
    else if (strncmp(buffer, "GET /trie", 9) == 0) {
        handle_trie(client, root);
    }
    else {
        const char *msg =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        send(client, msg, strlen(msg), 0);
    }

    closesocket(client);
}



void handle_query(SOCKET client, TrieNode *root, const char *request) {
    stats.queries++;
    char prefix[128] = {0};
    int k = 0;
    
    // Parse prefix parameter
    const char *prefix_start = strstr(request, "prefix=");
    if (prefix_start) {
        prefix_start += 7; // Skip "prefix="
        int i = 0;
        while (i < 127 && prefix_start[i] && prefix_start[i] != '&' && prefix_start[i] != ' ') {
            prefix[i] = prefix_start[i];
            i++;
        }
        prefix[i] = '\0';
    }
    
    // Parse k parameter
    const char *k_start = strstr(request, "k=");
    if (k_start) {
        sscanf(k_start, "k=%d", &k);
    }

    if (prefix[0] == '\0' || k <= 0) {
        send_response(client, "Invalid parameters");
        return;
    }
    char *cached = cache_get(prefix, k);
    if (cached != NULL) {
        printf("[CACHE HIT] prefix=%s k=%d\n", prefix, k);
        send_response(client, cached);
        return;
    }
    printf("[CACHE MISS] prefix=%s k=%d\n", prefix, k);

    MinHeap *heap = autocomplete_top_k(root, prefix, k);

    char response[BUFFER_SIZE];
    response[0] = '\0';

    if (heap) {
        for (int i = 0; i < heap->size; i++) {
            strcat(response, heap->data[i].word);
            strcat(response, "\n");
        }
        heap_free(heap);
    }
    cache_put(prefix, k, response);
    send_response(client, response);
}

void handle_select(TrieNode *root, const char *request) {
    stats.selections++;
    char word[128] = {0};

    const char *word_start = strstr(request, "word=");
    if (word_start) {
        word_start += 5; // Skip "word="
        int i = 0;
        while (i < 127 && word_start[i] && word_start[i] != ' ') {
            word[i] = word_start[i];
            i++;
        }
        word[i] = '\0';
    }

    if (word[0] == '\0') return;

    trie_increment_frequency(root, word);

    cache_clear();
}

void send_response(SOCKET client, const char *body) {
    char header[512];
    int body_len = (int)strlen(body);

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        body_len
    );

    send(client, header, strlen(header), 0);
    send(client, body, body_len, 0);
}


void get_request_line(char *buffer, char *line) {
    int i = 0;
    while (buffer[i] && !(buffer[i] == '\r' && buffer[i+1] == '\n')) {
        line[i] = buffer[i];
        i++;
    }
    line[i] = '\0';
}
void handle_stats(SOCKET client) {
    char response[256];

    snprintf(
        response,
        sizeof(response),
        "queries=%d\nselections=%d\ninserts=%d\ndeletes=%d\n",
        stats.queries,
        stats.selections,
        stats.inserts,
        stats.deletes
    );

    send_response(client, response);
}