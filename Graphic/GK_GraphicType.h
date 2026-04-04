#ifndef GK_GRAPHIC_TYPE_H
#define GK_GRAPHIC_TYPE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>


typedef int GK_ID;


// ====================================================================
// CONSTANTS
// ====================================================================
const static int GK_AMOUNT_MENU = 10;
constexpr static int GK_MAX_INPUT = 128;
constexpr GK_ID GK_INVALID_ID = -1;

// ====================================================================
// ENUMS
// ====================================================================

enum GK_GraphicObjectKind {
    GK_GRAPHIC_BUTTON,
    GK_GRAPHIC_TEXT,
    GK_GRAPHIC_VIDEO,
    GK_GRAPHIC_IMAGE
};

enum GK_ActionKind {
    GK_ACTION_NONE,

    GK_ACTION_BEGIN_PLAY,
    GK_ACTION_RETURN_TO_START,
    GK_ACTION_EXIT,
    GK_ACTION_SKIP,

    GK_ACTION_CONTROL_MUSIC,

    GK_ACTION_CHOICE_0,
    GK_ACTION_CHOICE_1,
    GK_ACTION_CHOICE_2,
    GK_ACTION_CHOICE_3,
};

enum GK_GraphicTextKind {
    GK_GRAPHIC_TEXT_KIND_OUTPUT,
    GK_GRAPHIC_TEXT_KIND_INPUT,
};

enum GK_InputMode {
    GK_INPUT_NONE,
    GK_INPUT_TEXT
};

// ====================================================================
// LIBRARY OBJECTS
// ====================================================================

// ------------------------------------------------------------------
struct GK_GraphicButton {
    SDL_Rect place;
    struct {
        SDL_Texture *pressed;
        SDL_Texture *unpressed;
    } texture;

    char *text;

    bool is_pressed;
    bool is_hovered;
    bool is_switcher;

    GK_ActionKind act;
};

// ------------------------------------------------------------------
struct GK_GraphicTextObject {
    char *syms;
    int size;
    int cur_pos;
};

struct GK_GraphicText {
    SDL_Rect place;
    SDL_Texture *background;

    GK_GraphicTextKind kind;
    GK_GraphicTextObject data;
};

// ------------------------------------------------------------------
struct GK_GraphicVideo {
    SDL_Rect place;
    SDL_Texture **data;
    int size;
    int current;

    uint64_t delay;
    uint64_t last_time;
};

// ------------------------------------------------------------------
struct GK_GraphicImage {
    SDL_Rect place;
    SDL_Texture *tex;
};


// ------------------------------------------------------------------
struct GK_GraphicObject {
    GK_GraphicObjectKind kind;
    GK_ID id;
    bool must_show;
    union {
        GK_GraphicButton *but;
        GK_GraphicText *text;
        GK_GraphicVideo *vid;
        GK_GraphicImage *img;
    } data;
};

// ------------------------------------------------------------------
struct GK_ObjectPool {
    GK_GraphicObject *pool;
    int size;
};

// ------------------------------------------------------------------
struct GK_Menu {
    GK_ID *data;
    int size;
    int capacity;
};

// ------------------------------------------------------------------
struct GK_GraphicSystem {
    SDL_Window *win;
    SDL_Renderer *ren;
    TTF_Font *font;
};

struct GK_TextInput {
    char buffer[128];
    int size;
    int max_size;
    int cursor;

    bool is_active;
    bool is_hidden;
};

#endif /* GK_GRAPHIC_TYPE_H */
