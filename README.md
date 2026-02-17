MonoBomberMan
=================================================

# IP-based Multiplayer Bomber Man

Setup is a trivial as compilation.

## Game Internals
We want the traditional experience with simple power-ups and benign enemies.

Design-wise we'll keep things centralized and assume that this is a unified highly particular program - as any game is.

We do not just allocate things, we designate them. Runtime Allocation is for losers.
Hardcoding is our one true design goal. Knowing exactly what we want, rather than trying to cover a hypothetical's hypothetical potdream.

#### Structures

Our Corpus:

    typedef struct {
      tiles_t tiles;
      players_t players;
      bombs_t bombs;
      enemies_t enemies;
      config_t config;
      u16 time_limit;
      u8 client;
    
      Font font;
      Rectangle atlas[4*6];
      Texture spritesheet;
      Camera2D camera;
    } game_t;

Everything near the bottom is for rendering / Raylib wrangling, and does not really correspond to the actual important data.

Our Configuration detail:

    #define CONFIG_STRING_LIMIT 128
    typedef struct {
      u16 resolution_x;
      u16 resolution_y;
      u8 fps;
      u8 ups;
      char font[CONFIG_STRING_LIMIT];
      char spritesheet[CONFIG_STRING_LIMIT];
      char window_name[CONFIG_STRING_LIMIT];
      u16 spritesheet_scale;
      /* --- */
      u8 player_count;
      u8 map_x;
      u8 map_y;
    } config_t;

These configuration details are parameterized in GameInitialize in game.c:

    #define DEFAULT(a, b) ((b) ? (b) : (a))
      game->config.resolution_x = MAX(200, DEFAULT(600, game->config.resolution_x));
      game->config.resolution_y = game->config.resolution_x+ FONT_SIZE;
      game->config.fps = MAX(1, DEFAULT(60, game->config.fps));
      game->config.ups = MAX(1, DEFAULT(30, game->config.ups));
      if (!*game->config.font)
      { strlcpy(game->config.font, "fonts/Atkinson/mono/AtkinsonHyperlegibleMono-Bold.otf", CONFIG_STRING_LIMIT); }
      if (!*game->config.spritesheet)
      { strlcpy(game->config.spritesheet, "assets/simple.png", CONFIG_STRING_LIMIT); }
      game->config.spritesheet_scale = DEFAULT(128, game->config.spritesheet_scale);
      game->config.map_x = MAX(5, DEFAULT(13, game->config.map_x));
      game->config.map_y = MAX(5, DEFAULT(13, game->config.map_y));
      game->config.player_count = CLAMP(DEFAULT(4, game->config.player_count), 1, 4);
      if (!*game->config.window_name)
      { strlcpy(game->config.window_name, "Unset Window Name, lol lmao", CONFIG_STRING_LIMIT); }
    #undef DEFAULT
 
This ensures a rational set of possible values allowing for finite flexibility in our finite memory space.

    tiles_t tiles;
    players_t players;
    bombs_t bombs;
    enemies_t enemies;

The above should be thought of as _subgroups_, instead of as structures. These structures do not really exist in terms of the program.

We'll start from the top, being the most verbose and critical:

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

The above serves as our direct mapping onto our Atlas. This is not very useful past being a set of useful offsets and laying out the data. While we could forgo the following and simply have a 5 bit texture, instead this explicit bitfielding is easier:

    typedef struct {
      u8 texture   : 1; // frames for everything that doesn't move, static assets.
      u8 explosive : 1; // explosion animations, coopts texture for explosion frames.
      u8 passable  : 1; // important subgroup.
      u8 breakable : 1;
      u8 lethal    : 1; // player will die if they occupy this space during the check
      i8 pickup    : 4; // positive / negative pickups
      // 3 bits left for extensions.
    } tile_data_t;

This covers edge cases like enemies which emit damage and directly encode powerups.

    static const tile_data_t impassable_tile = (tile_data_t) {.passable = 0};
    static const tile_data_t impassable_wall = (tile_data_t) {.passable = 0, .texture = 1};
    static const tile_data_t breakable_wall = (tile_data_t) {.passable = 0, .texture = 1, .breakable = 1};
    static const tile_data_t passable_tile = (tile_data_t) {.passable = 1};
    static const tile_data_t explosive_tile = (tile_data_t) {.passable = 1, .explosive = 1, .lethal = 1, .texture = 0};
    
This is a not very useful list that provides short presents. This, in my mind, is the only valid use of const.

    typedef struct {
      tile_data_t state[TILE_LENGTH_LIMIT][TILE_LENGTH_LIMIT];
      u8 color;
    } tiles_t;
	
Our tile system is very simple. The color is seeded at Init-time.

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
	
Need I say more?

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

Bombs behave very primitively, inheriting everything and being excluded in every regard from everything else. This is so that they're isolated for further improvements, even though that'll most likely never occur. 

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

Enemies are unimplemented and trivial, walking in a certain pattern in a cleared area.

#### Loading

Loading is trivial, we want a single font and a single png specified at runtime.
this is specified in game->conf.

#### Networking

## Copyright
[monobomberman](https://github.com/8e8m/monobomberman) by [Anon Anonison](https://github.com/agvxov) & [Emil Williams](https://github.com/8e8m) is marked [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

![(CC)](https://mirrors.creativecommons.org/presskit/icons/cc.svg)
![(0)](https://mirrors.creativecommons.org/presskit/icons/zero.svg)

## Contact
Any inqueries may be directed to [Emil](mailto:emil@chud.cyou).
