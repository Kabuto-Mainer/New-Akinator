#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GK_DSLLib.h"
#include "GK_TreeType.h"
#include "GK_AllFunc.h"


constexpr static int GK_MAX_SIZE_KIND = 20;
constexpr static int GK_MAX_SIZE_TEXT = 1024;

// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

static void gk_add_token(GK_TokenContext *cont, GK_Token token) {
    assert(cont);

    if (cont->data.capacity == 0) {
        cont->data.tokens = (GK_Token *)calloc(10, sizeof(GK_Token));
        if (cont->data.tokens == NULL)  ExitF("NULL Calloc", );

        cont->data.capacity = 10;
        cont->data.size = 0;
    }

    if (cont->data.capacity == cont->data.size) {
        cont->data.tokens = (GK_Token *)realloc(cont->data.tokens, sizeof(GK_Token) * ((size_t) cont->data.capacity * 2));
        if (cont->data.tokens == NULL)  ExitF("NULL Realloc", );

        cont->data.capacity *= 2;
    }
    cont->data.tokens[cont->data.size++] = token;
    // printf("%s\n", token.text ? token.text : "NULL");
}

// ----------------------------------------------------------------------
static void gk_add_data(GK_Node *node, GK_NodeData data) {
    assert(node);

    if (node->data_amount == 0) {
        if (node->data != NULL)     free(node->data);

        node->data = (GK_NodeData *)calloc(1, sizeof(GK_NodeData));
        if (node->data == NULL)     ExitF("NULL Calloc", );
    } else {
        if (node->data == NULL)     ExitF("NULL Data", );

        node->data = (GK_NodeData *)realloc(node->data, sizeof(GK_NodeData) * ((size_t) node->data_amount + 1));
        if (node->data == NULL)     ExitF("NULL Realloc", );
    }

    node->data[node->data_amount++] = data;
}

// ----------------------------------------------------------------------
static void gk_add_choice(GK_Node *node, GK_NodeChoice next) {
    assert(node);

    if (node->next.with_choice.amount == 0) {
        if (node->next.with_choice.next != NULL)     free(node->next.with_choice.next);

        node->next.with_choice.next = (GK_NodeChoice *)calloc(1, sizeof(GK_NodeChoice));
        if (node->next.with_choice.next == NULL)     ExitF("NULL Calloc", );
    } else {
        if (node->next.with_choice.next == NULL)     ExitF("NULL Data", );

        node->next.with_choice.next = (GK_NodeChoice *)realloc(node->next.with_choice.next,
            sizeof(GK_NodeChoice) * ((size_t) node->next.with_choice.amount + 1));
        if (node->next.with_choice.next == NULL)     ExitF("NULL Realloc", );
    }

    node->next.with_choice.next[node->next.with_choice.amount++] = next;
}

// ----------------------------------------------------------------------
static void gk_add_node(GK_Tree *tree, GK_Node *node) {
    assert(tree);
    assert(node);

    if (tree->capacity == 0) {
        tree->nodes = (GK_Node *)calloc(10, sizeof(GK_Node));
        if (tree->nodes == NULL)    ExitF("NULL Calloc", );
        tree->capacity = 10;
    }

    if (tree->capacity == tree->amount) {
        tree->nodes = (GK_Node *)realloc(tree->nodes, sizeof(GK_Node) * ((size_t) tree->capacity * 2));
        if (tree->nodes == NULL)    ExitF("NULL Realloc", );
        tree->capacity *= 2;
    }
    tree->nodes[tree->amount++] = *node;
    // printf("[%s] %s\n", node->id, node->kind == GK_NODE_NEXT ? node->next.without_choice.name : "CHOICE");
}

