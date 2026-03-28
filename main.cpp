#include <stdio.h>
#include <assert.h>

#include "GK_AllFunc.h"

// ====================================================================
// CONSTATS
// ====================================================================

int GK_SCREEN_WIDTH = 1280;
int GK_SCREEN_HEIGHT = 720;
const char *GK_SYSTEM_FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";


// ====================================================================
// FUNCTIONS
// ====================================================================

int main() {
    GK_Main app{};

    GK_InitTree(&(app.tree));
    GK_InitDisplay(&(app.disp));

    app.disp.cur_menu = GK_MENU_START_GUESS;
    GK_MainLoop(&app);
    GK_DestroyDisplay(&app.disp);

    return 0;
}

void GK_MainLoop(GK_Main *main_data) {
    if (main_data == nullptr) {
        return;
    }

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
