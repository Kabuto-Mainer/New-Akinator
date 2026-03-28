#ifndef GK_PARSER_TYPE_H
#define GK_PARSER_TYPE_H

#include <stdio.h>
#include "GK_GraphicType.h"
#include "GK_DSLLib.h"

// ====================================================================
// CONSTANTS
// ====================================================================

constexpr static int GK_PARSER_MAX_KIND = 64;
constexpr static int GK_PARSER_MAX_PATH = 256;
constexpr static int GK_PARSER_MAX_TEXT = 1024;
constexpr static int GK_PARSER_MAX_FRAMES = 64;
constexpr static int GK_PARSER_MAX_ITEMS = GK_OBJECT_COUNT;

// ====================================================================
// STRUCTS
// ====================================================================

struct GK_Parser {
    char *buffer;
    int cur_p;
    int size;

    int cur_o;
    GK_GraphicObject *pool;
    SDL_Renderer *render;
};



#endif /* GK_PARSER_TYPE_H */
