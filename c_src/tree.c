#include "tree.h"
#include <stdlib.h>
#include <stddef.h>
#include "stringmachine.h"
// AVL tree implementation in C

#include <stdio.h>
#include <stdlib.h>

/*
 * Okay so the way I'm thinking about this is that
 * we allocate all the tree nodes up front.
 *
 * as nodes are pulled (ie newNode) we take them from the heap
 * as nodes are removed (ie remove_node_from_tree) we add the index value of those nodes
 * to CLEARED_NODES and when a new node is pulled, we prioritize nodes in the CLEARED_NODES
 *
 * */

#define NUM_TREE_NODES 100000
struct TreeNode TREE_NODE_HEAP[NUM_TREE_NODES];
int NEXT_NODE = 0;
int CLEARED_NODES[NUM_TREE_NODES];
int CLEARED_NODES_DEPTH = -1;

int max(int a, int b);

// Calculate height
int height(struct TreeNode *N) {
  if (N == NULL) {
    return 0;
  }
  return N->height;
}

int max(int a, int b) {
  return (a > b) ? a : b;
}

void init_tree_nodes() {
  for (int i = 0; i < NUM_TREE_NODES; i++) {
    TREE_NODE_HEAP[i].idx = i;
    TREE_NODE_HEAP[i].height = -1;
    TREE_NODE_HEAP[i].key = -1;
    TREE_NODE_HEAP[i].value = -1;
    TREE_NODE_HEAP[i].left = -1;
    TREE_NODE_HEAP[i].right = -1;
  }
}

// Create a node
struct TreeNode *newNode(int key, int value) {
  struct TreeNode *node;

  if (CLEARED_NODES_DEPTH > -1) {
    node = &TREE_NODE_HEAP[CLEARED_NODES[CLEARED_NODES_DEPTH--]];
  } else {
    if (NEXT_NODE >= NUM_TREE_NODES) {
      printf("Ran out of tree nodes, consider adding more if this is not a memory leak issue\n");
      exit(-1);
    }
    node = &TREE_NODE_HEAP[NEXT_NODE++];
  }

  node->key = key;
  node->value = value;
  node->left = -1;
  node->right = -1;
  node->height = 1;
  return (node);
}

struct TreeNode *getNodeForIdx(int idx) {
   if (idx >= NUM_TREE_NODES || idx < 0) return NULL;
   return &TREE_NODE_HEAP[idx];
}

// Right rotate
struct TreeNode *rightRotate(struct TreeNode *y) {
  struct TreeNode *x = getNodeForIdx(y->left);
  struct TreeNode *T2 = getNodeForIdx(x->right);

  x->right = y == NULL ? -1 : y->idx;
  y->left = T2 == NULL ? -1 : T2->idx;
  y->height = max(height(getNodeForIdx(y->left)), height(getNodeForIdx(y->right))) + 1;
  x->height = max(height(getNodeForIdx(x->left)), height(getNodeForIdx(x->right))) + 1;

  return x;
}

// Left rotate
struct TreeNode *leftRotate(struct TreeNode *x) {
  struct TreeNode *y = getNodeForIdx(x->right);
  struct TreeNode *T2 = getNodeForIdx(y->left);
  
  y->left = x == NULL ? -1 : x->idx;
  x->right = T2 == NULL ? -1 : T2->idx;

  x->height = max(height(getNodeForIdx(x->left)), height(getNodeForIdx(x->right))) + 1;
  y->height = max(height(getNodeForIdx(y->left)), height(getNodeForIdx(y->right))) + 1;

  return y;
}

// Get the balance factor
int getBalance(struct TreeNode *N) {
  if (N == NULL)
    return 0;
  return height(getNodeForIdx(N->left)) - height(getNodeForIdx(N->right));
}

// Insert node
struct TreeNode *push_to_tree(struct TreeNode *node, int key, int value) {
  struct TreeNode *temp = NULL;
  // Find the correct position to insertNode the node and insertNode it
  if (node == NULL) {
    return (newNode(key, value));
  }
  if (key < node->key) {
    temp = push_to_tree(getNodeForIdx(node->left), key, value);
    node->left = temp == NULL ? -1 : temp->idx;
  }
  else if (key > node->key) {
    temp = push_to_tree(getNodeForIdx(node->right), key, value);
    node->right = temp == NULL ? -1 : temp->idx;
  }
  else {
    return node;
  }

  // Update the balance factor of each node and
  // Balance the tree
  node->height = 1 + max(height(getNodeForIdx(node->left)),
               height(getNodeForIdx(node->right)));

  int balance = getBalance(node);
  temp = getNodeForIdx(node->left);
  if (balance > 1 && temp != NULL && key < temp->key) {
    return rightRotate(node);
  }

