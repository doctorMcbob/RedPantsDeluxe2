#ifndef STRING_MACHINE_LOAD
#define STRING_MACHINE_LOAD

typedef struct StringIndexer {
  char key[3];
  int idx;
  struct StringIndexer *next;
  struct StringIndexer *prev;
} StringIndexer;

typedef struct DynamicString {
  char* string;
  struct DynamicString *next;
  struct DynamicString *prev;
} DynamicString;

void add_indexer(char* key, int idx);
char* get_string(int idx);
int index_string(char* string);
int add_string(char* string, int skipCheck);
int concat_strings(char* str1, char* str2);
char *int_to_string(int num);
char *float_to_string(float num);
#endif
