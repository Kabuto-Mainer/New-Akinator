#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "type.h"
#include "DSL_lib.h"


void GK_InitTree(GK_Tree *tree) {
    assert(tree);

    tree->null = sup_GK_AllocateObject();
    tree->cur = tree->null;
    tree->size = 1;

    // sup_GK_UploadData(tree);

    return ;
}


static GK_TreeObject * sup_GK_AllocateObject(void) {
    GK_TreeObject *obj = (GK_TreeObject *)calloc(1, sizeof(GK_TreeObject));
    if (obj == NULL)  EXIT_F("NULL Calloc", NULL);
    return obj;
}

void sup_GK_GoToLeaf(GK_Display *disp, GK_TreeObject *obj) {
    assert(disp);
    assert(obj);


}

