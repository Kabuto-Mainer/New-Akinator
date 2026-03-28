#ifndef TYPE_H
#define TYPE_H

#include "music_type.h"
#include "graphic_type.h"
#include "tree_type.h"

struct GK_Display {
    GK_GraphicSystem sys;

    GK_ObjectPool *data;
    GK_Menu menus[GK_AMOUNT_MENU];
    int cur_menu;

    GK_Music music;
};

struct GK_Main {
    GK_Tree tree;
    GK_Display disp;
};

enum EVENT {
    GK_EVENT_START,
    GK_EVENT_GUESS,
    GK_EVENT_EXIT,
    GK_EVENT_TO_ADMIN,
    GK_EVENT_TO_BASE
};

#endif /* TYPE_H */
