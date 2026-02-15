#include "all.h"

static void CheckKilled(game_t * game);
static void UpdateExplosions(game_t * game);
static void PlaceBomb(game_t * game);
static int CheckInputDebug(game_t * game);
static void CheckInputPlaceBomb(game_t * game);
static void CheckInputMovement(game_t * game);
static void UpdatePlayer(game_t * game);
static void UpdateBomb(game_t * game);

i16 Update(game_t * game, timespec_t now) {
  (void) now;
  PollInputEvents();
  GameResize(game);
  if (CheckInputDebug(game)) { return 1; }
  CheckInputMovement(game);
  CheckInputPlaceBomb(game);
  UpdatePlayer(game);
  UpdateBomb(game);
  CheckKilled(game);
  UpdateExplosions(game);
  return 0;
}

static void CheckKilled(game_t * game) {
  for (size_t i = 0; i < PLAYER_LIMIT; ++i) {
    if (game->tiles.state[game->players.x[i]][game->players.y[i]].lethal) {
      game->players.state[i].alive = 0;
    }
  }
}


static void UpdateExplosions(game_t * game) {
  size_t i, j;
  for (i = 0; i < game->config.map_x; ++i) {
    for (j = 0; j < game->config.map_y; ++j) {
      if (game->tiles.state[i][j]._ >= PASSIBLE_EXPLOSIVE_LETHAL
      &&  game->tiles.state[i][j]._ <= PASSIBLE_EXPLOSIVE_LETHAL_END) {
        if (game->tiles.state[i][j]._ == PASSIBLE_EXPLOSIVE_LETHAL_END)
        { game->tiles.state[i][j]._ = PASSIBLE_NOTHING; }
        else
        { ++game->tiles.state[i][j]._; }
      }
    }
  }
}

static void PlaceBomb(game_t * game) {
  auto state = &game->players.state[game->client];
  if (state->bomb_count < state->bomb_limit) {
    game->tiles.state
      [game->players.x[game->client]]
      [game->players.y[game->client]]._ = IMPASSIBLE_NOTHING;
    game->bombs.x[game->client][state->bomb_count] = game->players.x[game->client];
    game->bombs.y[game->client][state->bomb_count] = game->players.y[game->client];
    game->bombs.state[game->client][state->bomb_count].power = state->power;
    game->bombs.state[game->client][state->bomb_count].pierce = state->pierce;
    game->bombs.state[game->client][state->bomb_count].bounce = state->bounce;
    game->bombs.timer[game->client][state->bomb_count] = game->config.ups;
    ++state->bomb_count;
  }
}

static int CheckInputDebug(game_t * game) {
  switch (GetKeyPressed()) {
  case KEY_ESCAPE: return 1;
#ifndef NDEBUG
  case KEY_F1: GameReinitialize(game); break;
  case KEY_R: MultiPlayer(game); break;
  case KEY_T: if (game->client <  3) game->client++; break;
  case KEY_G: if (game->client != 0) game->client--; break;
#endif
  }
  return 0;
}

static void CheckInputPlaceBomb(game_t * game) {
  auto state = &game->players.state[game->client];
  if ((IsKeyPressed(KEY_FIVE) ||  IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_U) || IsKeyPressed(KEY_O)
  ||   IsKeyPressed(KEY_M)    || IsKeyPressed(KEY_PERIOD) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_C)
  ||   IsKeyPressed(KEY_Q)    || IsKeyPressed(KEY_E)      || IsKeyPressed(KEY_ENTER))
  && state->alive)
  { PlaceBomb(game); }
}


