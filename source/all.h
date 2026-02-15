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

/* Spritesheets will be (128*4x128*6)
   group 0 is   [0-1][0  ]     / un/breakable walls
   group 1 (just explosions, the nulls are skipped for sake of compactness)
           is   [2-3][0  ]    |
   group 2 is   [0-3][1  ]    | Negatives use color mask
                [0-2][2  ]    /
   group 3 is   [0-3][3  ]   / player
   group 4 is   [0-3][4  ]  / bomb
   group 5 is   [0-3][4  ] / enemy

 */

enum {
  /* each group is for its own group of textures,
     with at most 8 textures being possible within a functional group.
     we assume LSB per x86-64 ABI,
     this would need to be redefined / bit field part removed */
  EXPLOSIVE                     = (1 << 3),
  PASSIBLE                      = (1 << 4),
  BREAKABLE                     = (1 << 5),
  LETHAL                        = (1 << 6),
  POWERUP                       = (1 << 7),
  /* lethality / breakable / explosive are not specially grouped. */

  /* group 0 "impassible" set */
  IMPASSIBLE_WALL               = 0,
  IMPASSIBLE_BREAKABLE_WALL     = 1,
  IMPASSIBLE_NOTHING            = 2,

  /* group 1 "passible" set */
  PASSIBLE_NOTHING              = PASSIBLE | 0,
  PASSIBLE_NOTHING_LETHAL       = PASSIBLE | LETHAL | 1,
  PASSIBLE_EXPLOSIVE_LETHAL     = PASSIBLE | LETHAL | EXPLOSIVE | 0,
  PASSIBLE_EXPLOSIVE_LETHAL_END = PASSIBLE | LETHAL | EXPLOSIVE | 1,

  /* group 2 "pickup" set */
  POWERUP_BOMB                  = PASSIBLE | POWERUP | 0,
  POWERUP_POWER                 = PASSIBLE | POWERUP | 1,
  POWERUP_SPEED                 = PASSIBLE | POWERUP | 2,
  // These will probably never be negative:
  POWERUP_PIERCE                = PASSIBLE | POWERUP | 3,
  POWERUP_KICK                  = PASSIBLE | POWERUP | 4,
  POWERUP_THROW                 = PASSIBLE | POWERUP | 5,
  POWERUP_BOUNCE                = PASSIBLE | POWERUP | 6,

  /* group 3 and higher is not directly classified by the tile system */

};

#define TILE_LENGTH_LIMIT 21

typedef struct {
  union {
    u16 _;
    struct {
      u8 texture   : 3; // frames for everything that doesn't move, static assets.
      u8 explosive : 1; // explosion animations, coopts texture for explosion frames.
      u8 passable  : 1; // important subgroup.
      u8 breakable : 1;
      u8 lethal    : 1; // player will die if they occupy this space during the check
      i8 pickup    : 4; // positive / negative pickups
      // 5 bits left for extensions.
    };
  } state[TILE_LENGTH_LIMIT][TILE_LENGTH_LIMIT] aligned;
  u8 color aligned;
  Rectangle wall[2] aligned;
  Rectangle explosion[2] aligned;
  Rectangle powerup[8] aligned;
} tiles_t;

#define PLAYER_LIMIT (1<<2)

enum {
  RIGHT, LEFT, UP, DOWN
};
    
typedef struct {
  i16 x[PLAYER_LIMIT] aligned;
  i16 y[PLAYER_LIMIT] aligned;
  f32 animation_x[PLAYER_LIMIT] aligned;
  f32 animation_y[PLAYER_LIMIT] aligned;
  union {
    u32 _;
    struct {
      u8 bomb_limit : 4;
      u8 bomb_count : 4;
      u8 power  : 4; // < MAX(TILE_WIDTH, TILE_HEIGHT)
      u8 speed  : 4; // travels n units per second
      u8 pierce : 1;
      u8 kick   : 1;
      u8 throw  : 1; // no intent to implement
      u8 bounce : 1;
      u8 alive  : 1;
      u8 direction : 2; // right left up down
      u8 moving : 1;
      // 10 bits left for extensions.
    };
  } state[PLAYER_LIMIT] aligned;
  u8 color[PLAYER_LIMIT] aligned;
  Rectangle player[4] aligned;
} players_t;

#define BOMB_LIMIT (1<<4)
    
typedef struct {
  i16 x[PLAYER_LIMIT][BOMB_LIMIT] aligned;
  i16 y[PLAYER_LIMIT][BOMB_LIMIT] aligned;
  union {
    u16 _;
    struct {
      u8 power  : 4; // < MAX(TILE_WIDTH, TILE_HEIGHT)
      u8 pierce : 1;
      u8 bounce : 1;
      // 10 bits left for extensions.
    };
  } state[PLAYER_LIMIT][BOMB_LIMIT] aligned;
  u16 timer[PLAYER_LIMIT][BOMB_LIMIT] aligned; // updates until explosion.
  u8 color[2] aligned;
  Rectangle bomb[4] aligned;
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
  Rectangle enemy[4] aligned;
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

  Font font aligned;

  Texture spritesheet aligned;
  Camera2D camera aligned;
  u16 time_limit aligned;
  u8 client aligned;
} game_t;

/* game.c */

void GameStart(config_t config);
void GameResize(game_t * game);
void GameReinitialize(game_t * game);

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
