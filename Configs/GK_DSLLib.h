#ifndef GK_DSL_LIB_H
#define GK_DSL_LIB_H

#include "GK_GraphicType.h"

// ====================================================================
#define XXX constexpr GK_ID

XXX GK_INIT_FON_VID                 = 0;
XXX GK_INIT_TEXT                    = 1;

XXX GK_START_GUESS_FON_VID          = 0;
XXX GK_START_GUESS_MAIN_TEXT        = 4;
XXX GK_START_GUESS_GET_ADMIN_BUTTON = 5;
XXX GK_START_GUESS_BEGIN_BUTTON     = 5;
XXX GK_START_GUESS_EXIT_BUTTON      = 6;

XXX GK_GUESS_FON_VID                = 0;
XXX GK_GUESS_YES_BUTTON             = 5;
XXX GK_GUESS_NO_BUTTON              = 6;
XXX GK_GUESS_MAIN_TEXT              = 4;
XXX GK_GUESS_IMAGE                  = 5;

XXX GK_SUCCESS_FON_VID              = 0;
XXX GK_SUCCESS_RETURN_BUTTON        = 5;
XXX GK_SUCCESS_MAIN_TEXT            = 4;
XXX GK_SUCCESS_IMAGE                = 6;

XXX GK_N_SUCCESS_FON_VID            = 0;
XXX GK_N_SUCCESS_MAIN_TEXT          = 4;
XXX GK_N_SUCCESS_RETURN_BUTTON      = 5;
XXX GK_N_SUCCESS_IMAGE              = 6;

XXX GK_ADMIN_MENU_FON_VID           = 0;
XXX GK_ADMIN_MENU_MAIN_TEXT         = 12;
XXX GK_ADMIN_MENU_PSW_TEXT          = 11;
XXX GK_ADMIN_MENU_ADD_BUTTON        = 13;

XXX GK_ADD_MENU_FON_VID             = 0;
XXX GK_ADD_MENU_MAIN_TEXT           = 14;
XXX GK_ADD_MENU_INP_TEXT            = 15;
XXX GK_ADD_MENU_RETURN_BUTTON       = 16;

XXX GK_OBJECT_COUNT                 = 20;

#undef XXX
// ====================================================================

#endif /* GK_DSL_LIB_H */
