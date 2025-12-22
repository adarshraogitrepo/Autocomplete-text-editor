CC = gcc

CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lws2_32

OBJ = build/server.o build/trie.o build/heap.o build/dictionary.o build/util.o
TARGET = build/server.exe

all: $(TARGET)

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	
build/%.o: src/%.c
	if not exist build mkdir build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	if exist build rmdir /s /q build