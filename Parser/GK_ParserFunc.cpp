#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

#include "GK_AllFunc.h"
#include "GK_ParserType.h"
#include "GK_DSLLib.h"

// ====================================================================
// DECLARATION SUPPORT FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
// File support
static int gk_get_file_size(const char *name);
static char *gk_create_file_buffer(const char *name, int size);

// ------------------------------------------------------------------
// String processing
static constexpr uint64_t gk_get_hash(const char *buffer) {
    int idx = 0;
    uint64_t hash = 5137;

    while (buffer[idx] != '\0') {
        hash = hash * 33 + (uint64_t)(buffer[idx++]);
    }
    return hash;
}

// ------------------------------------------------------------------
// Texture works
static inline SDL_Texture *gk_load_texture(SDL_Renderer *render, const char *name);
static inline SDL_Texture *gk_create_color_texture(SDL_Renderer *render, const SDL_Rect *place,
    const SDL_Color *color);

// ------------------------------------------------------------------
// Support macros
static void gk_skip_void(GK_Parser *par);
static inline char get_c(GK_Parser *par);
static inline void next_c(GK_Parser *par);

// ------------------------------------------------------------------
// Parser config
static void gk_parse_text(GK_Parser *par);
static void gk_parse_text_arg(GK_Parser *par, GK_GraphicText *txt);

static void gk_parse_video(GK_Parser *par);
static void gk_parse_video_arg(GK_Parser *par, GK_GraphicVideo *txt);

static void gk_parse_button(GK_Parser *par);
static void gk_parse_button_arg(GK_Parser *par, GK_GraphicButton *txt);

static void gk_parse_image(GK_Parser *par);
static void gk_parse_image_arg(GK_Parser *par, GK_GraphicImage *txt);

// ====================================================================
// MAIN PARSE FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
void GK_ParseInit(GK_Parser *par, SDL_Renderer *render, const char *name_file) {
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
void GK_ParseLoop(GK_Parser *par) {
    assert(par);

    char buffer_kind[GK_PARSER_MAX_KIND] = "";
    int len = 0;

    while (par->cur_p < par->size) {
        sscanf(par->buffer + par->cur_p, "%s %n", buffer_kind, &len);
        par->cur_p += len;

        uint64_t hash = gk_get_hash(buffer_kind);
        switch (hash) {
            case gk_get_hash("text"):
                gk_parse_text(par);
                break;

            case gk_get_hash("button"):
                gk_parse_button(par);
                break;

            case gk_get_hash("video"):
                gk_parse_video(par);
                break;

            case gk_get_hash("image"):
                gk_parse_image(par);
                break;

            default:
                ExitF("Unknown Kind", );
        }
        gk_skip_void(par);
    }
    return ;
}

// ------------------------------------------------------------------
void GK_ParseDestroy(GK_Parser *par) {
    assert(par);

    free(par->buffer);
    return ;
}



// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
static int gk_get_file_size(const char *name) {
    assert(name);

    struct stat file_stat = {};
    if (stat(name, &file_stat) == -1)   ExitF("Bad Stat", 0);

    return (int) file_stat.st_size;
}

// ------------------------------------------------------------------
static char *gk_create_file_buffer(const char *name, int size) {
    assert(name);
    assert(size >= 0);

    FILE *stream = fopen(name, "rb");
    if (stream == NULL) ExitF("NULL File", NULL);

    char *buffer = (char *)calloc((size_t)size + 1, sizeof(char));
    if (buffer == NULL) {
        fclose(stream);
        ExitF("NULL Calloc", NULL);
    }

    fread(buffer, sizeof(char), (size_t)size, stream);
    fclose(stream);

    return buffer;
}

// ------------------------------------------------------------------
static void gk_skip_void(GK_Parser *par) {
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
static inline char get_c(GK_Parser *par) {
    return par->buffer[par->cur_p];
}

// ------------------------------------------------------------------
static inline void next_c(GK_Parser *par) {
    par->cur_p++;
}



// ====================================================================
// SUPPORT PARSE FUNCTIONS
// ====================================================================

// ====================================================================
// TEXT
static void gk_parse_text(GK_Parser *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicText *text = (GK_GraphicText *)calloc(1, sizeof(GK_GraphicText));
    if (text == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_text_arg(par, text);
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

static void gk_parse_text_arg(GK_Parser *par, GK_GraphicText *txt) {
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
            sscanf(par->buffer + par->cur_p, "$%[^$]$ %n", text_buffer, &len);
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
static void gk_parse_video(GK_Parser *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicVideo *vid = (GK_GraphicVideo *)calloc(1, sizeof(GK_GraphicVideo));
    if (vid == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_video_arg(par, vid);
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

static void gk_parse_video_arg(GK_Parser *par, GK_GraphicVideo *vid) {
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
            sscanf(par->buffer + par->cur_p, "$%[^$]$ %n", path_dir, &len);
            par->cur_p += len;

            vid->size = size;
            vid->data = (SDL_Texture **)calloc((size_t)size, sizeof(SDL_Texture *));
            if (vid->data == NULL) {
                ExitF("NULL Calloc", );
            }

            len = (int)strlen(path_dir);
            for (int i = 0; i < size; i++) {
                sprintf(path_dir + len, "/%d.png", i);
                SDL_Texture *tex = gk_load_texture(par->render, path_dir);
                vid->data[i] = tex;
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
static void gk_parse_button(GK_Parser *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicButton *but = (GK_GraphicButton *)calloc(1, sizeof(GK_GraphicButton));
    if (but == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_button_arg(par, but);
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

static void gk_parse_button_arg(GK_Parser *par, GK_GraphicButton *but) {
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
        case gk_get_hash("action"): {
            int action = 0;
            sscanf(par->buffer + par->cur_p, "%d %n", &action, &len);
            par->cur_p += len;

            but->act = (GK_ActionKind)action;
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
static void gk_parse_image(GK_Parser *par) {
    assert(par);
    gk_skip_void(par);

    if (get_c(par) != '{') ExitF("Skiped {", );
    next_c(par);
    gk_skip_void(par);

    GK_GraphicImage *img = (GK_GraphicImage *)calloc(1, sizeof(GK_GraphicImage));
    if (img == NULL) ExitF("NULL Calloc", );

    while (get_c(par) != '}' && get_c(par) != '\0') {
        gk_parse_image_arg(par, img);
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

static void gk_parse_image_arg(GK_Parser *par, GK_GraphicImage *img) {
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
            sscanf(par->buffer + par->cur_p, "$%[^$]$ %n", path_buffer, &len);
            par->cur_p += len;

            img->tex = gk_load_texture(par->render, path_buffer);
            break;
        }

        default:
            ExitF("Unknown word in config", );
    }
    return ;
}
