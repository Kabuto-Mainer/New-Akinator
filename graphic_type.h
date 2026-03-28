#ifndef GRAPHIC_TYPE_H
#define GRAPHIC_TYPE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

const static int GK_AMOUNT_MENU = 10;

enum GK_GraphicObjectKind {
    GK_GRAPHIC_BUTTON,
    GK_GRAPHIC_TEXT,
    GK_GRAPHIC_VIDEO,
    GK_GRAPHIC_IMAGE
};

typedef int GK_ID;

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

enum GK_ActionKind {
    GK_ACTION_NONE,
    GK_ACTION_BEGIN_GUESS,
    GK_ACTION_OPEN_ADMIN,
    GK_ACTION_EXIT,
    GK_ACTION_ANSWER_YES,
    GK_ACTION_ANSWER_NO,
    GK_ACTION_RETURN_TO_MENU,
    GK_ACTION_LOAD_DATA,
    GK_ACTION_UPLOAD_DATA,
    GK_ACTION_ADD_OBJECT,
    GK_ACTION_CONTROL_RECORD,
};

struct GK_GraphicButton {
    SDL_Rect place;
    struct {
        SDL_Texture *pressed;
        SDL_Texture *unpressed;
    } texture;

    bool is_pressed;
    bool is_hovered;

    GK_ActionKind act;
};

enum GK_GraphicTextKind {
    GK_GRAPHIC_TEXT_KIND_INPUT,
    GK_GRAPHIC_TEXT_KIND_OUTPUT
};

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

struct GK_GraphicVideo {
    SDL_Rect place;
    SDL_Texture **data;
    int size;
    int current;

    uint64_t delay;
    uint64_t last_time;
};

struct GK_GraphicImage {
    SDL_Rect place;
    SDL_Texture *tex;
};

struct GK_ObjectPool {
    GK_GraphicObject *pool;
    int size;
};

struct GK_Menu {
    GK_ID *data;
    int size;
};

struct GK_GraphicSystem {
    SDL_Window *win;
    SDL_Renderer *ren;
    TTF_Font *font;
};

#endif /* GRAPHIC_TYPE_H */
