#include <stdio.h>
#include <assert.h>

#include "GK_AllFunc.h"

// ====================================================================
// CONSTATS
// ====================================================================

int GK_SCREEN_WIDTH = 1280;
int GK_SCREEN_HEIGHT = 720;
const char *GK_SYSTEM_FONT = "Data/font.ttf";
extern const SDL_Color GK_FONT_COLOR = {255, 255, 255, 255};
const char *GK_CONFIG_OBJECT_FILE = "obj.conf";
const char *GK_CONFIG_MENU_FILE = "menu.conf";

// ====================================================================
// FUNCTIONS
// ====================================================================

int main() {
    GK_Main app{};

    GK_InitTree(&(app.tree));
    GK_InitDisplay(&(app.disp));

    GK_Parse(&(app.disp));

    app.disp.cur_menu = GK_MENU_INIT;
    GK_MainLoop(&app);
    GK_DestroyDisplay(&app.disp);

    return 0;
}

void GK_MainLoop(GK_Main *main_data) {
    assert(main_data);

    bool running = true;

    while (running) {
        const GK_ActionKind action = GK_PollAction(&main_data->disp);

        if (action == GK_ACTION_EXIT) {
            running = false;
            continue;
        }

        GK_Update(main_data, action);
        GK_Render(main_data);
        SDL_Delay(8);
    }
}
