#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "GK_AllFunc.h"
#include "GK_ParserType.h"
#include "GK_DSLLib.h"

// ====================================================================
// USED CONSTANTS
// ====================================================================

extern const char *GK_CONFIG_OBJECT_FILE;
extern const char *GK_CONFIG_MENU_FILE;


// ====================================================================
// DECLARATION SUPPORT FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
// Texture works
static inline SDL_Texture *gk_load_texture(SDL_Renderer *render, const char *name);
static inline SDL_Texture *gk_create_color_texture(SDL_Renderer *render, const SDL_Rect *place,
    const SDL_Color *color);

// ------------------------------------------------------------------
// Support macros
static void gk_skip_void(GK_ParserObject *par);
static void gk_skip_void(GK_ParserMenu *par);

static inline char get_c(GK_ParserObject *par);
static inline char get_c(GK_ParserMenu *par);

static inline void next_c(GK_ParserObject *par);
static inline void next_c(GK_ParserMenu *par);


// ------------------------------------------------------------------
// Parser object config
static void gk_parse_object_text(GK_ParserObject *par);
static void gk_parse_object_text_arg(GK_ParserObject *par, GK_GraphicText *txt);

static void gk_parse_object_video(GK_ParserObject *par);
static void gk_parse_object_video_arg(GK_ParserObject *par, GK_GraphicVideo *txt);

static void gk_parse_object_button(GK_ParserObject *par);
static void gk_parse_object_button_arg(GK_ParserObject *par, GK_GraphicButton *txt);

static void gk_parse_object_image(GK_ParserObject *par);
static void gk_parse_object_image_arg(GK_ParserObject *par, GK_GraphicImage *txt);


// ------------------------------------------------------------------
// Parse menu config
static void gk_push_menu(GK_Menu *menu, int value);


// ====================================================================
// MAIN PARSE FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
void GK_ParseObjectInit(GK_ParserObject *par, SDL_Renderer *render, const char *name_file) {
    assert(par);

    int size = gk_get_file_size(name_file);
    char *buffer = gk_create_file_buffer(name_file, size);

    par->buffer = buffer;
    par->size = size;
    par->cur_p = 0;

    GK_GraphicObject *pool = (GK_GraphicObject *)calloc(GK_PARSER_MAX_ITEMS,
        sizeof(GK_GraphicObject));

    if (pool == NULL) {
        free(buffer);
        ExitF("NULL Calloc", );
    }
    par->pool = pool;
    par->cur_o = 0;

    par->render = render;

    return ;
}

// ------------------------------------------------------------------
int GK_ParseObjectLoop(GK_ParserObject *par) {
    assert(par);

    char buffer_kind[GK_PARSER_MAX_KIND] = "";
    int len = 0;

    while (par->cur_p < par->size) {
        if (get_c(par) == '#') {
            while (get_c(par) != '\n') {
                par->cur_p++;
            }
            par->cur_p++;

            gk_skip_void(par);
            continue;
        }

        sscanf(par->buffer + par->cur_p, "%s %n", buffer_kind, &len);
        par->cur_p += len;

        uint64_t hash = gk_get_hash(buffer_kind);
        switch (hash) {
            case gk_get_hash("text"):
                gk_parse_object_text(par);
                break;

            case gk_get_hash("button"):
                gk_parse_object_button(par);
                break;

            case gk_get_hash("video"):
                gk_parse_object_video(par);
                break;

            case gk_get_hash("image"):
                gk_parse_object_image(par);
                break;

            default:
                ExitF("Unknown Kind", -1);
        }
        gk_skip_void(par);
    }
    return par->cur_o;
}

// ------------------------------------------------------------------
void GK_ParseObjectDestroy(GK_ParserObject *par) {
    assert(par);

    free(par->buffer);
    return ;
}

// ------------------------------------------------------------------
void GK_ParseMenuInit(GK_ParserMenu *par, GK_Menu *menus, const char *name_file) {
    assert(par);
    assert(menus);
    assert(name_file);

    int size = gk_get_file_size(name_file);
    char *buffer = gk_create_file_buffer(name_file, size);

    par->buffer = buffer;
    par->cur_p = 0;
    par->size = size;

    par->menu_pool = menus;
    par->cur_menu = 0;

    return ;
}

// ------------------------------------------------------------------
void GK_ParseMenuLoop(GK_ParserMenu *par) {
    assert(par);

    int len = 0;
    int value = 0;

    gk_skip_void(par);
    while ((get_c(par) == '|' && par->cur_p < par->size) || get_c(par) == '#') {
        if (get_c(par) == '#') {
            while (get_c(par) != '\n') {
                par->cur_p++;
            }
            par->cur_p++;
            gk_skip_void(par);
            continue;
        }

        next_c(par);

        while (get_c(par) != '|') {
            sscanf(par->buffer + par->cur_p, "%d %n", &value, &len);
            par->cur_p += len;

            gk_push_menu(&(par->menu_pool[par->cur_menu]), value);
            gk_skip_void(par);
        }
        next_c(par);
        gk_skip_void(par);

        par->cur_menu++;
    }
    return ;
}