// ----------------------------------------------------------------------
static inline void gk_skip_void(GK_TokenContext *cont) {
    assert(cont);

    while (cont->cur_p < cont->size_buffer &&
           (cont->buffer[cont->cur_p] == '\n' ||
            cont->buffer[cont->cur_p] == '\t' ||
            cont->buffer[cont->cur_p] == ' '  ||
            cont->buffer[cont->cur_p] == '\r')) {
        cont->cur_p++;
    }
}
static void gk_tokenize_buffer(GK_TokenContext *cont) {
    assert(cont);

    char buffer[GK_MAX_SIZE_TEXT] = "";
    int len = 0;

    while (cont->cur_p < cont->size_buffer && cont->buffer[cont->cur_p] != '\0') {
        gk_skip_void(cont);

        if (cont->cur_p >= cont->size_buffer || cont->buffer[cont->cur_p] == '\0') {
            break;
        }

        GK_Token token = {};
        len = 0;
        buffer[0] = '\0';

        switch (cont->buffer[cont->cur_p]) {
            case '"':
                if (sscanf(cont->buffer + cont->cur_p, "\"%[^\"]\"%n", buffer, &len) != 1) {
                    ExitF("Bad string token", );
                }
                // printf("TOKEN RAW: [%s]\n", buffer);
                cont->cur_p += len;

                token.kind = GK_NODE_TOKEN_STRING;
                token.text = strdup(buffer);
                // printf("%s\n", token.text);
                break;

            case '{':
                token.kind = GK_NODE_TOKEN_LBRACE;
                cont->cur_p++;
                break;

            case '}':
                token.kind = GK_NODE_TOKEN_RBRACE;
                cont->cur_p++;
                break;

            default: {
                if (sscanf(cont->buffer + cont->cur_p, "%1023s%n", buffer, &len) != 1) {
                    ExitF("Bad token read", );
                }
                cont->cur_p += len;

                uint64_t hash = gk_get_hash(buffer);

                switch (hash) {
                    case gk_get_hash("step"):
                        token.kind = GK_NODE_TOKEN_STEP;
                        break;
                    case gk_get_hash("text:"):
                        token.kind = GK_NODE_TOKEN_TEXT;
                        break;
                    case gk_get_hash("img:"):
                        token.kind = GK_NODE_TOKEN_IMG;
                        break;
                    case gk_get_hash("smp:"):
                        token.kind = GK_NODE_TOKEN_SMP;
                        break;
                    case gk_get_hash("go:"):
                        token.kind = GK_NODE_TOKEN_GO;
                        break;
                    case gk_get_hash("next:"):
                        token.kind = GK_NODE_TOKEN_NEXT;
                        break;
                    case gk_get_hash("choice"):
                        token.kind = GK_NODE_TOKEN_CHOICE;
                        break;
                    default:
                        token.kind = GK_NODE_TOKEN_NAME;
                        token.text = strdup(buffer);
                        // printf("NAME: %s\n", buffer);
                        break;
                }
                break;
            }
        }

        // printf("TOKEN kind=%d text=%s\n", token.kind, token.text ? token.text : "NULL");
        gk_add_token(cont, token);
    }
}

static inline GK_Token get_t(GK_TokenContext *cont) {
    assert(cont);
    assert(cont->data.tokens);
    assert(cont->data.capacity >= 0);
    assert(cont->data.capacity < cont->data.size);
    return cont->data.tokens[cont->data.capacity];
}

static inline void next_t(GK_TokenContext *cont) {
    if (cont->data.size != cont->data.capacity) cont->data.capacity++;
}

static void gk_parse_step(GK_TokenContext *cont, GK_Node *node) {
    assert(cont);
    assert(node);

    if (get_t(cont).kind != GK_NODE_TOKEN_STEP)     ExitF("Bad Parsing Step", );
    next_t(cont);

    if (get_t(cont).kind != GK_NODE_TOKEN_LBRACE)   ExitF("Skipped { in file", );

    GK_NodeData node_data = {};
    bool running = true;

    while (running) {
        next_t(cont);
        switch ((int) get_t(cont).kind) {
            case (int) GK_NODE_TOKEN_RBRACE:
                running = false;
                break;

            case (int) GK_NODE_TOKEN_TEXT:
                next_t(cont);
                if (get_t(cont).kind != GK_NODE_TOKEN_STRING) {
                    ExitF("Not String After \"Text:\"", );
                }
                node_data.set |= GK_NODE_HAVE_TEXT;
                node_data.text = get_t(cont).text;
                break;


            case (int) GK_NODE_TOKEN_IMG:
                next_t(cont);
                if (get_t(cont).kind != GK_NODE_TOKEN_STRING) {
                    ExitF("Not String After \"Img:\"", );
                }
                node_data.set |= GK_NODE_HAVE_IMAGE;
                node_data.img = get_t(cont).text;
                break;


            case (int) GK_NODE_TOKEN_SMP:
                next_t(cont);
                if (get_t(cont).kind != GK_NODE_TOKEN_STRING) {
                    ExitF("Not String After \"Smp:\"", );
                }
                node_data.set |= GK_NODE_HAVE_SAMPLE;
                node_data.smp = get_t(cont).text;
                break;

            default:
                ExitF("Incorrect Token In Step", );
        }
    }

    if (get_t(cont).kind != GK_NODE_TOKEN_RBRACE)   ExitF("Skipper } in file", );
    next_t(cont);

    gk_add_data(node, node_data);
    return ;
}

