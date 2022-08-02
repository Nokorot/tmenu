

/// // TODO: Batched llist
/// struct BLList {
///     LLNode *first, *last;
/// 
///     int bachsize;
///     LList *batches;
/// };

// ******************************************
// *********** LList (linked list) **********
// ******************************************

typedef struct LLNode LLNode;
typedef struct LList  LList;

// # Linked List:
struct LLNode {
    // int children;
    void *data;
    struct LLNode *prev, *next;
};

struct LList {
    LLNode *first, *last;
    int size;
};


LList *ll_new();
LLNode *ll_new_node(void *data);
void ll_add(LList *ll, void *data, int index);


// ******************************************
// *********** BiTree (binary tree) *********
// ******************************************

typedef int (*bitree_cmp_func)(void *, void *);
typedef struct BiTree BiTree;
typedef struct BiNode BiNode;

struct BiTree {
    BiNode *root;
    int size;
    bitree_cmp_func compare;
    
    int capacity;
};

struct BiNode {
    void *data;
    struct BiNode *left, *right;
};

// Create a new empty BiTeee, with compare function set to 'compare.
BiTree *bitree_new(bitree_cmp_func compare);

// Create a new empty BiNode with data 'data.
BiNode *binode_new(void *data);

// Adds 'data to the BiTree 'bt.
//   Returns true, if a new node was created and false if
//   a node whos data equals 'data, wrt. the compare function.
//
//   In either case 'new_nd is set to point at the corresonding node.
bool bitree_add(BiTree *bt, void *data, BiNode **new_nd);

// 
bool bitree_find(BiTree *bt, void *data, BiNode **new_nd);


bool bitree_rm(BiTree *bt, void *data);


void itter(BiNode *nd, bool (*func)(void *data));

// ******************************************
// *********** Stack ************************
// ******************************************



