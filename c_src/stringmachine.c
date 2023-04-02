#include "stringdata.h"
#include "stringmachine.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utlist.h"

StringIndexer* indexers = NULL;

void concat_strings(int idx1, int idx2) {
    const char* str1 = STRINGS[idx1];
    const char* str2 = STRINGS[idx2];
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int total_len = len1 + len2 + 1;
    char* concat_str = (char*)malloc(total_len);
    strcpy(concat_str, str1);
    strcat(concat_str, str2);

    DYNAMIC_STRINGS++;
    D_STRINGS = (const char**)realloc(D_STRINGS, DYNAMIC_STRINGS * sizeof(const char*));
    D_STRINGS[DYNAMIC_STRINGS-1] = concat_str;
    add_indexer(concat_str, NUM_STRINGS+DYNAMIC_STRINGS-1);
}

StringIndexer* _get_last() {
  StringIndexer *si = indexers;
  if (si == NULL) return NULL;
  while (si->next != NULL) si = si->next;
  return si;
}

const char* get_string(int idx) {
  if (idx < 0) return NULL;
  if (idx < NUM_STRINGS) return STRINGS[idx];
  if (idx < NUM_STRINGS + DYNAMIC_STRINGS) return D_STRINGS[idx - NUM_STRINGS];
  return NULL;
}

void add_indexer(char* key, int idx) {
  StringIndexer *si = malloc(sizeof(StringIndexer));
  StringIndexer *end = _get_last();
  if (end != NULL && end->key[0] == key[0] && end->key[1] == key[1]) return;
  int len = strlen(key);
  if (len>0)
    si->key[0] = key[0];
  if (len>1)
    si->key[1] = key[1];
  else
    si->key[1] = '\0';
  si->key[2] = '\0';
  si->idx = idx;
  DL_APPEND(indexers, si);
}

int index_string(char* string) {
  int len = strlen(string);
  if (len==0) return -1;
  StringIndexer *si;
  DL_FOREACH(indexers, si) {
    if (si->key[0] != string[0]) continue;
    if (len > 1 && si->key[1] != string[1]) continue;
    int starter = si->idx;
    int tocheck = si->next != NULL ? si->next->idx - starter : NUM_STRINGS - starter;
    for (int i = 0; i < tocheck; i++) {
      if (starter + 1 < NUM_STRINGS + DYNAMIC_STRINGS) {
	if (strcmp(STRINGS[starter + i], string) == 0) return starter + i;
      } else {
	if (strcmp(D_STRINGS[(starter + i) - NUM_STRINGS], string) == 0) return starter + i;
      }  
    }
  }
  return -1;
}

