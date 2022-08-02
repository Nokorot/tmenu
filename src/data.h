

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
    size_t size;
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

typedef size_t bitmap_packet;

struct BiTree {
    BiNode *root;
    size_t size;
    BiNode *nodes;
    size_t capacity;

    // BitMap
    size_t next;
    bitmap_packet *mask;

    bitree_cmp_func compare;
};

struct BiNode {
    void *data;
    struct BiNode *left, *right;
};

// Create a new empty BiTeee, with compare function set to 'compare.
BiTree *bitree_new(bitree_cmp_func compare, size_t capacity);

// Adds 'data to the BiTree 'bt.
//   Returns true, if a new node was created and false if
//   a node whos data equals 'data, wrt. the compare function.
//
//   In either case 'new_nd is set to point at the corresonding node.
bool bitree_add(BiTree *bt, void *data, BiNode **new_nd);


bool bitree_rm(BiTree *bt, void *data);

bool bitree_find(BiTree *bt, void *data, BiNode **new_nd);

void itter(BiNode *nd, bool (*func)(void *data));

// ******************************************
// *********** Stack ************************
// ******************************************



