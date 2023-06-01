#include "lists.h"
#include "floatmachine.h"
#include "scripts.h"
#include "stringmachine.h"
#include "uthash.h"
#include "utlist.h"

ListEntry *lists = NULL;
int list_count = 0;

void clear_ownerless_lists() {
  ListEntry *le, *tmp;
  HASH_ITER(hh, lists, le, tmp) {
    if (le->owners != 0)
      continue;
    delete_list(le->id);
  }
}

int get_num_lists() { return HASH_COUNT(lists); }

void add_owner(int id) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return;
  le->owners++;
}

void remove_owner(int id) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return;
  le->owners--;
  if (le->owners == 0)
    delete_list(le->id);
}

int add_list() {
  ListEntry *le = malloc(sizeof(ListEntry));
  le->id = list_count++;
  le->head = NULL;
  le->size = 0;
  le->owners = 0;
  HASH_ADD_INT(lists, id, le);
  return le->id;
}

int len_list(int id) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return -1;
  return le->size;
}

int add_to_list(int id, int type, int value) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return -1;
  ListNode *ln = malloc(sizeof(ListNode));
  ln->value = value;
  ln->type = type;
  DL_APPEND(le->head, ln);
  le->size++;
  return 0;
}

ListNode *get_from_list(int id, int index) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return NULL;
  ListNode *ln;
  DL_FOREACH(le->head, ln) {
    if (index-- != 0)
      continue;
    return ln;
  }
  return NULL;
}

void remove_from_list(int id, int type, int value) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return;
  ListNode *ln, *tmp;
  DL_FOREACH_SAFE(le->head, ln, tmp) {
    if (ln->type != type || ln->value != value)
      continue;
    DL_DELETE(le->head, ln);
    free(ln);
    le->size--;
  }
}

int count_in_list(int id, int type, int value) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return -1;
  int count = 0;
  ListNode *ln;
  DL_FOREACH(le->head, ln) {
    if (ln->type != type || ln->value != value)
      continue;
    count++;
  }
  return count;
}

int in_list(int id, int type, int value) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return -1;
  ListNode *ln;
  DL_FOREACH(le->head, ln) {
    if (ln->type != type || ln->value != value)
      continue;
    return 1;
  }
  return 0;
}

void delete_list(int id) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return;
  ListNode *ln, *tmp;
  DL_FOREACH_SAFE(le->head, ln, tmp) {
    DL_DELETE(le->head, ln);
    free(ln);
  }
  HASH_DEL(lists, le);
  free(le);
}

void print_list(int id) {
  ListEntry *le;
  HASH_FIND_INT(lists, &id, le);
  if (le == NULL)
    return;
  ListNode *ln;
  DL_FOREACH(le->head, ln) {
    switch (ln->type) {
    case INT:
      printf("%i", ln->value);
      break;
    case FLOAT:
      printf("%f", get_float(ln->value));
      break;
    case STRING:
      printf("%s", get_string(ln->value));
      break;
    case LIST:
      print_list(ln->value);
      break;
    }
    if (ln->next != NULL)
      printf(", ");
  }
}