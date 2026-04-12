#ifndef TREE_TYPE_H
#define TREE_TYPE_H

#include "GK_GraphicType.h"

enum GK_TokenKind {
    GK_NODE_TOKEN_LBRACE,
    GK_NODE_TOKEN_RBRACE,
    GK_NODE_TOKEN_STEP,
    GK_NODE_TOKEN_TEXT,
    GK_NODE_TOKEN_IMG,
    GK_NODE_TOKEN_SMP,
    GK_NODE_TOKEN_CHOICE,
    GK_NODE_TOKEN_GO,
    GK_NODE_TOKEN_NEXT,
    GK_NODE_TOKEN_STRING,
    GK_NODE_TOKEN_NAME
};

struct GK_Token {
    GK_TokenKind kind;
    char *text;
};

struct GK_TokenContext {
    char *buffer;
    int cur_p;
    int size_buffer;

    struct {
        GK_Token *tokens;
        int size;
        int capacity;
        int pos;
    } data;
};


enum GK_NodeDataSetting {
    GK_NODE_HAVE_TEXT   = 0x0001,
    GK_NODE_HAVE_IMAGE  = 0x0002,
    GK_NODE_HAVE_SAMPLE = 0x0004,
};

enum GK_NodeKind {
    GK_NODE_CHOICE,
    GK_NODE_NEXT,
    GK_NODE_END
};

struct GK_NodeData {
    char *text;
    char *img;
    char *smp;
    int set;
};

struct GK_Node;

struct GK_NodeChoice {
    char *text;

    union {
        char *name;
        GK_Node *ptr;
    } target;
};

struct GK_Node {
    char *id;

    GK_NodeData *data;
    int data_amount;
    bool is_checked;

    GK_NodeKind kind;

    union {
        struct {
            GK_NodeChoice *next;
            int amount;
        } with_choice;

        union {
            char *name;
            GK_Node *ptr;
        } without_choice;
    } next;
};

struct GK_Tree {
    GK_Node *nodes;
    int amount;
    int capacity;

    GK_Node *start;
};

struct GK_Runtime {
    GK_Node *cur;
};

#endif /* TREE_TYPE_H */