static void gk_parse_choice(GK_TokenContext *cont, GK_Node *node) {
    assert(cont);
    assert(node);

    if (get_t(cont).kind != GK_NODE_TOKEN_CHOICE)     ExitF("Bad Parsing Choice", );
    next_t(cont);

    if (get_t(cont).kind != GK_NODE_TOKEN_LBRACE)   ExitF("Skipped { in file", );

    GK_NodeChoice node_choice = {};
    bool running = true;

    while (running) {
        next_t(cont);
        switch ((int) get_t(cont).kind) {
            case (int) GK_NODE_TOKEN_RBRACE:
                running = false;
                break;

            case (int) GK_NODE_TOKEN_TEXT:
                next_t(cont);
                if (get_t(cont).kind != GK_NODE_TOKEN_STRING) {
                    ExitF("Not String After \"Text:\"", );
                }
                node_choice.text = get_t(cont).text;
                break;


            case (int) GK_NODE_TOKEN_GO:
                next_t(cont);
                if (get_t(cont).kind != GK_NODE_TOKEN_STRING) {
                    ExitF("Not String After \"Go:\"", );
                }
                node_choice.target.name = get_t(cont).text;
                break;

            default:
                ExitF("Incorrect Token In Step", );
        }
    }

    if (get_t(cont).kind != GK_NODE_TOKEN_RBRACE)   ExitF("Skipper } in file", );
    next_t(cont);

    gk_add_choice(node, node_choice);
    node->kind = GK_NODE_CHOICE;

    return ;
}

static void gk_parse_token(GK_TokenContext *cont, GK_Tree *tree) {
    assert(cont);
    assert(tree);

    int last_capacity = cont->data.capacity;
    cont->data.capacity = 0;    // !!!! Capacity is counter of elements

    while (cont->data.capacity < cont->data.size) {
        GK_Node node = {};

        if (get_t(cont).kind != GK_NODE_TOKEN_NAME)     ExitF("Skipped Name", );
        node.id = get_t(cont).text;
        node.is_checked = false;
        // printf("NODE_TEXT: %s\n", node.id);
        next_t(cont);

        if (get_t(cont).kind != GK_NODE_TOKEN_LBRACE)   ExitF("Skipped { in file", );
        next_t(cont);

        bool running = true;
        while (running) {
            switch (get_t(cont).kind) {
                case GK_NODE_TOKEN_STEP:
                    gk_parse_step(cont, &node);
                    break;

                case GK_NODE_TOKEN_CHOICE:
                    // printf("CHOICE: %s\n", node.id);
                    gk_parse_choice(cont, &node);
                    break;

                case GK_NODE_TOKEN_NEXT:
                    next_t(cont);

                    if (get_t(cont).kind != GK_NODE_TOKEN_STRING) {
                        ExitF("Skipped string after \"next:\"", );
                    }
                    printf("NODE: %s\n NEXT: %s\n", node.id, get_t(cont).text);
                    node.next.without_choice.name = get_t(cont).text;
                    node.kind = GK_NODE_NEXT;
                    next_t(cont);

                    break;

                case GK_NODE_TOKEN_RBRACE:
                    running = false;
                    break;

                case GK_NODE_TOKEN_NAME:
                case GK_NODE_TOKEN_LBRACE:
                case GK_NODE_TOKEN_TEXT:
                case GK_NODE_TOKEN_IMG:
                case GK_NODE_TOKEN_SMP:
                case GK_NODE_TOKEN_GO:
                case GK_NODE_TOKEN_STRING:
                default:
                    ExitF("Bad Token In Stream", );
            }
        }
        if (get_t(cont).kind != GK_NODE_TOKEN_RBRACE)   ExitF("Skipped }", );
        next_t(cont);

        gk_add_node(tree, &node);
    }
    cont->data.capacity = last_capacity;
    return ;
}

static GK_Node *gk_find_node(GK_Tree *tree, const char *name) {
    assert(tree);
    assert(name);

    // printf("FindName: %s\n", name);
    uint64_t hash = gk_get_hash(name);

    for (int i = 0; i < tree->amount; i++) {
        uint64_t hash_to_cmp = gk_get_hash(tree->nodes[i].id);
        if (hash_to_cmp == hash && strcmp(name, tree->nodes[i].id) == 0) {
            return &(tree->nodes[i]);
        }
    }

    return NULL;
}

