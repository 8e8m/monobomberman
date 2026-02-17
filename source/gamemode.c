#include "all.h"

static void MapPrint(game_t * game) {
  u8 width  = game->config.map_x;
  u8 height = game->config.map_y;
  printf("texture, explosive, breakable, lethal, pickup\n");
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      tile_data_t * tile = &game->tiles.state[i][j];
      printf("%2d:%1d:%1d:%1d:%1d ", tile->texture, tile->explosive, tile->breakable, tile->lethal, tile->pickup);
    }
    printf("\n");
  }
}

static void GamemodeReset(game_t * game) {
  for (int i = 0; i < PLAYER_LIMIT; ++i) {
    for (int j = 0; j < BOMB_LIMIT; ++j) {
      game->bombs.timer[i][j] = 0;
    }
  }

  bzero(game->players.state, sizeof(*game->players.state) * PLAYER_LIMIT);
}

static void PlayerPlaceCorners(game_t * game, u8 * color, size_t color_count) {
  u8 width  = game->config.map_x;
  u8 height = game->config.map_y;
  float player_x[4] =
    {0, (width-1), (width-1), 0                   };
  float player_y[4] =
    {0, (height-1), 0,                  (height-1)};

  for (int i = 0; i < MIN(game->config.player_count, PLAYER_LIMIT); ++i) {
    game->players.x[i] = player_x[i % color_count];
    game->players.y[i] = player_y[i % color_count];
    game->players.color[i] = color[i % color_count];
  }
}

static void MapClearCorners(game_t * game) {
  /* areas that must be passible nothings */
  u8 width  = game->config.map_x;
  u8 height = game->config.map_y;

  u8 offset_x[12] =
    {0, 1, 0,  width-1,  width-2,  width-1, width-1, width-2, width-1,        0,        1,        0};
  u8 offset_y[12] =
    {0, 0, 1, height-1, height-1, height-2,       0,       0,       1, height-1, height-1, height-2};

  for (int i = 0; i < MIN((game->config.player_count * 3), 12); ++i) {
    game->tiles.state[offset_x[i]][offset_y[i]]  = passable_tile;
  }
}

void MultiPlayer(game_t * game) {
  u8 width  = game->config.map_x;
  u8 height = game->config.map_y;
  int i, j;

  GamemodeReset(game);

  for (i = 0; i < width; ++i) {
    for (j = 0; j < height; ++j) {
      game->tiles.state[i][j] = rand() % 10 ? breakable_wall : passable_tile;
      if (rand() % 3 == 0)
      {
        game->tiles.state[i][j].pickup = rand() % 5 ? POWERUP_POWER : POWERUP_BOMB;
      } else if (rand() % 10 == 0) {
        game->tiles.state[i][j].pickup = POWERUP_PIERCE;
      }
    }
  }

  for (i = 1; i < width; i += 2) {
    for (j = 1; j < height; j += 2) {
      game->tiles.state[i][j] = impassable_wall;
    }
  }

  for (i = 0; i < MIN(PLAYER_LIMIT, game->config.player_count); ++i) {
    game->players.state[i].bomb_limit = 1;
    game->players.state[i].power = 2;
    game->players.state[i].speed = 2;
    game->players.state[i].alive = 1;
    game->players.state[i].direction = DOWN;
  }

  game->time_limit = game->config.ups * 60 * 3;

  u8 color[4] = {
    GAME_RED | GAME_GREEN | GAME_OPAQUE,
    GAME_RED | GAME_GREEN | GAME_BLUE | GAME_OPAQUE,
    GAME_GREEN | GAME_OPAQUE,
    GAME_BLUE | GAME_OPAQUE,
  };

  PlayerPlaceCorners(game, color, 4);
  MapClearCorners(game);
  MapPrint(game);
}
