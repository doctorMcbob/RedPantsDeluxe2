#ifndef STRING_DATA_LOAD
#include "stringdata.h"
#endif
#include "stringmachine.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utlist.h"

StringIndexer *indexers = NULL;
DynamicString *dynamic_strings = NULL;

int add_string(char *string, int skipCheck) {
  if (!skipCheck) {
    int i = index_string(string);
    if (i != -1) {
      return i;
    }
  }
  DynamicString *ds = malloc(sizeof(DynamicString));
  ds->string = string;
  int idx = NUM_STRINGS + DYNAMIC_STRINGS;
  DYNAMIC_STRINGS++;
  DL_APPEND(dynamic_strings, ds);
  add_indexer(string, idx);
  return idx;
}

int concat_strings(char *str1, char *str2) {
  if (str1 == NULL || str2 == NULL)
    return -1;

  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int total_len = len1 + len2 + 1;
  char *concat_str = (char *)malloc(total_len);
  strcpy(concat_str, str1);
  strcat(concat_str, str2);
  int idx = index_string(concat_str);
  if (idx == -1) {
    int idx = add_string(concat_str, 1);
    return idx;
  }
  free(concat_str);
  return idx;
}

StringIndexer *_get_last() {
  StringIndexer *si = indexers;
  if (si == NULL)
    return NULL;
  while (si->next != NULL)
    si = si->next;
  return si;
}

char *get_string(int idx) {
  if (idx < 0)
    return NULL;
  if (idx < NUM_STRINGS)
    return (char *)STRINGS[idx];
  if (idx < NUM_STRINGS + DYNAMIC_STRINGS) {
    DynamicString *ds;
    int d_idx = idx - NUM_STRINGS;
    DL_FOREACH(dynamic_strings, ds) {
      if (d_idx-- != 0)
        continue;
      return ds->string;
    }
  }
  return NULL;
}

void add_indexer(char *key, int idx) {
  StringIndexer *si = malloc(sizeof(StringIndexer));
  StringIndexer *end = _get_last();
  if (end != NULL && end->key[0] == key[0] && end->key[1] == key[1])
    return;
  int len = strlen(key);
  if (len > 0)
    si->key[0] = key[0];
  if (len > 1)
    si->key[1] = key[1];
  else
    si->key[1] = '\0';
  si->key[2] = '\0';
  si->idx = idx;
  DL_APPEND(indexers, si);
}

int index_string(char *string) {
  int len = strlen(string);
  if (len == 0)
    return -1;
  StringIndexer *si;
  DynamicString *ds = dynamic_strings;
  DL_FOREACH(indexers, si) {
    int starter = si->idx;
    int tocheck = si->next != NULL ? si->next->idx - starter
                                   : NUM_STRINGS + DYNAMIC_STRINGS - starter;
    if (si->key[0] != string[0]) {
      if (starter >= NUM_STRINGS && ds->next != NULL) {
        for (int i = 0; i < tocheck; i++)
          ds = ds->next;
      }
      continue;
    }
    if (len > 1 && si->key[1] != string[1]) {
      if (starter >= NUM_STRINGS && ds->next != NULL) {
        for (int i = 0; i < tocheck; i++)
          ds = ds->next;
      }
      continue;
    }
    for (int i = 0; i < tocheck; i++) {
      if (starter + i < NUM_STRINGS) {
        if (strcmp(STRINGS[starter + i], string) == 0) {
          return starter + i;
        }
      } else {
        if (strcmp(ds->string, string) == 0) {
          return starter + i;
        }
        ds = ds->next;
      }
    }
  }
  return -1;
}

char *int_to_string(int num) {
  int temp = num;
  int digits = 0;

  // Count the number of digits in the number
  do {
    digits++;
    temp /= 10;
  } while (temp != 0);

  if (num < 0)
    digits++;

  // Allocate enough memory to store the string representation of the number
  char *str = malloc((digits + 1) * sizeof(char));

  // Convert the number to a string
  sprintf(str, "%i", num);

  return str;
}

char *float_to_string(float num) {
  int digits_before_decimal = 0;
  int digits_after_decimal = 0;

  // Count the number of digits before and after the decimal point in the number
  int temp = (int)num; // Cast to int to ignore the decimal part
  while (temp != 0) {
    digits_before_decimal++;
    temp /= 10;
  }
  temp = (int)(num * 1000000) %
         1000000; // Get the decimal part and convert it to integer
  while (temp != 0) {
    digits_after_decimal++;
    temp /= 10;
  }

  // Allocate enough memory to store the string representation of the number
  char *str =
      malloc((digits_before_decimal + digits_after_decimal + 2) * sizeof(char));

  // Convert the number to a string
  sprintf(str, "%.6f", num);

  return str;
}
