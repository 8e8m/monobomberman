#include "all.h"

void MultiPlayer(game_t * game) {
  u8 width  = game->config.map_x;
  u8 height = game->config.map_y;
  int i, j;

  for (i = 0; i < PLAYER_LIMIT; ++i) {
    for (j = 0; j < BOMB_LIMIT; ++j) {
      game->bombs.timer[i][j] = 0;
    }
  }

  for (i = 0; i < width; ++i) {
    for (j = 0; j < height; ++j) {
      game->tiles.state[i][j]._ = rand() % 10 ? IMPASSIBLE_BREAKABLE_WALL : PASSIBLE_NOTHING;
    }
  }

  for (i = 1; i < width; i += 2) {
    for (j = 1; j < height; j += 2) {
      game->tiles.state[i][j]._ = IMPASSIBLE_WALL;
    }
  }

  bzero(game->players.state, sizeof(*game->players.state) * PLAYER_LIMIT);

  for (i = 0; i < MIN(PLAYER_LIMIT, game->config.player_count); ++i) {
    game->players.state[i].bomb_limit = 15;
    game->players.state[i].power = 2;
    game->players.state[i].speed = 2;
    game->players.state[i].alive = 1;
    game->players.state[i].direction = DOWN;
  }

  float player_x[4] =
    {0, (width-1), (width-1), 0                   };
  float player_y[4] =
    {0, (height-1), 0,                  (height-1)};

  u8 color[4] = {
    GAME_RED | GAME_GREEN | GAME_OPAQUE,
    GAME_RED | GAME_GREEN | GAME_BLUE | GAME_OPAQUE,
    GAME_GREEN | GAME_OPAQUE,
    GAME_BLUE | GAME_OPAQUE,
  };

  for (i = 0; i < MIN(game->config.player_count, PLAYER_LIMIT); ++i) {
    game->players.x[i] = player_x[i % 4];
    game->players.y[i] = player_y[i % 4];
    game->players.color[i] = color[i % 4];
  }

  /* areas that must be passible nothings */
  u8 offset_x[12] =
    {0, 1, 0,  width-1,  width-2,  width-1, width-1, width-2, width-1,        0,        1,        0};
  u8 offset_y[12] =
    {0, 0, 1, height-1, height-1, height-2,       0,       0,       1, height-1, height-1, height-2};

  for (i = 0; i < MIN((game->config.player_count * 3), 12); ++i) {
    game->tiles.state[offset_x[i]][offset_y[i]]._  = PASSIBLE_NOTHING;
  }

  for (i = 0; i < width; ++i) {
    for (j = 0; j < height; ++j) {
      printf("%3d ", game->tiles.state[i][j]._);
    }
    printf("\n");
  }
}
