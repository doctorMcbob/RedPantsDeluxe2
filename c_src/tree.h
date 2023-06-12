#ifndef TREE_IMPORT
#define TREE_IMPORT

typedef struct TreeNode {
    int key;
    int value;
    int height;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

struct TreeNode *push_to_tree(struct TreeNode *node, int key, int value);
int value_for_key(struct TreeNode* root, int key);
struct TreeNode *remove_from_tree(struct TreeNode *root, int key);
void printPreOrder(struct TreeNode *root);

#endif