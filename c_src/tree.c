#include "tree.h"
#include <stdlib.h>
#include <stddef.h>
#include "stringmachine.h"
// AVL tree implementation in C

#include <stdio.h>
#include <stdlib.h>

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

// Create a node
struct TreeNode *newNode(int key, int value) {
  struct TreeNode *node = (struct TreeNode *)
    malloc(sizeof(struct TreeNode));
  node->key = key;
  node->value = value;
  node->left = NULL;
  node->right = NULL;
  node->height = 1;
  return (node);
}

// Right rotate
struct TreeNode *rightRotate(struct TreeNode *y) {
  struct TreeNode *x = y->left;
  struct TreeNode *T2 = x->right;

  x->right = y;
  y->left = T2;
  y->height = max(height(y->left), height(y->right)) + 1;
  x->height = max(height(x->left), height(x->right)) + 1;

  return x;
}

// Left rotate
struct TreeNode *leftRotate(struct TreeNode *x) {
  struct TreeNode *y = x->right;
  struct TreeNode *T2 = y->left;

  y->left = x;
  x->right = T2;

  x->height = max(height(x->left), height(x->right)) + 1;
  y->height = max(height(y->left), height(y->right)) + 1;

  return y;
}

// Get the balance factor
int getBalance(struct TreeNode *N) {
  if (N == NULL)
    return 0;
  return height(N->left) - height(N->right);
}

// Insert node
struct TreeNode *push_to_tree(struct TreeNode *node, int key, int value) {
  // Find the correct position to insertNode the node and insertNode it
  if (node == NULL) {
    return (newNode(key, value));
  }
  if (key < node->key) {
    node->left = push_to_tree(node->left, key, value);
  }
  else if (key > node->key) {
    node->right = push_to_tree(node->right, key, value);
  }
  else {
    return node;
  }

  // Update the balance factor of each node and
  // Balance the tree
  node->height = 1 + max(height(node->left),
               height(node->right));

  int balance = getBalance(node);
  if (balance > 1 && key < node->left->key) {
    return rightRotate(node);
  }

  if (balance < -1 && key > node->right->key) {
    return leftRotate(node);
  }

  if (balance > 1 && key > node->left->key) {
    node->left = leftRotate(node->left);
    return rightRotate(node);
  }

  if (balance < -1 && key < node->right->key) {
    node->right = rightRotate(node->right);
    return leftRotate(node);
  }

  return node;
}

struct TreeNode *minValueNode(struct TreeNode *node) {
  struct TreeNode *current = node;

  while (current->left != NULL)
    current = current->left;

  return current;
}

// Delete a nodes
struct TreeNode *remove_from_tree(struct TreeNode *root, int key) {
  // Find the node and delete it
  if (root == NULL)
    return root;

  if (key < root->key)
    root->left = remove_from_tree(root->left, key);

  else if (key > root->key)
    root->right = remove_from_tree(root->right, key);

  else {
    if ((root->left == NULL) || (root->right == NULL)) {
      struct TreeNode *temp = root->left ? root->left : root->right;

      if (temp == NULL) {
        temp = root;
        root = NULL;
      } else
        *root = *temp;
      free(temp);
    } else {
      struct TreeNode *temp = minValueNode(root->right);

      root->key = temp->key;
      root->value = temp->value;

      root->right = remove_from_tree(root->right, temp->key);
    }
  }

  if (root == NULL)
    return root;

  // Update the balance factor of each node and
  // balance the tree
  root->height = 1 + max(height(root->left),
               height(root->right));

  int balance = getBalance(root);
  if (balance > 1 && getBalance(root->left) >= 0)
    return rightRotate(root);

  if (balance > 1 && getBalance(root->left) < 0) {
    root->left = leftRotate(root->left);
    return rightRotate(root);
  }

  if (balance < -1 && getBalance(root->right) <= 0)
    return leftRotate(root);

  if (balance < -1 && getBalance(root->right) > 0) {
    root->right = rightRotate(root->right);
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
    return value_for_key(root->left, key);
  if (key > root->key)
    return value_for_key(root->right, key);
  return -1;
}

void printPreOrder(struct TreeNode *root) {
  if (root != NULL) {
    printf("%d:%s->%i ", root->key, get_string(root->key), root->value);
    printPreOrder(root->left);
    printPreOrder(root->right);
  }
}