// ------------------------------------------------------------------
void GK_ParseMenuDestroy(GK_ParserMenu *par) {
    assert(par);

    free(par->buffer);
    return ;
}

// ------------------------------------------------------------------
void GK_Parse(GK_Display *disp) {
    assert(disp);

    GK_ParserObject par_obj = {};
    GK_ParseObjectInit(&par_obj, disp->sys.ren, GK_CONFIG_OBJECT_FILE);
    int size_pool = GK_ParseObjectLoop(&par_obj);

    disp->data.size = size_pool;
    disp->data.pool = par_obj.pool;

    GK_ParseObjectDestroy(&par_obj);

    GK_ParserMenu par_menu = {};
    GK_ParseMenuInit(&par_menu, disp->menus, GK_CONFIG_MENU_FILE);
    GK_ParseMenuLoop(&par_menu);
    GK_ParseMenuDestroy(&par_menu);

    return ;
}


// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
static void gk_skip_void(GK_ParserObject *par) {
    assert(par);

    int add_ind = 0;
    while (true) {
        char sym = par->buffer[par->cur_p + add_ind];
        if (sym == ' ' || sym == '\n') {
            add_ind++;
            continue;
        }
        break;
    }
    par->cur_p += add_ind;
    return ;
}
static void gk_skip_void(GK_ParserMenu *par) {
    assert(par);

    int add_ind = 0;
    while (true) {
        char sym = par->buffer[par->cur_p + add_ind];
        if (sym == ' ' || sym == '\n' ) {
            add_ind++;
            continue;
        }
        break;
    }
    par->cur_p += add_ind;
    return ;
}

// ----------------------------------------------------------------------
static inline SDL_Texture *gk_load_texture(SDL_Renderer *render, const char *name) {
    return IMG_LoadTexture(render, name);
}

// ------------------------------------------------------------------
static inline SDL_Texture *gk_create_color_texture(SDL_Renderer *render, const SDL_Rect *place,
            const SDL_Color *color) {
    assert(render);
    assert(place);
    assert(color);

    SDL_Texture *tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, place->w, place->h);

    if (tex == NULL) ExitF("NULL Texture", NULL);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(render, tex);
    SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(render, color->r, color->g, color->b, color->a);
    SDL_RenderFillRect(render, NULL);
    SDL_SetRenderTarget(render, NULL);

    return tex;
}

// ------------------------------------------------------------------
static inline char get_c(GK_ParserObject *par) {
    return par->buffer[par->cur_p];
}
static inline char get_c(GK_ParserMenu *par) {
    return par->buffer[par->cur_p];
}

// ------------------------------------------------------------------
static inline void next_c(GK_ParserObject *par) {
    par->cur_p++;
}
static inline void next_c(GK_ParserMenu *par) {
    par->cur_p++;
}

// ------------------------------------------------------------------
static void gk_push_menu(GK_Menu *menu, int value) {
    assert(menu);

    if (menu->data == NULL) {
        menu->data = (GK_ID *)calloc(10, sizeof(GK_ID));
        if (menu->data == NULL)     ExitF("NULL Calloc", );

        menu->size = 0;
        menu->capacity = 10;
    }
    if (menu->size == menu->capacity) {
        menu->data = (GK_ID *)realloc(menu->data, (size_t)menu->size * 2 * sizeof(GK_ID));
        if (menu->data == NULL)     ExitF("NULL Calloc", );

        menu->capacity *= 2;
    }
    menu->data[menu->size++] = value;

    return ;
}


// ====================================================================
// SUPPORT PARSE OBJECT FUNCTIONS
// ====================================================================

// ====================================================================
// TEXT
static void gk_parse_object_text(GK_ParserObject *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicText *text = (GK_GraphicText *)calloc(1, sizeof(GK_GraphicText));
    if (text == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_object_text_arg(par, text);
        gk_skip_void(par);
    }
    if (get_c(par) == '\0') ExitF("Unexpected EOF", );
    next_c(par);

    GK_GraphicObject *obj = &(par->pool[par->cur_o]);

    obj->id = par->cur_o++;
    obj->kind = GK_GRAPHIC_TEXT;
    obj->must_show = true;
    obj->data.text = text;

    return ;
}