  temp = getNodeForIdx(node->right);
  if (balance < -1 && temp != NULL && key > temp->key) {
    return leftRotate(node);
  }

  temp = getNodeForIdx(node->left);
  if (balance > 1 && temp != NULL && key > temp->key) {
    temp = leftRotate(getNodeForIdx(node->left));
    node->left = temp == NULL ? -1 : temp->idx;
    return rightRotate(node);
  }

  temp = getNodeForIdx(node->right);
  if (balance < -1 && temp != NULL && key < temp->key) {
    temp = rightRotate(getNodeForIdx(node->right));
    node->right = temp == NULL ? -1 : temp->idx;
    return leftRotate(node);
  }

  return node;
}

struct TreeNode *minValueNode(struct TreeNode *node) {
  struct TreeNode *current = node;

  while (current->left != -1)
    current = getNodeForIdx(current->left);

  return current;
}

// Delete a nodes
struct TreeNode *remove_from_tree(struct TreeNode *root, int key) {
  if (root == NULL)
    return root;
  struct TreeNode *temp = NULL;
  if (key < root->key) {
    temp = remove_from_tree(getNodeForIdx(root->left), key);
    root->left = temp == NULL ? -1 : temp->idx;
  }
  else if (key > root->key) {
    temp = remove_from_tree(getNodeForIdx(root->right), key);
    root->right = temp == NULL ? -1 : temp->idx;
  }
  else {
    if ((root->left == -1) || (root->right == -1)) {
      temp = root->left != -1 ? getNodeForIdx(root->left) : getNodeForIdx(root->right);

      if (temp == NULL) {
        temp = root;
        root = NULL;
      } else {
	root->left = temp->left;
	root->right = temp->right;
	root->key = temp->key;
	root->value = temp->value;
	root->height = temp->height;
      }
      temp->left = -1;
      temp->right = -1;
      temp->key = -1;
      temp->value = -1;
      temp->height = -1;
      CLEARED_NODES[++CLEARED_NODES_DEPTH] = temp->idx;
      
    } else {
      temp = minValueNode(getNodeForIdx(root->right));

      root->key = temp->key;
      root->value = temp->value;

      temp = remove_from_tree(getNodeForIdx(root->right), temp->key);
      root->right = temp == NULL ? -1 : temp->idx;
    }
  }

  if (root == NULL)
    return root;

  // Update the balance factor of each node and
  // balance the tree
  root->height = 1 + max(height(getNodeForIdx(root->left)),
               height(getNodeForIdx(root->right)));

  int balance = getBalance(root);
  if (balance > 1 && getBalance(getNodeForIdx(root->left)) >= 0)
    return rightRotate(root);

  if (balance > 1 && getBalance(getNodeForIdx(root->left)) < 0) {
    temp = leftRotate(getNodeForIdx(root->left));
    root->left = temp == NULL ? -1 : temp->idx;
    return rightRotate(root);
  }

  if (balance < -1 && getBalance(getNodeForIdx(root->right)) <= 0)
    return leftRotate(root);

  if (balance < -1 && getBalance(getNodeForIdx(root->right)) > 0) {
    temp = rightRotate(getNodeForIdx(root->right));
    root->right = temp == NULL ? -1 : temp->idx;
    return leftRotate(root);
  }

  return root;
}

int value_for_key(TreeNode *root, int key) {
  if (root == NULL) 
    return -1;
  if (root->key == key) 
    return root->value;
  if (key < root->key)
    return value_for_key(getNodeForIdx(root->left), key);
  if (key > root->key)
    return value_for_key(getNodeForIdx(root->right), key);
  return -1;
}

void printTreeMemState() {
  printf("NextNode: %i\nClearedNodeDepth: %i\n", NEXT_NODE, CLEARED_NODES_DEPTH);
  if (CLEARED_NODES_DEPTH >= 0) {
    printf("[");
    for (int i=0; i <= CLEARED_NODES_DEPTH; i++) {
      printf("%i, ", CLEARED_NODES[i]);
    }
    printf("]\n");
  }
}

void printPreOrder(struct TreeNode *root) {
  if (root != NULL) {
    printf("   %d(%i):%s->%i L:%i R:%i\n",
	   root->key, root->idx, get_string(root->key), root->value, root->left, root->right);
    printPreOrder(getNodeForIdx(root->left));
    printPreOrder(getNodeForIdx(root->right));
  }
}

void free_whole_tree(struct TreeNode *root) {
  if (root != NULL) {
    free_whole_tree(getNodeForIdx(root->left));
    free_whole_tree(getNodeForIdx(root->right));
    root->left = -1;
    root->right = -1;
    root->key = -1;
    root->value = -1;
    root->height = -1;
    CLEARED_NODES[++CLEARED_NODES_DEPTH] = root->idx;
  }
}

