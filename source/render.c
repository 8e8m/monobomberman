#include "all.h"

static void RenderTiles(game_t * game);
static void RenderPlayers(game_t * game);
static void RenderBombs(game_t * game);
static void RenderBottomText(game_t * game);

void Render(game_t * game, f64 interpolation) {
  (void)game;
  (void)interpolation;

  BeginDrawing();

  ClearBackground(COLOR_TO_RAYLIB(game->tiles.color));
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

always_inline void AtlasDraw(game_t * game, u16 atlas_number, u8 color, int i, int j) {
  DrawTextureRec(
    game->spritesheet,
    game->atlas[atlas_number],
    (Vector2) {i*game->config.spritesheet_scale, j*game->config.spritesheet_scale},
    COLOR_TO_RAYLIB(color));
}

void RenderTiles(game_t * game) {
  for (int i = 0; i < game->config.map_x; ++i) {
    for (int j = 0; j < game->config.map_y; ++j) {
      tile_data_t * tile = &game->tiles.state[i][j];
      if (!tile->passable) {
	if (tile->texture)
	{ AtlasDraw(game, tile->breakable ? 1 : 0, game->tiles.color, i, j); }
      } else if (tile->pickup) {
	AtlasDraw(game, RENDER_POWERUP_BOMB + tile->pickup - 1, GAME_WHITE | GAME_OPAQUE, i, j);
      }
      if (tile->explosive) {
	AtlasDraw(game, RENDER_EXPLOSION_START + tile->texture, game->bombs.color[0], i, j);
      }
    }
  }
}

void RenderPlayers(game_t * game) {
  for (int i = 0; i < PLAYER_LIMIT; ++i) {
    if (game->players.state[i].alive) {
      AtlasDraw(game, RENDER_PLAYER_RIGHT + game->players.state[i].direction,
		game->players.color[i], game->players.x[i], game->players.y[i]);
    }
  }
}

void RenderBombs(game_t * game) {
  for (int i = 0; i < PLAYER_LIMIT; ++i) {
    for (int j = 0; j < PLAYER_LIMIT; ++j) {
      if (game->bombs.timer[i][j]) {
	AtlasDraw(game, RENDER_BOMB_0 + game->bombs.timer[i][j] % 4,
		  game->bombs.color[game->bombs.timer[i][j]%2],
		  game->bombs.x[i][j], game->bombs.y[i][j]);
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
