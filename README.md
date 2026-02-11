MonoBomberMan
=================================================

# IP-based Multiplayer Bomber Man

Setup is a trivial as compilation.

## Game Internals
We want the traditional experience with simple power-ups and benign enemies.

Design-wise we'll keep things centralized and assume that this is a unified highly particular program - as any game is.

#### Structures

We'll start with the fundamental corpus:

	#define EXPECTED_UPS 30 // shouldn't matter at low values.
	#define EXPECTED_FPS 60 // or higher

    typedef struct {
      tiles_t tiles;     // tiles classifying texturing, accessibility, and lethality.
                         // Any communication between entities is done here.
      players_t players; // Controlled-by-humans.
      bombs_t bombs;     // timed explosives.
      enemies_t enemies; // Simple walk-back-and-forth enemies.
    
      // Windowing / Game Loop
      Font font;                // the global font
      u16 horizontal, vertical; // highly mutable width & height of the current window
      u16 ups, fps;             // Individually tracked intervals
      // ...
    } game_t;
    
The naming here is to imply that these substructures clearly imply that they cover everything, rather than needing to be arrays.

    #define TILE_WIDTH 13
    #define TILE_HEIGHT 13
    #define TILE_LIMIT (TILE_WIDTH * TILE_HEIGHT)
    
    enum {
      POWERUP_NONE = 0,
      POWERUP_BOMB,
      POWERUP_POWER,
      POWERUP_SPEED,
      // These will probably never be negative:
      POWERUP_PIERCE,
      POWERUP_KICK,
      POWERUP_THROW,
      POWERUP_BOUNCE,
      // Curse is something I've slightly read up on, but it seems to be timer based.
      // POWERUP_CURSE, // no plan to implement.
    };
    
    typedef struct {
      union {
        u16 _;
        struct {
          u8 texture   : 3; // frames for everything that doesn't move, static assets.
          u8 explosive : 1; // explosion animations, coopts texture for explosion frames.
          u8 passable  : 1;
          u8 breakable : 1;
          u8 lethal    : 1; // player will die if they occupy this space during the check
          i8 pickup    : 4; // positive / negative pickups
          // 5 bits left for extensions.
        };
      } state[TILE_LIMIT];
      u8 color[TILE_LIMIT];
    } tiles_t;

We will track fundamental interactions here. Powerups are grouped in per their simplicity.

    #define PLAYER_LIMIT (1<<2)
    
    typedef struct {
      // for smooth movement, rounding is used for tile checks.
      f32 x[PLAYER_LIMIT], y[PLAYER_LIMIT];
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
          // 12 bits left for extensions.
        };
      } state[PLAYER_LIMIT];
      u8 color[PLAYER_LIMIT];
    } players_t;

The player will donate state to each bomb they place. The movement is meant to match the grid as best as possible and should set to the nearest whole number.
One update means one tile of movement, so we can reasonably set the updates per second in relation to that.
The framerate being a higher value should result in a visually consistent style.

    #define BOMB_LIMIT (PLAYER_LIMIT*(1<<4))
    
    typedef struct {
      f32 x[BOMB_LIMIT], y[BOMB_LIMIT];
      union {
        u16 _;
        struct {
          u8 power  : 4; // < MAX(TILE_WIDTH, TILE_HEIGHT)
          u8 pierce : 1;
          u8 bounce : 1;
          // 10 bits left for extensions.
        };
      } state[BOMB_LIMIT];
      u16 timer[BOMB_LIMIT]; // updates until explosion.
      u8 color[BOMB_LIMIT], color_flash[BOMB_LIMIT];
    } bombs_t;

Bombs are pretty simple. After they explode they lag behind.
The actual explosion is timed statically, and each animation frame is a certain time.
We can assume frames are interpolated and UPS is low.

    #define ENEMY_LIMIT (1<<4)
    
    enum {
      MOVEMENT_VERTICAL,
      MOVEMENT_HORIZONTAL,
      MOVEMENT_RANDOM,
    };
    
    typedef struct {
      f32 x[ENEMY_LIMIT], y[ENEMY_LIMIT];
      u8 movement[ENEMY_LIMIT];
    } enemies_t;

Enemies are primitive. These set the tile they are on as lethal.
We can assume they have the same base movement speed.

#### Loading

We want to load a bunch of information at the start and it's trivial with or without Raylib.

The important directories/files are:
assets/(spritesheet).png
fonts/(fontset)/
configuration.txt <- omitted for simplicity.

We'll look for the assets for tiles, bombs, explosion frames, & players.

#### Networking

## Copyright
[monobomberman](https://github.com/8e8m/monobomberman) by [Anon Anonison](https://github.com/agvxov) & [Emil Williams](https://github.com/8e8m) is marked [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

![(CC)](https://mirrors.creativecommons.org/presskit/icons/cc.svg)
![(0)](https://mirrors.creativecommons.org/presskit/icons/zero.svg)

## Contact
Any inqueries may be directed to [Emil](mailto:emil@chud.cyou).
