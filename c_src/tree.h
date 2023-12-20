#ifndef TREE_IMPORT
#define TREE_IMPORT

typedef struct TreeNode {
    int idx;
    int key;
    int value;
    int height;
    int left;
    int right;
} TreeNode;

void init_tree_nodes();
struct TreeNode *push_to_tree(struct TreeNode *node, int key, int value);
int value_for_key(struct TreeNode* root, int key);
struct TreeNode *remove_from_tree(struct TreeNode *root, int key);
void printPreOrder(struct TreeNode *root);
void printTreeMemState();
void free_whole_tree(struct TreeNode *root);
struct TreeNode *getNodeForIdx(int idx);
#endif
