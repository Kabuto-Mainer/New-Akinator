#include <stdio.h>
#include <assert.h>

#include "type.h"


int main() {
    return 0;
}


void GK_MainLoop(GK_Main *main_data) {
    assert(main_data);

    GK_Display disp = main_data->disp;
    GK_Tree tree = main_data->tree;

    GK_ActionKind action = GK_ACTION_NONE;
    while (action != GK_ACTION_EXIT) {
        action = GK_
    }
}
