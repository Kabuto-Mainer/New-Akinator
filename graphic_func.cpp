#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "type.h"
#include "DSL_lib.h"

extern int GK_SCREEN_WIDTH;
extern int GK_SCREEN_HEIGHT;
extern const char *GK_SYSTEM_FONT;


// ==================================================================
int GK_CreateDisplay (GK_Display *disp) {
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

    sup_GK_LoadResources(disp);
}

















// ==================================================================
void static sup_GK_LoadResources(GK_Display *disp) {
    assert(disp);
}

// ==================================================================
inline static SDL_Texture * sup_GK_LoadTexture(SDL_Renderer *render, const char *name) {
    assert(render);
    assert(name);

    SDL_Texture *tex = IMG_LoadTexture(render, name);
    if (tex == NULL) EXIT_F("NULL Texture", NULL);
    return tex;
}

// ==================================================================
inline static void sup_GK_ShowImage(SDL_Renderer *render,
                SDL_Texture *tex, const SDL_Rect place) {
    assert(render);
    assert(tex);

    SDL_RenderCopy(render, tex, NULL, &place);
    return ;
}

// ==================================================================
static void sup_GK_PlayVideo(SDL_Renderer *render, GK_GraphicVideo *vid,
        uint64_t cur_time) {
    assert(render);
    assert(vid);

    uint64_t diff = cur_time - vid->last_time;
    if (diff < vid->delay) return ;

    if (vid->current == vid->size) vid->current = 0;
    sup_GK_ShowImage(render, vid->data[vid->current], vid->place);

    vid->current = 0;
    vid->last_time = cur_time;
    return ;
}








// ==================================================================
static GK_ActionKind sup_GK_CheckClickButton(SDL_Event *event, GK_GraphicButton *but) {
    assert(event);
    assert(but);

    if (event->type != SDL_MOUSEMOTION && event->type != SDL_MOUSEBUTTONDOWN)
        return GK_ACTION_NONE;

    int mouse_x = event->button.x;
    int mouse_y = event->button.y;

    if (event->type == SDL_MOUSEMOTION) {
        but->is_pressed = false;
        but->is_hovered = false;

        if (mouse_x >= but->place.x &&
            mouse_x <= but->place.x + but->place.w &&
            mouse_y >= but->place.y &&
            mouse_y <= but->place.y + but->place.h) {
            but->is_hovered = true;
        }

        return GK_ACTION_NONE;
    } else if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_LEFT) {

        if (mouse_x >= but->place.x &&
            mouse_x <= but->place.x + but->place.w &&
            mouse_y >= but->place.y &&
            mouse_y <= but->place.y + but->place.h) {
            but->is_pressed = true;
            but->is_hovered = true;
            return but->act;
        }
    }
    but->is_hovered = false;
    but->is_pressed = false;

    return GK_ACTION_NONE;
}

// ==================================================================
static void sup_GK_AddText(GK_Display *disp, const char *text, GK_ID id) {
    assert(disp);
    assert(text);
    if (disp->data->size < id) EXIT_F("Bad ID", );

    GK_GraphicObject obj = disp->data->pool[id];
    if (obj.kind != GK_GRAPHIC_TEXT) EXIT_F("ID is not TEXT", );
    if (obj.data.text->kind != GK_GRAPHIC_TEXT_KIND_OUTPUT) EXIT_F("ID is input text", );

    GK_GraphicText *txt = obj.data.text;
    char *buffer = txt->data.syms;
    size_t add_len = strlen(text);

    if (buffer == NULL) {
        buffer = (char *)realloc(buffer, add_len + txt->data.size);
    } else {
        buffer = (char *)calloc(1, add_len);
    }

    strcpy(buffer + txt->data.size, text);

    txt->data.syms = buffer;
    txt->data.size += add_len;

    return ;
}

// ==================================================================
static void sup_GK_ClearText(GK_Display *disp, GK_ID id) {
    assert(disp);
    if (disp->data->size < id) EXIT_F("Bad ID");

    GK_GraphicObject obj = disp->data->pool[id];
    if (obj.kind != GK_GRAPHIC_TEXT) EXIT_F("ID is not TEXT");
    if (obj.data.text->kind != GK_GRAPHIC_TEXT_KIND_OUTPUT) EXIT_F("ID is input text");

    GK_GraphicText *txt = obj.data.text;
    if (txt->data.syms != NULL)     free(txt->data.syms);

    txt->data.syms = NULL;
    txt->data.size = 0;

    return ;
}

// ==================================================================
static void sup_GK_AddImage(GK_Display *disp, const char *file, GK_ID id) {
    assert(disp);
    if (disp->data->size < id) EXIT_F("Bad ID");

    GK_GraphicObject obj = disp->data->pool[id];
    if (obj.kind != GK_GRAPHIC_IMAGE) EXIT_F("ID is not IMAGE", );

    GK_GraphicImage *img = obj.data.img;
    if (img->tex != NULL)   SDL_DestroyTexture(img->tex);

    img->tex = sup_GK_LoadTexture(disp->sys.ren, file);
    return ;
}

