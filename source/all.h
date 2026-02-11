#ifndef EVERYTHING_ALWAYS_H_
#define EVERYTHING_ALWAYS_H_

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <raylib.h>
#include <raygui.h>
#include <rlgl.h>

#include "chad.h"

typedef struct {
  Font font __attribute__((aligned));
  u16 horizontal, vertical __attribute__((aligned));
  u16 ups, fps __attribute__((aligned));
} game_t;

/* game.c */

void GameInitialize(game_t * game, char * window_name);
void GameDeinitialize(game_t * game);
void GameFrame(game_t * game, size_t frame, f32 x, f32 y);
Vector2 GameFrameVector(game_t * game, size_t frame);
void GameLoop(game_t * game);
i16 GameUpdate(game_t * game, timespec_t now);
void GameRender(game_t * game, f64 interpolation);
void GameReport(game_t * game, f32 fps, f32 ups, u32 total_fps, u32 total_ups) ;

/* raylib.c */

Font DefaultFont(char * choice);
void GuiLoadStyleDarkSimple(void);

/* ... */

#endif /* EVERYTHING_ALWAYS_H_ */
