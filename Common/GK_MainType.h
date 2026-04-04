#ifndef GK_MAIN_TYPE_H
#define GK_MAIN_TYPE_H

#include "GK_MusicType.h"
#include "GK_GraphicType.h"
#include "GK_TreeType.h"
#include "GK_ParserType.h"

// ====================================================================
// ENUMS
// ====================================================================

// ------------------------------------------------------------------
enum GK_MenuKind {
    GK_MENU_INIT,
    GK_MENU_START_PLAY,
    GK_MENU_VIEW,
    GK_MENU_CHOICE,
    GK_MENU_EXIT,
};

// ====================================================================
// STRUCTS
// ====================================================================

// ------------------------------------------------------------------
struct GK_Display {
    GK_GraphicSystem sys;
    GK_ObjectPool data;
    GK_TextInput text_inp;

    GK_Menu menus[GK_AMOUNT_MENU];
    GK_MenuKind cur_menu;
    GK_Music music;
};

// ------------------------------------------------------------------
struct GK_Main {
    struct {
        GK_Node *node;
        int step;
    } cur;
    GK_Tree tree;
    GK_Display disp;
};



#endif /* GK_MAIN_TYPE_H */