static void gk_fill_id(GK_Tree *tree, GK_Node *node) {
    assert(tree);
    assert(node);

    // printf("NODE: %s\n", node->id);
    if (node->is_checked == true)   return ;

    node->is_checked = true;
    if (node->kind == GK_NODE_NEXT || node->kind == GK_NODE_END) {
        if (node->kind == GK_NODE_END)  return ;
        char *name = node->next.without_choice.name;
        node->next.without_choice.ptr = gk_find_node(tree, name);

        if (node->next.without_choice.ptr == NULL) {
            printf("ID: %s, Name: %s\n", node->id, name);
            ExitF("Can not find this block", );
        }

        if (strcmp("end", name) == 0) {
            node->next.without_choice.ptr->kind = GK_NODE_END;
        }

        free(name);
        gk_fill_id(tree, node->next.without_choice.ptr);
    } else if (node->kind == GK_NODE_CHOICE) {
        for (int i = 0; i < node->next.with_choice.amount; i++) {
            char *name = node->next.with_choice.next[i].target.name;
            node->next.with_choice.next[i].target.ptr = gk_find_node(tree, name);

            if (node->next.with_choice.next[i].target.ptr == NULL) {
                printf("Name: %s\n", name);
                ExitF("Can not find this block", );
            }

            if (strcmp("end", name) == 0) {
                node->next.with_choice.next[i].target.ptr->kind = GK_NODE_END;
            }

            free(name);
            gk_fill_id(tree, node->next.with_choice.next[i].target.ptr);
        }
    }
}

static void gk_fill_id(GK_Tree *tree) {
    assert(tree);

    tree->start = gk_find_node(tree, "start");
    if (tree->start == NULL)    ExitF("Can not find start step", );

    // for (int i = 0; i < tree->amount; i++) {
    //     printf("[%3d]{%s}\n", i, tree->nodes[i].id);
    // }

    gk_fill_id(tree, tree->start);
    return ;
}

void GK_LoadTree(GK_Tree *tree, const char *name) {
    assert(tree);
    assert(name);

    int size = gk_get_file_size(name);
    char *buffer = gk_create_file_buffer(name, size);

    GK_TokenContext cont = {.buffer = buffer, .cur_p = 0, .size_buffer = size,
                            {NULL, 0, 0, 0}};

    gk_tokenize_buffer(&cont);
    free(buffer);

    gk_parse_token(&cont, tree);
    gk_fill_id(tree);

    return ;
}

