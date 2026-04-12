#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "GK_DSLLib.h"
#include "GK_AllFunc.h"
#include "GK_GraphicSystemFunc.h"

// ====================================================================
// USED CONSTANTS
// ====================================================================
extern int GK_SCREEN_WIDTH;
extern int GK_SCREEN_HEIGHT;
extern const char *GK_SYSTEM_FONT;
extern const SDL_Color GK_FONT_COLOR;

// ====================================================================
// THIS FUNCTIONS MUST BE ADJUSTED WHEN CHANGING CONFIGS
// ====================================================================

// ----------------------------------------------------------------------
static GK_ID gk_get_output_text_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_INIT:      return GK_INIT_MAIN_TEXT;
        case GK_MENU_START_PLAY:return GK_START_PLAY_MAIN_TEXT;
        case GK_MENU_VIEW:      return GK_VIEW_MAIN_TEXT;
        case GK_MENU_CHOICE:    return GK_CHOICE_MAIN_TEXT;
        case GK_MENU_EXIT:      return GK_EXIT_MAIN_TEXT;
        default:                return GK_INVALID_ID;
    }
    return GK_INVALID_ID;
}

// ----------------------------------------------------------------------
static GK_ID gk_get_image_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_INIT:      return GK_INIT_MAIN_IMG;
        case GK_MENU_START_PLAY:return GK_START_PLAY_MAIN_IMG;
        case GK_MENU_VIEW:      return GK_VIEW_MAIN_IMG;
        case GK_MENU_CHOICE:    return GK_CHOICE_MAIN_IMG;
        case GK_MENU_EXIT:      return GK_EXIT_MAIN_IMG;
        default:                return GK_INVALID_ID;
    }
    return GK_INVALID_ID;
}


static void gk_display_node(GK_Main *app) {
    assert(app);

    const GK_ID text_win = gk_get_output_text_window(&(app->disp));
    const GK_ID img_win  = gk_get_image_window(&(app->disp));

    // printf("TEXT: %d\n IMG: %d\n", text_win, img_win);

    if (text_win != GK_INVALID_ID) {
        gk_clear_text(&(app->disp), text_win);
    }
    if (img_win != GK_INVALID_ID) {
        gk_clear_image(&(app->disp), img_win);
    }

    char *text = app->cur.node->data[app->cur.step].text;
    char *img = app->cur.node->data[app->cur.step].img;
    char *smp = app->cur.node->data[app->cur.step].smp;
    int set = app->cur.node->data[app->cur.step].set;

    // printf("Text: %s\n", text);

    if (set & GK_NODE_HAVE_TEXT) {
        gk_add_text(&(app->disp), text, text_win);
    }
    if (set & GK_NODE_HAVE_IMAGE) {
        gk_add_image(&(app->disp), img, img_win);
    }
    (void) smp;
    return ;
}

static inline bool gk_is_last_node(GK_Main *app) {
    return app->cur.node->kind == GK_NODE_END;
}

static bool gk_is_last_step(GK_Main *app) {
    return app->cur.step >= app->cur.node->data_amount - 1;
}

static void gk_next_view(GK_Main *app) {
    app->cur.step++;
    return ;
}

static void gk_add_buttons(GK_Main *app) {
    assert(app);

    for (int i = 0; i < GK_MAX_CHOICE; i++) {
        app->disp.data.pool[GK_CHOICE_BUT_0 + i].must_show = false;
    }

    GK_Node *node = app->cur.node;
    for (int i = 0; i < node->next.with_choice.amount; i++) {
        gk_add_text_button(&(app->disp),
            node->next.with_choice.next[i].text, GK_CHOICE_BUT_0 + i);

        app->disp.data.pool[GK_CHOICE_BUT_0 + i].must_show = true;
    }
    return ;
}

static void gk_goto_choice(GK_Main *app, int choice) {
    assert(app);
    if (choice + 1 > GK_MAX_CHOICE)  ExitF("Bad Choice", );

    app->cur.node = app->cur.node->next.with_choice.next[choice].target.ptr;
    app->cur.step = 0;

    app->disp.cur_menu = GK_MENU_VIEW;
    gk_display_node(app);

    return ;
}

// ====================================================================
// MAIN GRAPHIC FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
GK_ActionKind GK_PollAction(GK_Display *disp) {
    assert(disp);

    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return GK_ACTION_EXIT;
        }

        GK_Menu *menu = &(disp->menus[disp->cur_menu]);
        for (int i = 0; i < menu->size; i++) {
            const GK_ID id = menu->data[i];
            if (id < 0 || id >= disp->data.size) {
                continue;
            }

            GK_GraphicObject *obj = &(disp->data.pool[id]);
            if (obj->kind != GK_GRAPHIC_BUTTON || obj->data.but == NULL || obj->must_show == false) {
                continue;
            }

            const GK_ActionKind action = gk_check_click_button(&event, obj->data.but);
            if (action != GK_ACTION_NONE) {
                return action;
            }
        }
    }

    return GK_ACTION_NONE;
}

// ----------------------------------------------------------------------
void GK_Update(GK_Main *app, GK_ActionKind action) {
    assert(app);

    switch (action) {
        case GK_ACTION_NONE:
            return ;

        case GK_ACTION_BEGIN_PLAY:
            app->cur.node = app->tree.start;
            app->cur.step = 0;
            app->disp.cur_menu = GK_MENU_VIEW;
            gk_display_node(app);
            return ;

        case GK_ACTION_SKIP: {
            bool is_last_node = gk_is_last_node(app);
            bool is_last_step = gk_is_last_step(app);

            if (is_last_node) {
                app->disp.cur_menu = GK_MENU_START_PLAY;
                return ;
            }
            if (is_last_step) {
                if (app->cur.node->kind == GK_NODE_NEXT) {
                    app->cur.node = app->cur.node->next.without_choice.ptr;
                    app->cur.step = 0;
                } else {
                    gk_add_buttons(app);
                    app->disp.cur_menu = GK_MENU_CHOICE;
                }
                gk_display_node(app);
                return ;
            }
            gk_next_view(app);
            gk_display_node(app);
            return ;
        }

        case GK_ACTION_RETURN_TO_START: {
            app->disp.cur_menu = GK_MENU_START_PLAY;
            GK_ID text_id = gk_get_output_text_window(&(app->disp));
            gk_clear_text(&(app->disp), text_id);
            gk_add_text(&(app->disp), "Приветствуем тебя в нашей программе Акинатор", text_id);
            return ;
        }

        case GK_ACTION_CHOICE_0:
            gk_goto_choice(app, 0);
            return ;

        case GK_ACTION_CHOICE_1:
            gk_goto_choice(app, 1);
            return ;

        case GK_ACTION_CHOICE_2:
            gk_goto_choice(app, 2);
            return ;

        case GK_ACTION_CHOICE_3:
            gk_goto_choice(app, 3);
            return ;

        case GK_ACTION_CONTROL_MUSIC:
            gk_control_music(&(app->disp));
            return ;

        case GK_ACTION_EXIT:
        default:
            return ;
    }
    return ;
}
