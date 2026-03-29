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

    SDL_RenderCopy(render, tex, NULL, place);
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

    if (but->is_switcher == true) {
        if (gk_point_in_rect(event->button.x, event->button.y, but->place)) {
            but->is_pressed = !but->is_pressed;
            return but->act;
        }
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
    if (id < 0 || id >= disp->data.size)  ExitF("Bad ID", );

    printf("ADD TEXT \nINPUT:%s  \nBEFORE:  %s\n", text, disp->data.pool[id].data.text->data.syms);
    GK_GraphicObject *obj = &(disp->data.pool[id]);
    if (obj->kind != GK_GRAPHIC_TEXT) {
        return ;
    }
    if (obj->data.text->kind != GK_GRAPHIC_TEXT_KIND_OUTPUT) {
        return ;
    }

    // printf("ADD TEXT \nINPUT:%s  \nBEFORE:  %s\n", text, disp->data.pool[id].data.text->data.syms);
    GK_GraphicText *txt = obj->data.text;
    size_t old_len = txt->data.syms ? strlen(txt->data.syms) : 0;
    const size_t add_len = strlen(text);

    printf("OLD: %zu\nNEW: %zu\n", old_len, add_len);

    char *buffer = txt->data.syms;
    if (old_len == 0) {
        old_len = 1;
        buffer = (char *)calloc(add_len + 2, sizeof(char));
    } else {
        buffer = (char *)realloc(buffer, add_len + (size_t)txt->data.size);
    }

    if (buffer == NULL) ExitF("NULL allocate", );

    memcpy(buffer + old_len - 1, text, add_len + 1);
    txt->data.syms = buffer;
    txt->data.size = (int)(old_len + add_len);
    return ;
}

// ----------------------------------------------------------------------
static void gk_clear_text(GK_Display *disp, GK_ID id) {
    assert(disp);
    if (id < 0 || id >= disp->data.size) ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data.pool[id]);
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
    if (id < 0 || id >= disp->data.size) ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data.pool[id]);
    if (obj->kind != GK_GRAPHIC_IMAGE || obj->data.img == NULL) {
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
    if (id < 0 || id >= disp->data.size) ExitF("Bad ID", );

    GK_GraphicObject *obj = &(disp->data.pool[id]);
    if (obj->kind != GK_GRAPHIC_IMAGE || obj->data.img == NULL) {
        return ;
    }

    if (obj->data.img->tex != NULL) {
        SDL_DestroyTexture(obj->data.img->tex);
        obj->data.img->tex = NULL;
    }
    return ;
}


// ====================================================================
// THIS FUNCTIONS MUST BE ADJUSTED WHEN CHANGING CONFIGS
// ====================================================================

// ----------------------------------------------------------------------
static GK_ID gk_get_output_text_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:     return GK_GUESS_MAIN_TEXT;
        case GK_MENU_SUCCESS:   return GK_SUCCESS_MAIN_TEXT;
        case GK_MENU_N_SUCCESS: return GK_N_SUCCESS_MAIN_TEXT;

        case GK_MENU_START_GUESS:   return GK_START_GUESS_MAIN_TEXT;
        case GK_MENU_ADMIN_MENU:    return GK_ADMIN_MENU_MAIN_TEXT;
        case GK_MENU_ADD_MENU:      return GK_ADD_MENU_MAIN_TEXT;
        case GK_MENU_EXIT_MENU:
        case GK_MENU_INIT:
        default:                return GK_INVALID_ID;
    }
    return GK_INVALID_ID;
}

static GK_ID gk_get_input_text_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_ADMIN_MENU:    return GK_ADMIN_MENU_PSW_TEXT;
        case GK_MENU_ADD_MENU:      return GK_ADD_MENU_INP_TEXT;

        case GK_MENU_GUESS:
        case GK_MENU_SUCCESS:
        case GK_MENU_N_SUCCESS:
        case GK_MENU_START_GUESS:
        case GK_MENU_EXIT_MENU:
        case GK_MENU_INIT:
        default:                return GK_INVALID_ID;
    }
    return GK_INVALID_ID;
}

// ----------------------------------------------------------------------
static GK_ID gk_get_image_window(const GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:     return GK_GUESS_IMAGE;
        case GK_MENU_SUCCESS:   return GK_SUCCESS_IMAGE;
        case GK_MENU_N_SUCCESS: return GK_N_SUCCESS_IMAGE;

        case GK_MENU_START_GUESS:
        case GK_MENU_ADMIN_MENU:
        case GK_MENU_ADD_MENU:
        case GK_MENU_EXIT_MENU:
        case GK_MENU_INIT:
        default:                return GK_INVALID_ID;
    }
    return GK_INVALID_ID;
}