static void CheckInputMovement(game_t * game) {
  auto state = &game->players.state[game->client];

  state->moving = 0;
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_I) || IsKeyPressed(KEY_W))
  { state->moving = 1; state->direction = UP; }
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_J) || IsKeyPressed(KEY_A))
  { state->moving = 1; state->direction = LEFT; }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_K) || IsKeyPressed(KEY_S))
  { state->moving = 1; state->direction = DOWN; }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_SIX) || IsKeyPressed(KEY_L) || IsKeyPressed(KEY_D))
  { state->moving = 1; state->direction = RIGHT; }

  /* if (IsKeyDown(KEY_EIGHT) || IsKeyDown(KEY_I) || IsKeyDown(KEY_W) */
  /* ||  IsKeyDown(KEY_FOUR)  || IsKeyDown(KEY_J) || IsKeyDown(KEY_A) */
  /* ||  IsKeyDown(KEY_TWO)   || IsKeyDown(KEY_K) || IsKeyDown(KEY_S) */
  /* ||  IsKeyDown(KEY_SIX)   || IsKeyDown(KEY_L) || IsKeyDown(KEY_D)) */
  /* { state->moving = 1; } */

  /* if (IsKeyReleased(KEY_EIGHT) || IsKeyReleased(KEY_I) || IsKeyReleased(KEY_W) */
  /* ||  IsKeyReleased(KEY_FOUR)  || IsKeyReleased(KEY_J) || IsKeyReleased(KEY_A) */
  /* ||  IsKeyReleased(KEY_TWO)   || IsKeyReleased(KEY_K) || IsKeyReleased(KEY_S) */
  /* ||  IsKeyReleased(KEY_SIX)   || IsKeyReleased(KEY_L) || IsKeyReleased(KEY_D)) */
  /* { state->moving = 0; } */
}

static void UpdatePlayer(game_t * game) {
  auto state = &game->players.state[game->client];
  i16 * player_x = &game->players.x[game->client];
  i16 * player_y = &game->players.y[game->client];
  /* f32 * animation_x = &game->players.animation_x[game->client]; */
  /* f32 * animation_y = &game->players.animation_y[game->client]; */
  float direction_x = -(state->direction == LEFT) + (state->direction == RIGHT);
  float direction_y = -(state->direction == UP) + (state->direction == DOWN);

  i16 delta_x = direction_x * state->moving;
  i16 delta_y = direction_y * state->moving;

  if (*player_x + delta_x >= 0
        &&  *player_x + delta_x < game->config.map_x
        &&  game->tiles.state[*player_x + delta_x][*player_y]._ & PASSIBLE)
  { *player_x += delta_x; }
  if (*player_y + delta_y >= 0
        && *player_y + delta_y < game->config.map_y
        &&  game->tiles.state[*player_x][*player_y + delta_y]._ & PASSIBLE)
  { *player_y += delta_y; }
}

static void UpdateBomb(game_t * game) {
  size_t i, j, k;
  ssize_t
    offset_x[4] = {-1, 1, 0, 0},
    offset_y[4] = { 0, 0, -1, 1};
  for (i = 0; i < PLAYER_LIMIT; ++i) {
    for (j = 0; j < BOMB_LIMIT; ++j) {
      if (game->bombs.timer[i][j]) {
        --game->bombs.timer[i][j];
        if (!game->bombs.timer[i][j]) {
	  ssize_t block[4] = {0};
          for (k = 0; k < 4 * game->players.state[i].power; ++k) {
	    if (block[k%4]) { continue; }
            i16
              rx = game->bombs.x[i][j] + offset_x[k%4] * ((k / 4) + 1),
              ry = game->bombs.y[i][j] + offset_y[k%4] * ((k / 4) + 1);
            if (rx < game->config.map_x && rx >= 0
	    &&  ry < game->config.map_y && ry >= 0)
            {
	      if (game->tiles.state[rx][ry]._ & PASSIBLE) {
		game->tiles.state[rx][ry]._ = PASSIBLE_EXPLOSIVE_LETHAL;
	      } else if (game->tiles.state[rx][ry]._ == IMPASSIBLE_BREAKABLE_WALL) {
		game->tiles.state[rx][ry]._ = PASSIBLE_EXPLOSIVE_LETHAL;
		if (!game->players.state[i].pierce) {
		  block[k%4] = 1;
		}
	      } else {
		block[k%4] = 1;
	      }
	    }
          }
          game->tiles.state[game->bombs.x[i][j]][game->bombs.y[i][j]]._ = PASSIBLE_EXPLOSIVE_LETHAL;
          --game->players.state[i].bomb_count;
        }
      }
    }
  }
}
