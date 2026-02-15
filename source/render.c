#include "all.h"

static void RenderTiles(game_t * game);
static void RenderPlayers(game_t * game);
static void RenderBombs(game_t * game);
static void RenderBottomText(game_t * game);

void Render(game_t * game, f64 interpolation) {
  (void)game;
  (void)interpolation;

  BeginDrawing();

#ifndef NDEBUG
  ClearBackground(BLACK);
#else
  ClearBackground(COLOR_TO_RAYLIB(game->tiles.color));
#endif
  BeginMode2D(game->camera);
  RenderTiles(game);
  RenderBombs(game);
  RenderPlayers(game);
  EndMode2D();
  RenderBottomText(game);
  /* --- */
  rlDrawRenderBatchActive();
  SwapScreenBuffer();
}

void RenderTiles(game_t * game) {
  for (int i = 0; i < game->config.map_x; ++i) {
    for (int j = 0; j < game->config.map_y; ++j) {
      #ifndef NDEBUG
      if ((game->tiles.state[i][j]._ | 1) & PASSIBLE || game->tiles.state[i][j]._ == IMPASSIBLE_NOTHING) {
        DrawRectangleRec((Rectangle) {i*game->config.spritesheet_scale, j*game->config.spritesheet_scale, game->config.spritesheet_scale, game->config.spritesheet_scale}, COLOR_TO_RAYLIB(game->tiles.color));
      }
      #endif

      if (game->tiles.state[i][j]._ < 2) {
        DrawTextureRec(
          game->spritesheet,
          game->tiles.wall[game->tiles.state[i][j].texture],
          (Vector2) {i*game->config.spritesheet_scale, j*game->config.spritesheet_scale},
          COLOR_TO_RAYLIB(game->tiles.color));
      }

      /* This almost requires horrible no good melding. Almost. */
      if (game->tiles.state[i][j]._ & EXPLOSIVE) {
        DrawTextureRec(
          game->spritesheet,
          game->tiles.explosion[game->tiles.state[i][j].texture],
          (Vector2) {i*game->config.spritesheet_scale, j*game->config.spritesheet_scale},
          WHITE);
      }
    }
  }
}

void RenderPlayers(game_t * game) {
  for (int i = 0; i < PLAYER_LIMIT; ++i) {
    if (game->players.state[i].alive) {
      DrawTextureRec(
          game->spritesheet,
          game->players.player[game->players.state[i].direction],
          (Vector2) {game->players.x[i] * game->config.spritesheet_scale, game->players.y[i] * game->config.spritesheet_scale},
          COLOR_TO_RAYLIB(game->players.color[i]));
    }
  }
}

void RenderBombs(game_t * game) {
  for (int i = 0; i < PLAYER_LIMIT; ++i) {
    for (int j = 0; j < PLAYER_LIMIT; ++j) {
      if (game->bombs.timer[i][j]) {
        DrawTextureRec(
          game->spritesheet,
          game->bombs.bomb[game->bombs.timer[i][j] % 4],
          (Vector2) {game->bombs.x[i][j]*game->config.spritesheet_scale, game->bombs.y[i][j]*game->config.spritesheet_scale},
          COLOR_TO_RAYLIB(game->bombs.color[game->bombs.timer[i][j]%2]));
      }
    }
  }
}

static void RenderBottomText(game_t * game) {
  BeginScissorMode(0, game->config.resolution_y - FONT_SIZE, game->config.resolution_x, FONT_SIZE);
  ClearBackground(BLACK);
  char center_string[128];
  snprintf(center_string, 128, "%02d:%02d", game->time_limit / game->config.ups / 60, game->time_limit / game->config.ups % 60);
  DrawText(center_string, game->config.resolution_x / 2 - MeasureText(center_string, FONT_SIZE) / 2, game->config.resolution_y - FONT_SIZE, FONT_SIZE, WHITE);
  EndScissorMode();
}