// ====================================================================
// INPUT FUNCTIONS
// ====================================================================

// ----------------------------------------------------------------------
static void gk_input_clear(GK_TextInput *inp, bool is_hidden) {
    assert(inp);

    memset(inp->buffer, 0, sizeof(inp->buffer));
    inp->buffer[0] = '\0';

    inp->is_hidden = is_hidden;
    inp->is_active = false;
    inp->cursor = 0;
    inp->size = 0;

    return ;
}

static inline void gk_input_set_max_size(GK_TextInput *inp, int size) {
    assert(inp);

    if (size > (int) sizeof(inp->buffer))  size = sizeof(inp->buffer);
    inp->max_size = size;

    return ;
}

// ----------------------------------------------------------------------
static void gk_input_append_text(GK_TextInput *inp, const char *text) {
    assert(inp);
    assert(text);

    if (!inp->is_active) return;

    size_t cap = sizeof(inp->buffer);
    size_t i = 0;

    while (text[i] != '\0' && (size_t)inp->size < cap - 1) {
        inp->buffer[inp->size++] = text[i++];
    }

    inp->buffer[inp->size] = '\0';
    inp->cursor = inp->size;
    return ;
}

// ----------------------------------------------------------------------
static void gk_input_backspace(GK_TextInput *inp) {
    assert(inp);

    if (!inp->is_active) return ;
    if (inp->size <= 0) return ;

    do {
        inp->size--;
    } while (inp->size > 0 &&
             (((unsigned char)inp->buffer[inp->size] & 0xC0u) == 0x80u));

    inp->buffer[inp->size] = '\0';
    inp->cursor = inp->size;

    return ;
}

// ----------------------------------------------------------------------
static void gk_input_begin(GK_TextInput *inp, bool is_hidden) {
    assert(inp);

    gk_input_clear(inp, is_hidden);
    inp->is_active = true;
    SDL_StartTextInput();

    return ;
}

// ----------------------------------------------------------------------
static void gk_input_end(GK_TextInput *inp) {
    assert(inp);

    SDL_StopTextInput();
    inp->is_active = false;

    return ;
}

// ----------------------------------------------------------------------
static inline void gk_input_hide(GK_TextInput *inp) {
    assert(inp);

    inp->is_hidden = true;
}

// ----------------------------------------------------------------------
static inline void gk_input_show(GK_TextInput *inp) {
    assert(inp);

    inp->is_hidden = false;
}

// ----------------------------------------------------------------------
static int gk_utf8_count_codepoints(const char *text) {
    assert(text);

    int count = 0;
    int i = 0;

    while (text[i] != '\0') {
        unsigned char c = (unsigned char)text[i];

        if ((c & 0xC0u) != 0x80u) {
            count++;
        }
        i++;
    }

    return count;
}

// ----------------------------------------------------------------------
static SDL_Surface *gk_input_text_create_surface(TTF_Font *font, GK_TextInput *inp) {
    assert(font);
    assert(inp);

    if (inp->is_active == false)  return NULL;

    SDL_Surface *text_sur = NULL;

    char buffer[sizeof(inp->buffer)] = " ";
    if (inp->is_hidden == true) {
        for (int i = 0; i < gk_utf8_count_codepoints(inp->buffer); i++) {
            buffer[i] = '*';
        }

        text_sur = TTF_RenderUTF8_Blended(font,
            buffer, GK_FONT_COLOR);
    } else {
        if (inp->buffer[0] == '\0') {
            text_sur = TTF_RenderUTF8_Blended(font,
            buffer, GK_FONT_COLOR);
        } else {
            text_sur = TTF_RenderUTF8_Blended(font,
                inp->buffer, GK_FONT_COLOR);
        }
    }

    if (text_sur == NULL)   ExitF("NULL Surface", NULL);

    const int underline_gap = 4;
    const int underline_h = 2;

    int final_w = text_sur->w;
    int final_h = text_sur->h + underline_gap + underline_h;

    if (final_w <= 0) final_w = 1;
    if (final_h <= 0) final_h = 1;

    SDL_Surface *final_sur = SDL_CreateRGBSurfaceWithFormat(
        0, final_w, final_h, 32, SDL_PIXELFORMAT_RGBA8888);

    if (final_sur == NULL) {
        SDL_FreeSurface(text_sur);
        ExitF("NULL Surface", NULL);
    }

    SDL_FillRect(final_sur, NULL,
        SDL_MapRGBA(final_sur->format, 0, 0, 0, 0));

    SDL_Rect text_dst = {0, 0, text_sur->w, text_sur->h};
    SDL_BlitSurface(text_sur, NULL, final_sur, &text_dst);

    SDL_Rect underline = {};
    underline.x = 0;
    underline.y = text_sur->h + underline_gap;
    underline.w = text_sur->w;
    underline.h = underline_h;

    Uint32 pixel = SDL_MapRGBA(final_sur->format,
        GK_FONT_COLOR.r,
        GK_FONT_COLOR.g,
        GK_FONT_COLOR.b,
        GK_FONT_COLOR.a);

    SDL_FillRect(final_sur, &underline, pixel);

    SDL_FreeSurface(text_sur);
    return final_sur;
}



