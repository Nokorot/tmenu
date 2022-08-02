#include <stdlib.h>
#include <assert.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "data.h"

// ******************************************
// *********** LList (linked list) **********
// ******************************************

LList *ll_new() {
    LList *ll = malloc(sizeof(LList));
    ll->size = 0;
    return ll;
}

LLNode *ll_new_node(void *data) {
    LLNode *nd = malloc(sizeof(LLNode));
    nd->data = data;
    return nd;
}

void ll_add(LList *ll, void *data, int index) {
    if (ll->size == 0) {
        LLNode *nd = ll_new_node(data);
        nd->prev = nd->next = NULL;
        ll->first = ll->last = nd;
        ll->size++;
        return;
    }

    LLNode *nd = ll_new_node(data);
    if (index == 0) {
        nd->prev = NULL;
        nd->next = ll->first;
        ll->first = nd;
        ll->size++;
        return;
    }
 
    LLNode *prev;
    // If index >= size-1 or index < 0, add at the end.
    if (index >= ll->size || index < 0) {
        fprintf(stdout, "-A\n");
        prev = ll->last;
        ll->last = nd;
    } else if (index > ll->size/2) {
        fprintf(stdout, "-B\n");
        prev = ll->last;
        for (int i=ll->size; i > index; --i) 
            prev = prev->prev;
    } else {
        fprintf(stdout, "-C\n");
        prev = ll->first;
        for (int i=1; i < index; ++i) 
            prev = prev->next;
    }

    assert(prev); // "ERROR: 'prev' node is null, in 'll_add'"

    nd->prev = prev;
    nd->next = prev->next;
    prev->next = nd;
    if (nd->next) 
        nd->next->prev = nd;
    ll->size++;
}

// void ll_nodes_array(LList *ll, )
// void ll_as_array(LList *ll) { }

/*  Test:
int main() {
    LList *ll = ll_new();
    
    ll_add(ll, "A", 1);   
    ll_add(ll, "B", 1);   
    ll_add(ll, "C", 1);   
    ll_add(ll, "D", 1);   
    ll_add(ll, "F", 1);   
    ll_add(ll, "E", 1);   

    fprintf(stdout, "###\n");

    for (LLNode *nd=ll->first; nd; nd = nd->next) {
        fprintf(stdout, "%s\n", nd->data);
    }

    fprintf(stdout, "###\n");
     

    for (LLNode *nd=ll->last; nd; nd = nd->prev) {
        fprintf(stdout, "%s\n", nd->data);
    }
} */



// ******************************************
// *********** BiTree (binary tree) *********
// ******************************************

BiTree *bitree_new(bitree_cmp_func compare) {
    BiTree *bt = malloc(sizeof(BiTree));
    bt->compare = compare;
    bt->root = NULL;
    bt->size = 0;
    return bt;
}

BiNode *binode_new(void *data) {
    BiNode *nd = malloc(sizeof(BiNode));
    nd->left = nd->right = NULL;
    nd->data = data;
    return nd;
}

bool binode_add(BiTree *bt, BiNode *nd, void *data, BiNode **new_nd) {
    assert(nd);

    int cmp = bt->compare(data, nd->data);
    if (cmp < 0) {
        if (nd->left) 
            return binode_add(bt, nd->left, data, new_nd);
        nd->left = binode_new(data);
        if (new_nd) *new_nd = nd->left;
        bt->size++;
        return true;
    } else if (cmp > 0) {
        if (nd->right) 
            return binode_add(bt, nd->right, data, new_nd);
        nd->right = binode_new(data);
        if (new_nd) *new_nd = nd->right;
        bt->size++;
        return true;
    }
    if (new_nd) *new_nd = nd;
    return false;
}

bool bitree_add(BiTree *bt, void *data, BiNode **new_nd) {
    if (!bt->root) {
        bt->root = binode_new(data);
        if (new_nd) *new_nd = bt->root;
        bt->size = 1;
        return true;
    }
    return binode_add(bt, bt->root, data, new_nd);
}

bool binode_find(BiTree *bt, BiNode *nd, void *data, BiNode **new_nd) {
    // printf("'%s'", *((char **) nd->data));

    int cmp = bt->compare(data, nd->data);
    if (cmp < 0) {
        if (nd->left) 
            return binode_find(bt, nd->left, data, new_nd);
        return false;
    } else if (cmp > 0) {
        if (nd->right) 
            return binode_find(bt, nd->right, data, new_nd);
        return false;
    }
    if (new_nd) *new_nd = nd;
    return true;
}

bool bitree_find(BiTree *bt, void *data, BiNode **new_nd) {
    if (!bt->root)
        return false;
    return binode_find(bt, bt->root, data, new_nd);
}

void attachLeft(BiNode *nd, BiNode *nda) {
    assert(nd); 
    assert(nda);
    // if (!nda) return;

    if (!nd->left) {
        nd->left = nda;
    } else {
        attachRight(nda, nd->left);
        nd->left = nda;
    }
    return;
}

void attachRight(BiNode *nd, BiNode *nda) {
    assert(nd); 
    assert(nda);
    // if (!nda) return;

    if (!nd->right) {
        nd->right = nda;
    } else {
        attachLeft(nda, nd->right);
        nd->right = nda;
    }
    return;
}

bool binode_rm(BiTree *bt, BiNode *nd, void *data, bool *found) {
    int cmp = bt->compare(data, nd->data);
    if (cmp < 0) {
        if (nd->left && binode_rm(bt, nd->left, data, found)) {
            // Del nd->left
            BiNode *ll = nd->left->left;
            free(nd->left);
            nd->left = ll;
        }
        return false;
    } else if (cmp > 0) {
        if (nd->right && binode_rm(bt, nd->right, data, found)) {
            BiNode *rl = nd->right->left;
            free(nd->right);
            nd->right = rl;
        }
        return false;
    }

    *found = true;
    // Put everything on the left side
    if (nd->right) {
        if (nd->left)
            attachRight(nd->left, nd->right);
        else 
            nd->left = nd->right;
    }

    return true;
}

bool bitree_rm(BiTree *bt, void *data) {
    if (!bt->root) {
        return false;
    }
    bool *found = false;
    if (binode_rm(bt, bt->root, data, found)) {
        

    }
    return *found;
}

void itter(BiNode *nd, bool (*func)(void *)) {
    if (nd->left)
        itter(nd->left, func);
    func(nd->data);
    if (nd->right) 
        itter(nd->right, func);
}

/* //  Test BiTree:
bool str_print(char *str) {
    printf("%s\n", str);
}

int main() {
    BiTree *bt = bitree_new((bitree_cmp_func) strcmp);
    
    if (bitree_add(bt, "A", NULL)) { printf("TRUE A");  }
    if (bitree_add(bt, "B", NULL)) { printf("TRUE B");  }
    if (bitree_add(bt, "C", NULL)) { printf("TRUE C");  }
    if (bitree_add(bt, "C", NULL)) { printf("TRUE C");  }
    if (bitree_add(bt, "D", NULL)) { printf("TRUE D");  }
    if (bitree_add(bt, "F", NULL)) { printf("TRUE F");  }
    if (bitree_add(bt, "E", NULL)) { printf("TRUE E");  }

    fprintf(stdout, "###\n");

    itter(bt->root, str_print);


    fprintf(stdout, "###\n");
     

} */



