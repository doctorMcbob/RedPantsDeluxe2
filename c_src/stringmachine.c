#include "stringdata.h"
#include "stringmachine.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utlist.h"

StringIndexer* indexers = NULL;

const char* get_string(int idx) {
  if (idx >= NUM_STRINGS || idx < 0) return NULL;
  return STRINGS[idx];
}

void add_indexer(char* key, int idx) {
  StringIndexer *si = malloc(sizeof(StringIndexer));
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
  int starter = -1;
  int tocheck = -1;
  StringIndexer *si;
  DL_FOREACH(indexers, si) {
    if (si->key[0] != string[0]) continue;
    if (len > 1 && si->key[1] != string[1]) continue;
    starter = si->idx;
    break;
  }
  if (starter == -1) return -1;
  tocheck = si->next != NULL ? si->next->idx - starter : NUM_STRINGS - starter;
  for (int i = 0; i < tocheck; i++) {
    if (strcmp(STRINGS[starter + i], string) == 0) return starter + i;
  }
  return -1;
}