// ====================================================================
// RENDER FUNCTIONS
// ====================================================================

static SDL_Surface *gk_render_multiline_text(TTF_Font *font, const char *text) {
    assert(font);
    assert(text);

    int line_count = 1;
    for (const char *p = text; *p; p++) {
        if (*p == '$') line_count++;
    }

    int max_width = 0;
    int line_height = TTF_FontHeight(font);
    char *text_copy = strdup(text);
    char *saveptr = NULL;
    char *line = strtok_r(text_copy, "$", &saveptr);

    while (line != NULL) {
        int w = 0, h = 0;
        TTF_SizeUTF8(font, line, &w, &h);
        if (w > max_width) max_width = w;
        line = strtok_r(NULL, "$", &saveptr);
    }
    free(text_copy);

    int total_height = line_height * line_count;
    SDL_Surface *final_sur = SDL_CreateRGBSurfaceWithFormat(0,
        max_width, total_height, 32, SDL_PIXELFORMAT_RGBA8888);
    if (final_sur == NULL) ExitF("NULL Surface", NULL);

    SDL_FillRect(final_sur, NULL, SDL_MapRGBA(final_sur->format, 0, 0, 0, 0));

    int y_offset = 0;
    text_copy = strdup(text);
    line = strtok_r(text_copy, "$", &saveptr);

    while (line != NULL) {
        SDL_Surface *line_sur = TTF_RenderUTF8_Blended(font, line, GK_FONT_COLOR);
        if (line_sur) {
            SDL_Rect dest = {0, y_offset, line_sur->w, line_sur->h};
            SDL_BlitSurface(line_sur, NULL, final_sur, &dest);
            SDL_FreeSurface(line_sur);
            y_offset += line_height;
        }
        line = strtok_r(NULL, "$", &saveptr);
    }
    free(text_copy);

    return final_sur;
}


