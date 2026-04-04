#ifndef GK_DSL_LIB_H
#define GK_DSL_LIB_H

#include "GK_GraphicType.h"


// НОМЕРА КНОПОК ДЛЯ ВЫБОРОВ ДОЛЖНЫ ЛЕЖАТЬ ПОДРЯД !!!!!!
// ====================================================================
#define XXX constexpr GK_ID

XXX GK_INIT_MAIN_TEXT               = 0;
XXX GK_START_PLAY_MAIN_TEXT         = 1;
XXX GK_VIEW_MAIN_TEXT               = 2;
XXX GK_CHOICE_MAIN_TEXT             = 3;
XXX GK_EXIT_MAIN_TEXT               = 4;

XXX GK_INIT_MAIN_IMG                = 5;
XXX GK_START_PLAY_MAIN_IMG          = 6;
XXX GK_VIEW_MAIN_IMG                = 7;
XXX GK_CHOICE_MAIN_IMG              = 8;
XXX GK_EXIT_MAIN_IMG                = 9;

XXX GK_CHOICE_BUT_0                 = 10;
XXX GK_CHOICE_BUT_1                 = 11;
XXX GK_CHOICE_BUT_2                 = 12;
XXX GK_CHOICE_BUT_3                 = 13;

XXX GK_OBJECT_COUNT                 = 20;
XXX GK_MAX_CHOICE                   = 4;

#undef XXX
// ====================================================================

#endif /* GK_DSL_LIB_H */
