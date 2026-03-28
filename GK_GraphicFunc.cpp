#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "GK_DSLLib.h"
#include "GK_AllFunc.h"

// ====================================================================
// USED CONSTANTS
// ====================================================================
extern int GK_SCREEN_WIDTH;
extern int GK_SCREEN_HEIGHT;
extern const char *GK_SYSTEM_FONT;
extern const SDL_Color GK_FONT_COLOR;

// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
static SDL_Texture *gk_load_texture(SDL_Renderer *render, const char *name) {
    assert(render);
    assert(name);

    return IMG_LoadTexture(render, name);
}

// ----------------------------------------------------------------------
static void gk_show_image(SDL_Renderer *render, SDL_Texture *tex,
                const SDL_Rect *place) {
    assert(render);
    assert(tex);
    assert(place);

    SDL_RenderCopy(render, tex, nullptr, place);
    return ;
}

// ----------------------------------------------------------------------
static inline bool gk_point_in_rect(int x, int y, const SDL_Rect &rect) {
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
}

// ----------------------------------------------------------------------
static GK_ActionKind gk_check_click_button(SDL_Event *event, GK_GraphicButton *but) {
    assert(event);
    assert(but);

    if (event->type == SDL_MOUSEMOTION) {
        but->is_pressed = false;
        but->is_hovered = gk_point_in_rect(event->motion.x, event->motion.y, but->place);
        return GK_ACTION_NONE;
    }

    if (event->type != SDL_MOUSEBUTTONDOWN ||
        event->button.button != SDL_BUTTON_LEFT) {
        return GK_ACTION_NONE;
    }

    but->is_pressed = gk_point_in_rect(event->button.x, event->button.y, but->place);
    but->is_hovered = but->is_pressed;

    return but->is_pressed ? but->act : GK_ACTION_NONE;
}

// ----------------------------------------------------------------------
static void gk_add_text(GK_Display *disp, const char *text, GK_ID id) {
    assert(disp);
    assert(text);
    if (id < 0 || id >= disp->data->size)  ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data->pool[id]);
    if (obj->kind != GK_GRAPHIC_TEXT || obj->data.text == nullptr) {
        return;
    }
    if (obj->data.text->kind != GK_GRAPHIC_TEXT_KIND_OUTPUT) {
        return;
    }

    GK_GraphicText *txt = obj->data.text;
    const size_t old_len = txt->data.syms ? strlen(txt->data.syms) : 0;
    const size_t add_len = strlen(text);

    char *buffer = old_len ? (char *)calloc(1, add_len) :
                             (char *)realloc(buffer, add_len + txt->data.size);

    if (buffer == NULL) ExitF("NULL allocate", );

    memcpy(buffer + old_len, text, add_len + 1);
    txt->data.syms = buffer;
    txt->data.size = (int)(old_len + add_len);
    return ;
}

// ----------------------------------------------------------------------
static void gk_clear_text(GK_Display *disp, GK_ID id) {
    assert(disp);
    if (id < 0 || id >= disp->data->size) ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data->pool[id]);
    if (obj->kind != GK_GRAPHIC_TEXT || obj->data.text->kind != GK_GRAPHIC_TEXT_KIND_OUTPUT
        || obj->data.text == NULL) {
        return ;
    }

    free(obj->data.text->data.syms);
    obj->data.text->data.syms = NULL;
    obj->data.text->data.size = 0;
    obj->data.text->data.cur_pos = 0;

    return ;
}

// ----------------------------------------------------------------------
static void gk_add_image(GK_Display *disp, const char *file, GK_ID id) {
    assert(disp);
    assert(file);
    if (id < 0 || id >= disp->data->size) ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data->pool[id]);
    if (obj->kind != GK_GRAPHIC_IMAGE || obj->data.img == nullptr) {
        return ;
    }

    if (obj->data.img->tex != NULL) {
        SDL_DestroyTexture(obj->data.img->tex);
        obj->data.img->tex = NULL;
    }

    obj->data.img->tex = gk_load_texture(disp->sys.ren, file);
    return ;
}

// ----------------------------------------------------------------------
static void gk_clear_image(GK_Display *disp, GK_ID id) {
    assert(disp);
    if (id < 0 || id >= disp->data->size) ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data->pool[id]);
    if (obj->kind != GK_GRAPHIC_IMAGE || obj->data.img == nullptr) {
        return ;
    }

    if (obj->data.img->tex != NULL) {
        SDL_DestroyTexture(obj->data.img->tex);
        obj->data.img->tex = nullptr;
    }
    return ;
}