// ----------------------------------------------------------------------
static void gk_render_text(SDL_Renderer *render, TTF_Font *font, GK_GraphicText *txt) {
    assert(render);
    assert(font);
    assert(txt);

    if (txt->background != NULL) {
        SDL_RenderCopy(render, txt->background, NULL, &txt->place);
    }

    const char *text = txt->data.syms;
    if (text == NULL || text[0] == '\0') {
        return ;
    }

    // const Uint32 wrap_width = (txt->place.w > 0) ? (Uint32)txt->place.w : 0;
    // SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(font, text,
    //     GK_FONT_COLOR, wrap_width);

    SDL_Surface *surface = gk_render_multiline_text(font, txt->data.syms);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(render, surface);
    SDL_Rect dst = {};
    dst.w = surface->w;
    dst.h = surface->h;

    dst.x = txt->place.x + (txt->place.w - surface->w) / 2;
    dst.y = txt->place.y + (txt->place.h - surface->h) / 2;

/* if (dst.w <= 0)  dst.w = surface->w; */
/* if (dst.h <= 0)  dst.h = surface->h; */

    SDL_RenderCopy(render, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return ;
}

// ----------------------------------------------------------------------
static void gk_render_text(SDL_Renderer *render, TTF_Font *font, GK_GraphicText *txt, GK_TextInput *inp) {
    assert(render);
    assert(font);
    assert(txt);
    assert(inp);

    if (inp->is_active == false || txt->kind != GK_GRAPHIC_TEXT_KIND_INPUT) {
        return ;
    }

    if (txt->background != NULL) {
        SDL_RenderCopy(render, txt->background, NULL, &txt->place);
    }

    SDL_Surface *surface = gk_input_text_create_surface(font, inp);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(render, surface);
    SDL_Rect dst = {};
    dst.w = surface->w;
    dst.h = surface->h;

    dst.x = txt->place.x + (txt->place.w - surface->w) / 2;
    dst.y = txt->place.y + (txt->place.h - surface->h) / 2;

    SDL_RenderCopy(render, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return ;
}

// ----------------------------------------------------------------------
static void gk_render_button(SDL_Renderer *render, TTF_Font *font, GK_GraphicButton *but) {
    assert(render);
    assert(but);

    SDL_Texture *tex = (but->is_pressed | but->is_hovered) ? but->texture.pressed : but->texture.unpressed;
    if (tex != NULL) {
        SDL_RenderCopy(render, tex, NULL, &but->place);
    }
    if (but->text != NULL) {
        SDL_Surface *surface = gk_render_multiline_text(font, but->text);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(render, surface);
        SDL_Rect dst = {};
        dst.w = surface->w;
        dst.h = surface->h;

        dst.x = but->place.x + (but->place.w - surface->w) / 2;
        dst.y = but->place.y + (but->place.h - surface->h) / 2;

        SDL_RenderCopy(render, texture, NULL, &dst);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
    return ;

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

    TTF_Font *font = TTF_OpenFont(GK_SYSTEM_FONT, 30);

    disp->sys.font = font;
    disp->sys.win = win;
    disp->sys.ren = render;

    gk_input_clear(&(disp->text_inp), false);
    // gk_load_resources(disp);
    return ;
}

// ----------------------------------------------------------------------
void GK_DestroyDisplay(GK_Display *disp) {
    assert(disp);

    if (disp->sys.font != NULL) {
        TTF_CloseFont(disp->sys.font);
        disp->sys.font = NULL;
    }

    if (disp->sys.ren != NULL) {
        SDL_DestroyRenderer(disp->sys.ren);
        disp->sys.ren = NULL;
    }

    if (disp->sys.win != NULL) {
        SDL_DestroyWindow(disp->sys.win);
        disp->sys.win = NULL;
    }

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

// ----------------------------------------------------------------------
void GK_DisplayTreeBranch(GK_Display *disp, GK_TreeObject *obj) {
    assert(disp);
    assert(obj);

    const GK_ID text_win = gk_get_output_text_window(disp);
    const GK_ID img_win  = gk_get_image_window(disp);

    if (text_win != GK_INVALID_ID) {
        gk_clear_text(disp, text_win);
    }
    if (img_win != GK_INVALID_ID) {
        gk_clear_image(disp, img_win);
    }

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:
            gk_add_text(disp, (obj->set & GK_TREE_OBJECT_HAVE_TEXT) ? obj->text : "неизвестный объект", text_win);
            if ((obj->set & GK_TREE_OBJECT_HAVE_IMAGE) && obj->files.img != NULL) {
                gk_add_image(disp, obj->files.img, img_win);
            }
            break;

        case GK_MENU_SUCCESS:
            gk_add_text(disp, "Я думаю, это:  ", text_win);
            gk_add_text(disp, (obj->set & GK_TREE_OBJECT_HAVE_TEXT) ? obj->text : "неизвестный объект", text_win);
            break;

        case GK_MENU_N_SUCCESS:
            gk_add_text(disp, "Не угадал. Объект: ", text_win);
            gk_add_text(disp, (obj->set & GK_TREE_OBJECT_HAVE_TEXT) ? obj->text : "неизвестный объект", text_win);
            break;

        case GK_MENU_START_GUESS:
        case GK_MENU_ADMIN_MENU:
        case GK_MENU_EXIT_MENU:
        case GK_MENU_ADD_MENU:
        case GK_MENU_INIT:
        default:
            break;
    }
    return ;
}

// ----------------------------------------------------------------------
GK_ActionKind GK_PollAction(GK_Display *disp) {
    assert(disp);

    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return GK_ACTION_EXIT;
        }

        if (disp->text_inp.is_active) {
            if (event.type == SDL_TEXTINPUT) {
                gk_input_append_text(&(disp->text_inp), event.text.text);
                return GK_ACTION_INPUT_CHANGED;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_BACKSPACE:
                        gk_input_backspace(&(disp->text_inp));
                        return GK_ACTION_INPUT_CHANGED;

                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        return GK_ACTION_INPUT_SUBMIT;

                    case SDLK_ESCAPE:
                        return GK_ACTION_INPUT_CANCEL;

                    default:
                        break;
                }
            }
            continue;
        }

        GK_Menu *menu = &(disp->menus[disp->cur_menu]);
        for (int i = 0; i < menu->size; i++) {
            const GK_ID id = menu->data[i];
            if (id < 0 || id >= disp->data.size) {
                continue;
            }

            GK_GraphicObject *obj = &(disp->data.pool[id]);
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

// ----------------------------------------------------------------------
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
            gk_input_begin(&(app->disp.text_inp), true);
            return ;

        case GK_ACTION_RETURN_TO_MENU: {
            app->disp.cur_menu = GK_MENU_START_GUESS;
            GK_ID text_id = gk_get_output_text_window(&(app->disp));
            gk_clear_text(&(app->disp), text_id);
            gk_add_text(&(app->disp), "Приветствуем тебя в нашей программе Акинатор", text_id);
            return ;
        }

        case GK_ACTION_ANSWER_YES: {
            if (app->tree.cur == NULL) {
                return ;
            }

            if (app->tree.cur->branch.yes->kind == GK_TREE_OBJECT_LEAF) {
                app->disp.cur_menu = GK_MENU_SUCCESS;
            }

            app->tree.cur = app->tree.cur->branch.yes;
            GK_DisplayTreeBranch(&app->disp, app->tree.cur);
            return ;
        }

        case GK_ACTION_ANSWER_NO: {
            if (app->tree.cur == NULL) {
                return ;
            }

            if (app->tree.cur->branch.no->kind == GK_TREE_OBJECT_LEAF) {
                app->disp.cur_menu = GK_MENU_SUCCESS;
            }

            app->tree.cur = app->tree.cur->branch.no;
            GK_DisplayTreeBranch(&app->disp, app->tree.cur);
            return ;
        }


        case GK_ACTION_INPUT_CANCEL:
        case GK_ACTION_INPUT_SUBMIT: {
            if (app->disp.cur_menu == GK_MENU_ADMIN_MENU) {

                if (strncmp(app->disp.text_inp.buffer, "fopf huiny",
                    sizeof("fopf huiny")) == 0) {
                    GK_ID text_in_id = gk_get_input_text_window(&(app->disp));
                    app->disp.data.pool[text_in_id].must_show = false;
                } else {
                    app->disp.cur_menu = GK_MENU_START_GUESS;
                }

            } else if (app->disp.cur_menu == GK_MENU_ADD_MENU) {
                // Add Object
            }

            gk_input_end(&(app->disp.text_inp));
            // gk_input_stop(&(app->disp.text_inp));
            return ;
        }

        case GK_ACTION_INPUT_CHANGED:
        case GK_ACTION_EXIT:
        case GK_ACTION_LOAD_DATA:
        case GK_ACTION_UPLOAD_DATA:
        case GK_ACTION_ADD_OBJECT:
        case GK_ACTION_CONTROL_RECORD:
        default:
            return ;
    }
    return ;
}

// ----------------------------------------------------------------------
void GK_Render(GK_Main *app) {
    assert(app);

    SDL_Renderer *render = app->disp.sys.ren;
    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    SDL_RenderClear(render);

    GK_Menu *menu = &app->disp.menus[app->disp.cur_menu];
    for (int i = 0; i < menu->size; i++) {
        const GK_ID id = menu->data[i];
        if (id < 0 || id >= app->disp.data.size) {
            continue;
        }

        GK_GraphicObject *obj = &(app->disp.data.pool[id]);
        if (!obj->must_show) {
            continue;
        }

        switch (obj->kind) {
            case GK_GRAPHIC_BUTTON:
                gk_render_button(render, app->disp.sys.font, obj->data.but);
                break;

            case GK_GRAPHIC_IMAGE:
                if (obj->data.img != NULL) {
                    gk_show_image(render, obj->data.img->tex, &(obj->data.img->place));
                }
                break;

            case GK_GRAPHIC_TEXT:
                if (obj->data.text->kind == GK_GRAPHIC_TEXT_KIND_OUTPUT) {
                    gk_render_text(render, app->disp.sys.font, obj->data.text);
                } else {
                    gk_render_text(render, app->disp.sys.font, obj->data.text, &(app->disp.text_inp));
                }

                break;

            case GK_GRAPHIC_VIDEO:
                gk_render_video(render, obj->data.vid);
                break;

            default:
                break;
        }
    }

    SDL_RenderPresent(render);
    return ;
}