// ==================================================================
static void sup_GK_ClearImage(GK_Display *disp, GK_ID id) {
    assert(disp);
    if (disp->data->size < id) EXIT_F("Bad ID", );

    GK_GraphicObject obj = disp->data->pool[id];
    if (obj.kind != GK_GRAPHIC_IMAGE) EXIT_F("ID is not IMAGE", );

    GK_GraphicImage *img = obj.data.img;
    if (img->tex != NULL)   SDL_DestroyTexture(img->tex);

    img->tex = NULL;
    return ;
}

// ==================================================================
static GK_ID sup_GK_GetTextWindow(GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:
            return GK_GUESS_MAIN_TEXT;

        case GK_MENU_SUCCESS:
            return GK_SUCCESS_MAIN_TEXT;

        case GK_MENU_N_SUCCESS:
            return GK_N_SUCCESS_MAIN_TEXT;

        case GK_MENU_INIT:
        case GK_MENU_START_GUESS:
        case GK_MENU_ADMIN_MENU:
        case GK_MENU_EXIT_MENU:
        default:
            return -1;
    }
    return -1;
}

// ==================================================================
static GK_ID sup_GK_GetImageWindow(GK_Display *disp) {
    assert(disp);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:
            return GK_GUESS_IMAGE;

        case GK_MENU_SUCCESS:
            return GK_SUCCESS_IMAGE;

        case GK_MENU_N_SUCCESS:
            return GK_N_SUCCESS_IMAGE;

        case GK_MENU_INIT:
        case GK_MENU_START_GUESS:
        case GK_MENU_ADMIN_MENU:
        case GK_MENU_EXIT_MENU:
        default:
            return -1;
    }
    return -1;
}








void sup_GK_DisplayTreeBranch(GK_Display *disp, GK_TreeObject *obj) {
    assert(disp);
    assert(obj);

    GK_ID text_win = sup_GK_GetTextWindow(disp);
    GK_ID img_win = sup_GK_GetImageWindow(disp);

    sup_GK_ClearText(disp, text_win);
    sup_GK_ClearImage(disp, img_win);

    switch (disp->cur_menu) {
        case GK_MENU_GUESS:
            sup_GK_AddText(disp, "Это ", text_win);
            if (obj->set & GK_TREE_OBJECT_HAVE_TEXT) {
                sup_GK_AddText(disp, obj->text, text_win);
            } else {
                sup_GK_AddText(disp, "Tree ERROR ", text_win);
            }
            sup_GK_AddText(disp, "?", text_win);

            if (obj->set & GK_TREE_OBJECT_HAVE_IMAGE) {
                sup_GK_AddImage(disp, obj->files.img, img_win);
            }

        case GK_MENU_SUCCESS:
        case GK_MENU_N_SUCCESS:
    }
}



















// ==================================================================
static GK_ActionKind sup_GK_PollAction(SDL_Event *event, GK_Display *disp) {
    assert(event);
    assert(disp);

    GK_GraphicObject *pool = disp->data->pool;
    GK_Menu *menu = &(disp->menus[disp->cur_menu]);
    GK_ActionKind ret_val = GK_ACTION_NONE;

    for (int i = 0; i < menu->size; i++) {
        GK_GraphicObject obj = pool[menu->data[i]];
        if (obj.kind != GK_GRAPHIC_BUTTON) continue;

        ret_val = sup_GK_CheckClickButton(event, obj.data.but);
        if (ret_val != GK_ACTION_NONE)  return ret_val;
    }

    return GK_ACTION_NONE;
}


// ==================================================================
static void sup_GK_Update(GK_Main *app, GK_ActionKind action) {
    assert(app);

    switch (action) {
        case GK_ACTION_NONE:
            return;

        case GK_ACTION_BEGIN_GUESS:
            app->disp.cur_menu = GK_MENU_GUESS;
            return;

        case GK_ACTION_OPEN_ADMIN:
            app->disp.cur_menu = GK_MENU_ADMIN_MENU;
            return;

        case GK_ACTION_RETURN_TO_MENU:
            app->disp.cur_menu = GK_MENU_START_GUESS;
            return;

        case GK_ACTION_ANSWER_YES:
            if (app->tree.cur->kind == GK_TREE_OBJECT_LEAF) {
                app->disp.cur_menu = GK_MENU_SUCCESS;
                return ;
            }

            app->tree.cur = app->tree.cur->branch.yes;
            return ;

        case GK_ACTION_ANSWER_NO:
            if (app->tree.cur->kind == GK_TREE_OBJECT_LEAF) {
                app->disp.cur_menu = GK_MENU_N_SUCCESS;
                return ;
            }

            app->tree.cur = app->tree.cur->branch.no;
            return ;

        case GK_ACTION_EXIT:
        default:
            return ;
    }
    return ;
}

// ==================================================================
static void sup_GK_Render(GK_Main *app) {
    assert(app);

    GK_GraphicObject *pool = app->disp.data->pool;
    GK_Menu *menu = &(app->disp.menus[app->disp.cur_menu]);

    for (GK_ID i = 0; i < menu->size; i++) {
        GK_GraphicObject *obj = &(pool[menu->data[i]]);
        if (obj->must_show == false)    continue;

        switch (obj->kind) {
            case GK_GRAPHIC_BUTTON:


            case GK_GRAPHIC_IMAGE:
            case GK_GRAPHIC_TEXT:
            case GK_GRAPHIC_VIDEO:
        }
    }

}


