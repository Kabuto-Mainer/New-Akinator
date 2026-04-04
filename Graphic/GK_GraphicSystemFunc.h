#ifndef GK_GRAPHIC_SYSTEM_H
#define GK_GRAPHIC_SYSTEM_H

#include "GK_DSLLib.h"
#include "GK_AllFunc.h"

GK_ActionKind gk_check_click_button(SDL_Event *event, GK_GraphicButton *but);

void gk_add_text(GK_Display *disp, const char *text, GK_ID id);
void gk_clear_text(GK_Display *disp, GK_ID id);
void gk_add_text_button(GK_Display *disp, const char *text, GK_ID id);

void gk_add_image(GK_Display *disp, const char *file, GK_ID id);
void gk_clear_image(GK_Display *disp, GK_ID id);

void gk_input_clear(GK_TextInput *inp, bool is_hidden);
void gk_input_append_text(GK_TextInput *inp, const char *text);
void gk_input_backspace(GK_TextInput *inp);
void gk_input_begin(GK_TextInput *inp, bool is_hidden);
void gk_input_end(GK_TextInput *inp);

inline void gk_input_hide(GK_TextInput *inp);
inline void gk_input_show(GK_TextInput *inp);

#endif /* GK_GRAPHIC_SYSTEM_H */