//
// static GK_TreeObject *gk_parse_tree(char **cur_p);
//
// // ----------------------------------------------------------------------
// static GK_TreeObject *gk_allocate_object(void) {
//     GK_TreeObject *obj = (GK_TreeObject *)calloc(1, sizeof(GK_TreeObject));
//     if (obj == NULL)  ExitF("NULL Calloc", NULL);
//     return obj;
// }
//
// // ------------------------------------------------------------------
// // String processing
// static constexpr uint64_t gk_get_hash(const char *buffer) {
//     int idx = 0;
//     uint64_t hash = 5137;
//
//     while (buffer[idx] != '\0') {
//         hash = hash * 33 + (uint64_t)(buffer[idx++]);
//     }
//     return hash;
// }
//
// static void gk_skip_void(char **cur_p) {
//     assert(cur_p);
//     assert(*cur_p);
//
//     while (**cur_p == ' ' || **cur_p == '\t' || **cur_p == '\n') {
//         (*cur_p)++;
//     }
//     return ;
// }
//
// // ------------------------------------------------------------------
// static void gk_destroy_object(GK_TreeObject *obj) {
//     assert(obj);
//
//     if (obj->set & GK_TREE_OBJECT_HAVE_TEXT) {
//         free(obj->text);
//     }
//     if (obj->set & GK_TREE_OBJECT_HAVE_IMAGE) {
//         free(obj->files.img);
//     }
//     if (obj->set & GK_TREE_OBJECT_HAVE_SAMPLE) {
//         free(obj->files.smp);
//     }
//
//     if (obj->kind == GK_TREE_OBJECT_BRANCH) {
//         if (obj->branch.yes != NULL)    gk_destroy_object(obj->branch.yes);
//         if (obj->branch.no != NULL)    gk_destroy_object(obj->branch.no);
//     }
//     free(obj);
//     return ;
// }
//
//
// // ----------------------------------------------------------------------
// static GK_TreeObject *gk_parse_tree(char **cur_p) {
//     assert(cur_p);
//     assert(*cur_p);
//
//     gk_skip_void(cur_p);
//
//     // printf("STR: %s\n", *cur_p);
//     if (**cur_p != '{') {
//         if (strncmp(*cur_p, "nil", 3) == 0) {
//             *cur_p += 3;
//             gk_skip_void(cur_p);
//             return NULL;
//         }
//         ExitF("NO Argument", NULL);
//     }
//     (*cur_p)++;
//     gk_skip_void(cur_p);
//
//     // printf("STR_1: %s\n", *cur_p);
//
//     char kind[GK_MAX_SIZE_KIND] = "";
//     char text[GK_MAX_SIZE_TEXT] = "";
//     int len = 0;
//
//     if (sscanf(*cur_p, "\"%[^\"]\" %n", text, &len) != 1) ExitF("Bad Config Tree", NULL);
//     *cur_p += len;
//
//     // printf("STR_2: %s\n", *cur_p);
//     GK_TreeObject *obj = gk_allocate_object();
//
//     obj->text = strdup(text);
//     obj->set |= GK_TREE_OBJECT_HAVE_TEXT;
//
//     while (true) {
//         gk_skip_void(cur_p);
//         if (**cur_p != ',')     break;
//         (*cur_p)++;
//
//         gk_skip_void(cur_p);
//         if (sscanf(*cur_p, "%[^:]:\"%[^\"]\" %n", kind, text, &len) != 2) {
//             free(obj->text);
//             free(obj);
//             ExitF("Bad Config Tree", NULL);
//         }
//         *cur_p += len;
//
//         uint64_t hash = gk_get_hash(kind);
//         switch (hash) {
//             case gk_get_hash("img"): {
//                 obj->set |= GK_TREE_OBJECT_HAVE_IMAGE;
//                 obj->files.img = strdup(text);
//                 break;
//             }
//
//             case gk_get_hash("mus"): {
//                 obj->set |= GK_TREE_OBJECT_HAVE_SAMPLE;
//                 obj->files.smp = strdup(text);
//                 break;
//             }
//             default: {
//                 free(obj->text);
//                 free(obj);
//                 ExitF("Unknown kind", NULL);
//             }
//         }
//     }
//
//     obj->branch.no = gk_parse_tree(cur_p);
//     obj->branch.yes = gk_parse_tree(cur_p);
//
//     if (obj->branch.no == NULL && obj->branch.yes == NULL) {
//         obj->kind = GK_TREE_OBJECT_LEAF;
//     } else {
//         obj->kind = GK_TREE_OBJECT_BRANCH;
//     }
//
//     if (**cur_p != '}') {
//         free(obj->text);
//         free(obj);
//         ExitF("No '}'", NULL);
//     }
//     (*cur_p)++;
//     gk_skip_void(cur_p);
//
//     return obj;
// }
//
//
// // ====================================================================
// // MAIN TREE FUNCTIONS
// // ====================================================================
//
// ----------------------------------------------------------------------
void GK_InitTree(GK_Tree *tree) {
    assert(tree);

    return ;
}
//
// // ----------------------------------------------------------------------
// void GK_LoadTree(GK_Tree *tree, const char *name) {
//     assert(tree);
//     assert(name);
//
//     int size = gk_get_file_size(name);
//     char *buffer = gk_create_file_buffer(name, size);
//
//     char *cur_p = buffer;
//     GK_TreeObject *null = gk_parse_tree(&cur_p);
//
//     free(buffer);
//
//     if (tree->null != NULL) {
//         gk_destroy_object(tree->null);
//     }
//
//     tree->null = null;
//     tree->cur = null;
//
//     return ;
// }
//
// void GK_TreeMoveData(GK_TreeObject *first, GK_TreeObject *second) {
//     assert(first);
//     assert(second);
//
//     GK_TreeObject buffer = {.branch = first->branch,
//                             .files = first->files,
//                             .kind = first->kind,
//                             .set = first->set,
//                             .text = first->text};
//
//     first->branch = second->branch;
//     first->files = second->files;
//     first->kind = second->kind;
//     first->set = second->set;
//     first->text = second->text;
//
//     second->branch = buffer.branch;
//     second->files = buffer.files;
//     second->kind = buffer.kind;
//     second->set = buffer.set;
//     second->text = buffer.text;
//
//     return ;
// }ы
//
// GK_TreeObject *GK_TreeCreateObject(const char *text) {
//     assert(text);
//
//     GK_TreeObject *obj = gk_allocate_object();
//     obj->kind = GK_TREE_OBJECT_LEAF;
//     obj->set = GK_TREE_OBJECT_HAVE_TEXT;
//     obj->text = strdup(text);
//
//     return obj;
// }
//
// // void GK_TreeAddObject(GK_TreeObject *root, GK_TreeObject *new_obj, bool side) {
// //     assert(root);
// //     assert(new_obj);
// //
// //     if (side) {
// //         if (root.)
// //     }
// // }
