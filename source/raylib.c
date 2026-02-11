/* raygui.c */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <style_dark.h>
#pragma GCC diagnostic pop

/* removes the timewaster of decompressing and loading a font never used */
void GuiLoadStyleDarkSimple(void) {
  for (int i = 0; i < DARK_STYLE_PROPS_COUNT; i++) {
    GuiSetStyle(darkStyleProps[i].controlId, darkStyleProps[i].propertyId, darkStyleProps[i].propertyValue);
  }
}

/* raylib.c */

#include <raylib.h>

Font DefaultFont(char * choice) {
  Font font = LoadFont(choice);
  if (!IsFontValid(font)) { font = GetFontDefault(); }
  return font;
}

void RaylibInitialize(int horizontal, int vertical, char * window_name, Font default_font) {
#ifdef NDEBUG
  SetTraceLogLevel(LOG_NONE);
#endif
  /* SetConfigFlags(FLAG_WINDOW_RESIZABLE); */
  InitWindow(horizontal, vertical, window_name);
  SetWindowState(FLAG_WINDOW_HIDDEN);
  /* we should spawn this in the center of the screen and have our window scale to the limit of the screen */
  InitAudioDevice();
  SetWindowPosition(0, 0);
  GuiLoadStyleDarkSimple();
  GuiSetFont(default_font);
}

void RaylibDeinitialize(void) {
  SetWindowState(FLAG_WINDOW_HIDDEN);
  CloseAudioDevice();
  CloseWindow();
}
