#ifndef LIST_H
#define LIST_H
#include "uthash.h"

typedef struct ListNode {
  int type;
  int value;
  struct ListNode *next;
  struct ListNode *prev;
} ListNode;

typedef struct ListEntry {
  int id;
  struct ListNode *head;
  int size;
  int owners;
  UT_hash_handle hh;
} ListEntry;

int add_list();
int len_list(int id);
int add_to_list(int id, int type, int value);
ListNode *get_from_list(int id, int index);
void remove_from_list(int id, int type, int value);
int count_in_list(int id, int type, int value);
int in_list(int id, int type, int value);
void delete_list(int id);
void remove_owner(int id);
void add_owner(int id);
void clear_ownerless_lists();
int get_num_lists();
void print_list(int id);
#endif