#ifndef GK_ALL_FUNC_H
#define GK_ALL_FUNC_H

#include "GK_MainType.h"

// ====================================================================
#define ExitF(__text__, __ret_val__) \
do {printf("ERROR in %s:%d||%s\n", __FILE__,__LINE__,__text__); \
    return __ret_val__; } while (0)
// ====================================================================



// ====================================================================
// DISPLAY FUNCTIONS
// ====================================================================

void GK_InitDisplay(GK_Display *disp);
void GK_DestroyDisplay(GK_Display *disp);

void GK_DisplayTreeBranch(GK_Display *disp, GK_TreeObject *obj);
GK_ActionKind GK_PollAction(GK_Display *disp);

void GK_Update(GK_Main *app, GK_ActionKind action);
void GK_Render(GK_Main *app);


// ====================================================================
// TREE FUNCTIONS
// ====================================================================

void GK_InitTree(GK_Tree *tree);
void GK_MainLoop(GK_Main *main_data);


// ====================================================================
// PARSER FUNCTIONS
// ====================================================================

void GK_ParseInit(GK_Parser *par, SDL_Renderer *render, const char *name_file);
void GK_ParseLoop(GK_Parser *par);
void GK_ParseDestroy(GK_Parser *par);



#endif /* GK_ALL_FUNC_H */
