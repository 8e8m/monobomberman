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
#include "all.h"
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

void RaylibInitialize(game_t * game) {
#ifdef NDEBUG
  SetTraceLogLevel(LOG_NONE);
#endif
  i16 horizontal = game->config.resolution_x;
  i16 vertical = game->config.resolution_y;
  /* SetConfigFlags(FLAG_WINDOW_RESIZABLE); */
  InitWindow(horizontal, vertical, game->config.window_name);
  SetWindowState(FLAG_WINDOW_HIDDEN);
  /* we should spawn this in the center of the screen and have our window scale to the limit of the screen */
  InitAudioDevice();
  int monitor = GetCurrentMonitor();
  int width = GetMonitorWidth(monitor), height = GetMonitorHeight(monitor);
  SetWindowPosition(
    width/2-horizontal/2,
    height/2-vertical/2);
  GuiLoadStyleDarkSimple();
  game->font = DefaultFont(game->config.font);
  GuiSetFont(game->font);
}

void RaylibDeinitialize(void) {
  SetWindowState(FLAG_WINDOW_HIDDEN);
  CloseAudioDevice();
  CloseWindow();
}
