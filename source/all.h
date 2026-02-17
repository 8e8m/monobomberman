#ifndef EVERYTHING_ALWAYS_H_
#define EVERYTHING_ALWAYS_H_

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <netinet/in.h>

#include <raylib.h>
#include <raygui.h>
#include <rlgl.h>

#include "chad.h"

#define StepStart(prefix)                                               \
  prefix##_delta = timespec_sub(now, prefix##_last);                    \
  if (timespec_cmp(prefix##_delta, prefix##_interval) >= 0) {           \
    (void) 0
    /* ...  */
#define StepStop(prefix)                                                \
    prefix##_last = timespec_add(prefix##_last, prefix##_interval);     \
    if (timespec_cmp(prefix##_last, now) < 0) {                         \
      prefix##_last = now;                                              \
    }                                                                   \
  }

/* this is precisely why namespacing is a good idea */
#define GAME_RED     (0x03<<0)
#define GAME_GREEN   (0x03<<2)
#define GAME_BLUE    (0x03<<4)
#define GAME_OPAQUE  (0x03<<6)
#define GAME_WHITE (GAME_RED | GAME_GREEN | GAME_BLUE)
#define COLOR_TO_RAYLIB(c) (Color) { \
    85*((c &   GAME_RED)>>0), \
    85*((c & GAME_GREEN)>>2), \
    85*((c &  GAME_BLUE)>>4), \
    85*((c &  GAME_OPAQUE)>>6), }

#define FONT_SIZE 20

#define TEXTURE_LIMIT (3*8)

/* Spritesheets will be (N*4xN*6)
   group 0 is   [0-1][0  ]     / un/breakable walls
   group 1 (just explosions, the nulls are skipped for sake of compactness)
           is   [2-3][0  ]    |
   group 2 is   [0-3][1  ]    | Negatives use color mask
                [0-2][2  ]    /
   group 3 is   [0-3][3  ]   / player
   group 4 is   [0-3][4  ]  / bomb
   group 5 is   [0-3][4  ] / enemy
 */

#define TILE_LENGTH_LIMIT 21

enum powerup {
  POWERUP_BOMB = 1,
  POWERUP_POWER,
  POWERUP_SPEED,
  POWERUP_PIERCE,
  POWERUP_KICK,
  POWERUP_THROW,
  POWERUP_BOUNCE,
  POWERUP_CURSE,
};

/* highly dependent on atlas definition in game.c */
enum atlas {
  RENDER_UNBREAKABLE = 0,
  RENDER_BREAKABLE,

  RENDER_EXPLOSION_START,
  RENDER_EXPLOSION_END,

  RENDER_POWERUP_BOMB,
  RENDER_POWERUP_POWER,
  RENDER_POWERUP_SPEED,
  RENDER_POWERUP_PIERCE,
  RENDER_POWERUP_KICK,
  RENDER_POWERUP_THROW,
  RENDER_POWERUP_BOUNCE,
  RENDER_POWERUP_CURSE,

  RENDER_PLAYER_RIGHT,
  RENDER_PLAYER_LEFT,
  RENDER_PLAYER_UP,
  RENDER_PLAYER_DOWN,

  RENDER_BOMB_0,
  RENDER_BOMB_1,
  RENDER_BOMB_2,
  RENDER_BOMB_3,

  RENDER_ENEMY_RIGHT,
  RENDER_ENEMY_LEFT,
  RENDER_ENEMY_UP,
  RENDER_ENEMY_DOWN,
};

typedef struct {
  u8 texture   : 1; // frames for everything that doesn't move, static assets.
  u8 explosive : 1; // explosion animations, coopts texture for explosion frames.
  u8 passable  : 1; // important subgroup.
  u8 breakable : 1;
  u8 lethal    : 1; // player will die if they occupy this space during the check
  i8 pickup    : 4; // positive / negative pickups
  // 3 bits left for extensions.
} tile_data_t;

static const tile_data_t impassable_tile = (tile_data_t) {.passable = 0};
static const tile_data_t impassable_wall = (tile_data_t) {.passable = 0, .texture = 1};
static const tile_data_t breakable_wall = (tile_data_t) {.passable = 0, .texture = 1, .breakable = 1};
static const tile_data_t passable_tile = (tile_data_t) {.passable = 1};
static const tile_data_t explosive_tile = (tile_data_t) {.passable = 1, .explosive = 1, .lethal = 1, .texture = 0};


typedef struct {
  tile_data_t state[TILE_LENGTH_LIMIT][TILE_LENGTH_LIMIT] aligned;
  u8 color aligned;
} tiles_t;

#define PLAYER_LIMIT (1<<2)

enum {
  RIGHT, LEFT, UP, DOWN
};

typedef struct {
  u8 bomb_limit : 4;
  u8 bomb_count : 4;
  u8 power  : 4; // < MAX(TILE_WIDTH, TILE_HEIGHT)
  u8 speed  : 4; // travels n units per second
  u8 pierce : 1;
  u8 kick   : 1; // no intent to implement
  u8 throw  : 1; // no intent to implement
  u8 bounce : 1; // no intent to implement
  u8 alive  : 1;
  u8 direction : 2; // right left up down
  u8 moving : 1;
  // 10 bits left for extensions.
} player_data_t;

typedef struct {
  i16 x[PLAYER_LIMIT] aligned;
  i16 y[PLAYER_LIMIT] aligned;
  player_data_t state[PLAYER_LIMIT] aligned;
  u8 color[PLAYER_LIMIT] aligned;
} players_t;

#define BOMB_LIMIT (1<<4)

typedef struct {
      u8 power  : 4; // < MAX(TILE_WIDTH, TILE_HEIGHT)
      u8 pierce : 1;
      u8 bounce : 1; // no intent to implement
      // 10 bits left for extensions.
} bomb_data_t;

typedef struct {
  i16 x[PLAYER_LIMIT][BOMB_LIMIT] aligned;
  i16 y[PLAYER_LIMIT][BOMB_LIMIT] aligned;
  bomb_data_t state[PLAYER_LIMIT][BOMB_LIMIT] aligned;
  u16 timer[PLAYER_LIMIT][BOMB_LIMIT] aligned; // updates until explosion.
  u8 color[2] aligned;
} bombs_t;

#define ENEMY_LIMIT (1<<4)

enum {
  MOVEMENT_VERTICAL,
  MOVEMENT_HORIZONTAL,
  MOVEMENT_RANDOM,
  MOVEMENT_LAST,
};

typedef struct {
  i16 x[ENEMY_LIMIT] aligned;
  i16 y[ENEMY_LIMIT] aligned;
  u8 movement[ENEMY_LIMIT] aligned;
} enemies_t;

#define CONFIG_STRING_LIMIT 128
typedef struct {
  u16 resolution_x aligned;
  u16 resolution_y aligned;
  u8 fps aligned;
  u8 ups aligned;
  char font[CONFIG_STRING_LIMIT] aligned;
  char spritesheet[CONFIG_STRING_LIMIT] aligned;
  char window_name[CONFIG_STRING_LIMIT] aligned;
  u16 spritesheet_scale aligned;
  /* --- */
  u8 player_count aligned;
  u8 map_x aligned;
  u8 map_y aligned;
} config_t;

typedef struct {
  tiles_t tiles aligned;
  players_t players aligned;
  bombs_t bombs aligned;
  enemies_t enemies aligned;
  config_t config aligned;
  u16 time_limit aligned;
  u8 client aligned;

  Font font aligned;
  Rectangle atlas[4*6] aligned;
  Texture spritesheet aligned;
  Camera2D camera aligned;
} game_t;

/* game.c */

void GameInitialize(game_t * game);
void GameDeinitialize(game_t * game);
void GameStart(config_t config);
void GameResize(game_t * game);

/* gamemode.c */

void MultiPlayer(game_t * game);

/* update.c */

i16 Update(game_t * game, timespec_t now);

/* render.c */

void Render(game_t * game, f64 interpolation);

/* raylib.c */

void GuiLoadStyleDarkSimple(void);
Font DefaultFont(char * choice);
void RaylibInitialize(game_t * game);
void RaylibDeinitialize(void);

/* ... */

#endif /* EVERYTHING_ALWAYS_H_ */
