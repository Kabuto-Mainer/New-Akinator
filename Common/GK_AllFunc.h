#ifndef GK_ALL_FUNC_H
#define GK_ALL_FUNC_H

#include "GK_MainType.h"

// ====================================================================
#define ExitF(__text__, __ret_val__) \
do {printf("ERROR in %s:%d||%s\n", __FILE__,__LINE__,__text__); \
    return __ret_val__; } while (0)
// ====================================================================

void GK_MainLoop(GK_Main *main_data);

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
void GK_LoadTree(GK_Tree *tree, const char *name);

// ====================================================================
// PARSER FUNCTIONS
// ====================================================================

void GK_Parse(GK_Display *disp);

void GK_ParseObjectInit(GK_ParserObject *par, SDL_Renderer *render, const char *name_file);
int  GK_ParseObjectLoop(GK_ParserObject *par);
void GK_ParseObjectDestroy(GK_ParserObject *par);

void GK_ParseMenuInit(GK_ParserMenu *par, GK_Menu *menus, const char *name_file);
void GK_ParseMenuLoop(GK_ParserMenu *par);
void GK_ParseMenuDestroy(GK_ParserMenu *par);

// ====================================================================
// SUPPORT FUNCTIONS
// ====================================================================

int gk_get_file_size(const char *name);
char *gk_create_file_buffer(const char *name, int size);

// ====================================================================
// DUMP FUNCTIONS
// ====================================================================

void GK_TreeDump(GK_Tree *tree);
void GK_CreateGraph(GK_TreeObject *null, const char *name_dot, const char *name_img);


#endif /* GK_ALL_FUNC_H */