// ----------------------------------------------------------------------
static GK_ID gk_get_text_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:     return GK_GUESS_MAIN_TEXT;
        case GK_MENU_SUCCESS:   return GK_SUCCESS_MAIN_TEXT;
        case GK_MENU_N_SUCCESS: return GK_N_SUCCESS_MAIN_TEXT;
        default:                return GK_INVALID_ID;
    }
    return ;
}

// ----------------------------------------------------------------------
static GK_ID gk_get_image_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:     return GK_GUESS_IMAGE;
        case GK_MENU_SUCCESS:   return GK_SUCCESS_IMAGE;
        case GK_MENU_N_SUCCESS: return GK_N_SUCCESS_IMAGE;
        default:                return GK_INVALID_ID;
    }
    return ;
}

// ====================================================================
// RENDER FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
static void gk_render_text(SDL_Renderer *render, TTF_Font *font, GK_GraphicText *txt) {
    assert(render);
    assert(font);
    assert(txt);

    if (txt->background != NULL) {
        SDL_RenderCopy(render, txt->background, nullptr, &txt->place);
    }

    const char *text = txt->data.syms;
    if (text == NULL || text[0] == '\0') {
        return;
    }

    const Uint32 wrap_width = (txt->place.w > 0) ? (Uint32)txt->place.w : 0;

    SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(font, text,
        GK_FONT_COLOR, wrap_width);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(render, surface);
    SDL_Rect dst = txt->place;

    if (dst.w <= 0) dst.w = surface->w;
    if (dst.h <= 0) dst.h = surface->h;

    SDL_RenderCopy(render, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return ;
}

// ----------------------------------------------------------------------
static void gk_render_button(SDL_Renderer *render, GK_GraphicButton *but) {
    assert(render);
    assert(but);

    SDL_Texture *tex = but->is_pressed ? but->texture.pressed : but->texture.unpressed;
    if (tex != NULL) {
        SDL_RenderCopy(render, tex, nullptr, &but->place);
        return ;
    }

    // Default settings
    SDL_SetRenderDrawColor(render, 80, 80, 80, 255);
    SDL_RenderFillRect(render, &but->place);
    SDL_SetRenderDrawColor(render, 180, 180, 180, 255);
    SDL_RenderDrawRect(render, &but->place);
    return ;
}

// ----------------------------------------------------------------------
static void gk_render_video(SDL_Renderer *render, GK_GraphicVideo *vid) {
    assert(render);
    assert(vid);
    assert(vid->data);

    const uint64_t cur_time = SDL_GetTicks64();
    const uint64_t diff = cur_time - vid->last_time;

    if (diff >= vid->delay) {
        vid->current++;
        if (vid->current >= vid->size) {
            vid->current = 0;
        }
        vid->last_time = cur_time;
    }

    gk_show_image(render, vid->data[vid->current], &(vid->place));
    return ;
}


// ====================================================================
// MAIN GRAPHIC FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
void GK_InitDisplay(GK_Display *disp) {
    assert(disp);

    SDL_Init(SDL_INIT_AUDIO);
    TTF_Init();

    SDL_Window *win = SDL_CreateWindow("GK from Kabuto",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        GK_SCREEN_WIDTH, GK_SCREEN_HEIGHT,0);

    SDL_Renderer *render = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Font *font = TTF_OpenFont(GK_SYSTEM_FONT, 24);

    disp->sys.font = font;
    disp->sys.win = win;
    disp->sys.ren = render;

    gk_load_resources(disp);
    return ;
}

// ----------------------------------------------------------------------
void GK_DestroyDisplay(GK_Display *disp) {
    assert(disp);

    if (disp->sys.font != NULL) {
        TTF_CloseFont(disp->sys.font);
        disp->sys.font = nullptr;
    }

    if (disp->sys.ren != NULL) {
        SDL_DestroyRenderer(disp->sys.ren);
        disp->sys.ren = nullptr;
    }

    if (disp->sys.win != NULL) {
        SDL_DestroyWindow(disp->sys.win);
        disp->sys.win = nullptr;
    }

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

// ----------------------------------------------------------------------
void GK_DisplayTreeBranch(GK_Display *disp, GK_TreeObject *obj) {
    assert(disp);
    assert(obj);

    const GK_ID text_win = gk_get_text_window(disp);
    const GK_ID img_win  = gk_get_image_window(disp);

    if (text_win != GK_INVALID_ID) {
        gk_clear_text(disp, text_win);
    }
    if (img_win != GK_INVALID_ID) {
        gk_clear_image(disp, img_win);
    }

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:
            gk_add_text(disp, "Это ", text_win);
            gk_add_text(disp, (obj->set & GK_TREE_OBJECT_HAVE_TEXT) ? obj->text : "неизвестный объект", text_win);
            gk_add_text(disp, "?", text_win);
            if ((obj->set & GK_TREE_OBJECT_HAVE_IMAGE) && obj->files.img != nullptr) {
                gk_add_image(disp, obj->files.img, img_win);
            }
            break;

        case GK_MENU_SUCCESS:
            gk_add_text(disp, "Я угадал? Это: ", text_win);
            gk_add_text(disp, (obj->set & GK_TREE_OBJECT_HAVE_TEXT) ? obj->text : "неизвестный объект", text_win);
            break;

        case GK_MENU_N_SUCCESS:
            gk_add_text(disp, "Не угадал. Объект: ", text_win);
            gk_add_text(disp, (obj->set & GK_TREE_OBJECT_HAVE_TEXT) ? obj->text : "неизвестный объект", text_win);
            break;

        default:
            break;
    }
    return ;
}

GK_ActionKind GK_PollAction(GK_Display *disp) {
    assert(disp);

    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return GK_ACTION_EXIT;
        }

        // if (disp->data == nullptr) {
        //     continue;
        // }

        GK_Menu *menu = &disp->menus[disp->cur_menu];
        for (int i = 0; i < menu->size; i++) {
            const GK_ID id = menu->data[i];
            if (id < 0 || id >= disp->data->size) {
                continue;
            }

            GK_GraphicObject *obj = &(disp->data->pool[id]);
            if (obj->kind != GK_GRAPHIC_BUTTON || obj->data.but == NULL) {
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

void GK_Update(GK_Main *app, GK_ActionKind action) {
    assert(app);

    switch (action) {
        case GK_ACTION_NONE:
            return ;

        case GK_ACTION_BEGIN_GUESS:
            app->tree.cur = app->tree.null;
            app->disp.cur_menu = GK_MENU_GUESS;
            GK_DisplayTreeBranch(&app->disp, app->tree.cur);
            return ;

        case GK_ACTION_OPEN_ADMIN:
            app->disp.cur_menu = GK_MENU_ADMIN_MENU;
            return ;

        case GK_ACTION_RETURN_TO_MENU:
            app->disp.cur_menu = GK_MENU_START_GUESS;
            return ;

        case GK_ACTION_ANSWER_YES:
            if (app->tree.cur == NULL) {
                return ;
            }
            if (app->tree.cur->kind == GK_TREE_OBJECT_LEAF) {
                app->disp.cur_menu = GK_MENU_SUCCESS;
                GK_DisplayTreeBranch(&app->disp, app->tree.cur);
                return ;
            }
            app->tree.cur = app->tree.cur->branch.yes;
            GK_DisplayTreeBranch(&app->disp, app->tree.cur);
            return ;

        case GK_ACTION_ANSWER_NO:
            if (app->tree.cur == NULL) {
                return ;
            }
            if (app->tree.cur->kind == GK_TREE_OBJECT_LEAF) {
                app->disp.cur_menu = GK_MENU_N_SUCCESS;
                GK_DisplayTreeBranch(&app->disp, app->tree.cur);
                return ;
            }
            app->tree.cur = app->tree.cur->branch.no;
            GK_DisplayTreeBranch(&app->disp, app->tree.cur);
            return ;

        case GK_ACTION_EXIT:
        case GK_ACTION_LOAD_DATA:
        case GK_ACTION_UPLOAD_DATA:
        case GK_ACTION_ADD_OBJECT:
        case GK_ACTION_CONTROL_RECORD:
        default:
            return;
    }
}

void GK_Render(GK_Main *app) {
    assert(app);

    SDL_Renderer *render = app->disp.sys.ren;
    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    SDL_RenderClear(render);

    GK_Menu *menu = &app->disp.menus[app->disp.cur_menu];
    for (int i = 0; i < menu->size; i++) {
        const GK_ID id = menu->data[i];
        if (id < 0 || id >= app->disp.data->size) {
            continue;
        }

        GK_GraphicObject *obj = &(app->disp.data->pool[id]);
        if (!obj->must_show) {
            continue;
        }

        switch (obj->kind) {
            case GK_GRAPHIC_BUTTON:
                gk_render_button(render, obj->data.but);
                break;

            case GK_GRAPHIC_IMAGE:
                if (obj->data.img != nullptr) {
                    gk_show_image(render, obj->data.img->tex, obj->data.img->place);
                }
                break;

            case GK_GRAPHIC_TEXT:
                gk_render_text(render, app->disp.sys.font, obj->data.text);
                break;

            case GK_GRAPHIC_VIDEO:
                gk_render_video(render, obj->data.vid);
                break;

            default:
                break;
        }
    }

    SDL_RenderPresent(render);
}
