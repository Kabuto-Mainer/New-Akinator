#ifndef TYPE_H
#define TYPE_H

#include "GK_MusicType.h"
#include "GK_GraphicType.h"
#include "GK_TreeType.h"


// ====================================================================
// ENUMS
// ====================================================================

// ------------------------------------------------------------------
enum GK_MenuKind {
    GK_MENU_INIT = 0,
    GK_MENU_START_GUESS,
    GK_MENU_GUESS,
    GK_MENU_SUCCESS,
    GK_MENU_N_SUCCESS,
    GK_MENU_ADMIN_MENU,
    GK_MENU_EXIT_MENU,
};

// ====================================================================
// STRUCTS
// ====================================================================

// ------------------------------------------------------------------
struct GK_Display {
    GK_GraphicSystem sys;
    GK_ObjectPool *data;
    GK_Menu menus[GK_AMOUNT_MENU];
    GK_MenuKind cur_menu;
    GK_Music music;
};

// ------------------------------------------------------------------
struct GK_Main {
    GK_Tree tree;
    GK_Display disp;
};



#endif /* TYPE_H */
