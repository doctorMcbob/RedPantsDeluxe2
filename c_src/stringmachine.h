#ifndef STRING_MACHINE_LOAD
#define STRING_MACHINE_LOAD 1

typedef struct StringIndexer {
  char key[3];
  int idx;
  struct StringIndexer *next;
  struct StringIndexer *prev;
} StringIndexer;

void add_indexer(char* key, int idx);
const char* get_string(int idx);
int index_string(char* string);
#endif

