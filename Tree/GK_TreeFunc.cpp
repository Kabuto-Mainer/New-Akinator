#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GK_DSLLib.h"
#include "GK_TreeType.h"
#include "GK_AllFunc.h"


// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
static GK_TreeObject *gk_allocate_object(void) {
    GK_TreeObject *obj = (GK_TreeObject *)calloc(1, sizeof(GK_TreeObject));
    if (obj == NULL)  ExitF("NULL Calloc", NULL);
    return obj;
}

// ====================================================================
// MAIN TREE FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
void GK_InitTree(GK_Tree *tree) {
    assert(tree);

    tree->null = gk_allocate_object();
    tree->cur = tree->null;
    tree->size = 1;

    // sup_GK_UploadData(tree);

    return ;
}
