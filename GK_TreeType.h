#ifndef TREE_TYPE_H
#define TREE_TYPE_H

#include "GK_GraphicType.h"

enum GK_TreeObjectKind {
    GK_TREE_OBJECT_LEAF,
    GK_TREE_OBJECT_BRANCH
};

enum GK_TreeObjectSetting {
    GK_TREE_OBJECT_HAVE_TEXT    = 0x0001,
    GK_TREE_OBJECT_HAVE_IMAGE   = 0x0002,
    GK_TREE_OBJECT_HAVE_SAMPLE  = 0x0004,
};

struct GK_TreeObject {
    GK_TreeObjectKind kind;
    int set;    // check GK_TreeObjectSetting

    char *text;
    struct {
        char *img;
        char *smp;
    } files;

    struct {
        GK_TreeObject *yes;
        GK_TreeObject *no;
    } branch;
};

struct GK_Tree {
    GK_TreeObject *null;
    GK_TreeObject *cur;
    int size;
};

#endif /* TREE_TYPE_H */
