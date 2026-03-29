#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GK_DSLLib.h"
#include "GK_TreeType.h"
#include "GK_AllFunc.h"


constexpr static int GK_MAX_SIZE_KIND = 20;
constexpr static int GK_MAX_SIZE_TEXT = 64;


// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

static GK_TreeObject *gk_parse_tree(char **cur_p);

// ----------------------------------------------------------------------
static GK_TreeObject *gk_allocate_object(void) {
    GK_TreeObject *obj = (GK_TreeObject *)calloc(1, sizeof(GK_TreeObject));
    if (obj == NULL)  ExitF("NULL Calloc", NULL);
    return obj;
}

// ------------------------------------------------------------------
// String processing
static constexpr uint64_t gk_get_hash(const char *buffer) {
    int idx = 0;
    uint64_t hash = 5137;

    while (buffer[idx] != '\0') {
        hash = hash * 33 + (uint64_t)(buffer[idx++]);
    }
    return hash;
}

static void gk_skip_void(char **cur_p) {
    assert(cur_p);
    assert(*cur_p);

    while (**cur_p == ' ' || **cur_p == '\t' || **cur_p == '\n') {
        (*cur_p)++;
    }
    return ;
}

// ------------------------------------------------------------------
static void gk_destroy_object(GK_TreeObject *obj) {
    assert(obj);

    if (obj->set & GK_TREE_OBJECT_HAVE_TEXT) {
        free(obj->text);
    }
    if (obj->set & GK_TREE_OBJECT_HAVE_IMAGE) {
        free(obj->files.img);
    }
    if (obj->set & GK_TREE_OBJECT_HAVE_SAMPLE) {
        free(obj->files.smp);
    }

    if (obj->kind == GK_TREE_OBJECT_BRANCH) {
        if (obj->branch.yes != NULL)    gk_destroy_object(obj->branch.yes);
        if (obj->branch.no != NULL)    gk_destroy_object(obj->branch.no);
    }
    free(obj);
    return ;
}

// ====================================================================
// MAIN TREE FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
void GK_InitTree(GK_Tree *tree) {
    assert(tree);

    tree->null = gk_allocate_object();
    tree->cur = tree->null;
    tree->null->kind = GK_TREE_OBJECT_LEAF;
    tree->null->set = GK_TREE_OBJECT_HAVE_TEXT;
    tree->null->text = strdup("Это ДЖИНН?");


    // sup_GK_UploadData(tree);

    return ;
}

// ----------------------------------------------------------------------
void GK_LoadTree(GK_Tree *tree, const char *name) {
    assert(tree);
    assert(name);

    int size = gk_get_file_size(name);
    char *buffer = gk_create_file_buffer(name, size);

    char *cur_p = buffer;
    GK_TreeObject *null = gk_parse_tree(&cur_p);

    free(buffer);

    if (tree->null != NULL) {
        gk_destroy_object(tree->null);
    }

    tree->null = null;
    tree->cur = null;

    return ;
}

// ----------------------------------------------------------------------
static GK_TreeObject *gk_parse_tree(char **cur_p) {
    assert(cur_p);
    assert(*cur_p);

    gk_skip_void(cur_p);

    // printf("STR: %s\n", *cur_p);
    if (**cur_p != '{') {
        if (strncmp(*cur_p, "nil", 3) == 0) {
            *cur_p += 3;
            gk_skip_void(cur_p);
            return NULL;
        }
        ExitF("NO Argument", NULL);
    }
    (*cur_p)++;
    gk_skip_void(cur_p);

    // printf("STR_1: %s\n", *cur_p);

    char kind[GK_MAX_SIZE_KIND] = "";
    char text[GK_MAX_SIZE_TEXT] = "";
    int len = 0;

    if (sscanf(*cur_p, "\"%[^\"]\" %n", text, &len) != 1) ExitF("Bad Config Tree", NULL);
    *cur_p += len;

    // printf("STR_2: %s\n", *cur_p);
    GK_TreeObject *obj = gk_allocate_object();

    obj->text = strdup(text);
    obj->set |= GK_TREE_OBJECT_HAVE_TEXT;

    while (true) {
        gk_skip_void(cur_p);
        if (**cur_p != ',')     break;
        (*cur_p)++;

        gk_skip_void(cur_p);
        if (sscanf(*cur_p, "%[^:]:\"%[^\"]\" %n", kind, text, &len) != 2) {
            free(obj->text);
            free(obj);
            ExitF("Bad Config Tree", NULL);
        }
        *cur_p += len;

        uint64_t hash = gk_get_hash(kind);
        switch (hash) {
            case gk_get_hash("img"): {
                obj->set |= GK_TREE_OBJECT_HAVE_IMAGE;
                obj->files.img = strdup(text);
                break;
            }

            case gk_get_hash("mus"): {
                obj->set |= GK_TREE_OBJECT_HAVE_SAMPLE;
                obj->files.smp = strdup(text);
                break;
            }
            default: {
                free(obj->text);
                free(obj);
                ExitF("Unknown kind", NULL);
            }
        }
    }

    obj->branch.no = gk_parse_tree(cur_p);
    obj->branch.yes = gk_parse_tree(cur_p);

    if (obj->branch.no == NULL && obj->branch.yes == NULL) {
        obj->kind = GK_TREE_OBJECT_LEAF;
    } else {
        obj->kind = GK_TREE_OBJECT_BRANCH;
    }

    if (**cur_p != '}') {
        free(obj->text);
        free(obj);
        ExitF("No '}'", NULL);
    }
    (*cur_p)++;
    gk_skip_void(cur_p);

    return obj;
}