static void gk_parse_object_text_arg(GK_ParserObject *par, GK_GraphicText *txt) {
    assert(par);
    assert(txt);

    char buffer[GK_PARSER_MAX_KIND] = "";
    int len = 0;

    sscanf(par->buffer + par->cur_p, "%s %n", buffer, &len);
    par->cur_p += len;

    uint64_t hash = gk_get_hash(buffer);
    switch (hash) {

        // ---------------------------------------------------------------
        case gk_get_hash("rect"): {
            SDL_Rect place = {};

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &(place.x), &(place.y), &(place.w), &(place.h), &len);
            par->cur_p += len;

            txt->place = place;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("kind"): {
            sscanf(par->buffer + par->cur_p, "%s %n", buffer, &len);
            par->cur_p += len;

            hash = gk_get_hash(buffer);
            if (hash == gk_get_hash("input")) {
                txt->kind = GK_GRAPHIC_TEXT_KIND_INPUT;
            } else {
                txt->kind = GK_GRAPHIC_TEXT_KIND_OUTPUT;
            }
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("color"): {
            SDL_Color color = {};
            int r = 0, g = 0, b = 0, a = 0;

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &r, &g, &b, &a, &len);
            par->cur_p += len;

            color.r = (Uint8)r;
            color.g = (Uint8)g;
            color.b = (Uint8)b;
            color.a = (Uint8)a;

            txt->background = gk_create_color_texture(par->render,
                                &(txt->place), &color);
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("tex"): {
            gk_skip_void(par);

            char path_buffer[GK_PARSER_MAX_PATH] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", path_buffer, &len);
            par->cur_p += len;

            txt->background = gk_load_texture(par->render, path_buffer);
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("data"): {
            gk_skip_void(par);

            char text_buffer[GK_PARSER_MAX_TEXT] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", text_buffer, &len);
            par->cur_p += len;

            txt->data.syms = strdup(text_buffer);
            txt->data.size = (int)strlen(text_buffer);
            break;
        }

        default:
            ExitF("Unknown word in config", );
    }
    return ;
}

// ====================================================================
// VIDEO
static void gk_parse_object_video(GK_ParserObject *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicVideo *vid = (GK_GraphicVideo *)calloc(1, sizeof(GK_GraphicVideo));
    if (vid == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_object_video_arg(par, vid);
        gk_skip_void(par);
    }
    if (get_c(par) == '\0') ExitF("Unexpected EOF", );
    next_c(par);

    GK_GraphicObject *obj = &(par->pool[par->cur_o]);

    obj->id = par->cur_o++;
    obj->kind = GK_GRAPHIC_VIDEO;
    obj->must_show = true;
    obj->data.vid = vid;

    return ;
}

static void gk_parse_object_video_arg(GK_ParserObject *par, GK_GraphicVideo *vid) {
    assert(par);
    assert(vid);

    char buffer[GK_PARSER_MAX_KIND] = "";
    int len = 0;

    sscanf(par->buffer + par->cur_p, "%s %n", buffer, &len);
    par->cur_p += len;

    uint64_t hash = gk_get_hash(buffer);
    switch (hash) {
        // ---------------------------------------------------------------
        case gk_get_hash("rect"): {
            SDL_Rect place = {};

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &(place.x), &(place.y), &(place.w), &(place.h), &len);
            par->cur_p += len;

            vid->place = place;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("data"): {
            int size = 0;
            sscanf(par->buffer + par->cur_p, "%d %n", &size, &len);
            par->cur_p += len;

            char path_dir[GK_PARSER_MAX_PATH] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", path_dir, &len);
            par->cur_p += len;

            vid->size = size;
            vid->data = (SDL_Texture **)calloc((size_t)size, sizeof(SDL_Texture *));
            if (vid->data == NULL) {
                ExitF("NULL Calloc", );
            }

            len = (int)strlen(path_dir);

            for (int i = 1; i < size + 1; i++) {
                sprintf(path_dir + len, "/%d.png", i);
                SDL_Texture *tex = gk_load_texture(par->render, path_dir);
                vid->data[i - 1] = tex;
            }

            vid->current = 0;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("delay"): {
            uint64_t delay = 0;
            sscanf(par->buffer + par->cur_p, "%lu %n", &delay, &len);
            par->cur_p += len;

            vid->delay = delay;
            vid->last_time = 0;
            break;
        }

        default:
            ExitF("Unknown word in config", );
    }
    return ;
}

// ====================================================================
// BUTTON
static void gk_parse_object_button(GK_ParserObject *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicButton *but = (GK_GraphicButton *)calloc(1, sizeof(GK_GraphicButton));
    if (but == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_object_button_arg(par, but);
        gk_skip_void(par);
    }
    if (get_c(par) == '\0') ExitF("Unexpected EOF", );
    next_c(par);

    GK_GraphicObject *obj = &(par->pool[par->cur_o]);

    obj->id = par->cur_o++;
    obj->kind = GK_GRAPHIC_BUTTON;
    obj->must_show = true;
    obj->data.but = but;

    return ;
}

static void gk_parse_object_button_arg(GK_ParserObject *par, GK_GraphicButton *but) {
    assert(par);
    assert(but);

    char buffer[GK_PARSER_MAX_KIND] = "";
    int len = 0;

    sscanf(par->buffer + par->cur_p, "%s %n", buffer, &len);
    par->cur_p += len;

    uint64_t hash = gk_get_hash(buffer);
    switch (hash) {
        // ---------------------------------------------------------------
        case gk_get_hash("rect"): {
            SDL_Rect place = {};

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &(place.x), &(place.y), &(place.w), &(place.h), &len);
            par->cur_p += len;

            but->place = place;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("color_p"): {
            SDL_Color color = {};
            int r = 0, g = 0, b = 0, a = 0;

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &r, &g, &b, &a, &len);
            par->cur_p += len;

            color.r = (Uint8)r;
            color.g = (Uint8)g;
            color.b = (Uint8)b;
            color.a = (Uint8)a;

            but->texture.pressed = gk_create_color_texture(par->render,
                                &(but->place), &color);
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("color_u"): {
            SDL_Color color = {};
            int r = 0, g = 0, b = 0, a = 0;

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &r, &g, &b, &a, &len);
            par->cur_p += len;

            color.r = (Uint8)r;
            color.g = (Uint8)g;
            color.b = (Uint8)b;
            color.a = (Uint8)a;

            but->texture.unpressed = gk_create_color_texture(par->render,
                                &(but->place), &color);
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("tex_p"): {
            gk_skip_void(par);

            char path_buffer[GK_PARSER_MAX_PATH] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", path_buffer, &len);
            par->cur_p += len;

            but->texture.pressed = gk_load_texture(par->render, path_buffer);
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("tex_u"): {
            gk_skip_void(par);

            char path_buffer[GK_PARSER_MAX_PATH] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", path_buffer, &len);
            par->cur_p += len;

            but->texture.unpressed = gk_load_texture(par->render, path_buffer);
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("action"): {
            int action = 0;
            sscanf(par->buffer + par->cur_p, "%d %n", &action, &len);
            par->cur_p += len;

            but->act = (GK_ActionKind)action;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("switcher"): {
            but->is_switcher = true;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("data"): {
            gk_skip_void(par);

            char text_buffer[GK_PARSER_MAX_TEXT] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", text_buffer, &len);
            par->cur_p += len;

            but->text = strdup(text_buffer);
            // but.->data.syms = strdup(text_buffer);
            // but.->data.size = (int)strlen(text_buffer);
            break;
        }
        // ---------------------------------------------------------------
        default:
            ExitF("Unknown word in config", );
    }
    return ;
}

// ====================================================================
// IMAGE
static void gk_parse_object_image(GK_ParserObject *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicImage *img = (GK_GraphicImage *)calloc(1, sizeof(GK_GraphicImage));
    if (img == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_object_image_arg(par, img);
        gk_skip_void(par);
    }
    if (get_c(par) == '\0') ExitF("Unexpected EOF", );
    next_c(par);

    GK_GraphicObject *obj = &(par->pool[par->cur_o]);

    obj->id = par->cur_o++;
    obj->kind = GK_GRAPHIC_IMAGE;
    obj->must_show = true;
    obj->data.img = img;

    return ;
}

static void gk_parse_object_image_arg(GK_ParserObject *par, GK_GraphicImage *img) {
    assert(par);
    assert(img);

    char buffer[GK_PARSER_MAX_KIND] = "";
    int len = 0;

    sscanf(par->buffer + par->cur_p, "%s %n", buffer, &len);
    par->cur_p += len;

    uint64_t hash = gk_get_hash(buffer);
    switch (hash) {
        // ---------------------------------------------------------------
        case gk_get_hash("rect"): {
            SDL_Rect place = {};

            sscanf(par->buffer + par->cur_p, "%d %d %d %d %n",
                &(place.x), &(place.y), &(place.w), &(place.h), &len);
            par->cur_p += len;

            img->place = place;
            break;
        }

        // ---------------------------------------------------------------
        case gk_get_hash("data"): {
            char path_buffer[GK_PARSER_MAX_PATH] = "";
            sscanf(par->buffer + par->cur_p, "\"%[^\"]\" %n", path_buffer, &len);
            par->cur_p += len;

            img->tex = gk_load_texture(par->render, path_buffer);
            break;
        }

        default:
            ExitF("Unknown word in config", );
    }
    return ;
}
