#include "all.h"

static void GameInitialize(game_t * game);
static void GameDeinitialize(game_t * game);
static void GameLoop(game_t * game);
static void GameReport(game_t * game, f32 fps, f32 ups, u32 total_fps, u32 total_ups);

void GameReinitialize(game_t * game) {
  GameDeinitialize(game);
  GameInitialize(game);
}

void GameStart(config_t config) {
  _Alignas(64) game_t game[1] = {0};
  game->config = config;
  GameInitialize(game);
  GameLoop(game);
  GameDeinitialize(game);
}

static void GameRecalculateViewport(game_t * game) {
  game->config.resolution_x = GetScreenWidth();
  game->config.resolution_y = GetScreenHeight();
  game->camera = (Camera2D) {
    .offset = (Vector2) { 0 },
    .target = (Vector2) { 0 },
    .rotation = 0.,
    .zoom = fminf(
      (float) game->config.resolution_x /
      (game->config.map_x * game->config.spritesheet_scale),
      (float) game->config.resolution_y /
      (game->config.map_y * game->config.spritesheet_scale)),
  };
}

void GameResize(game_t * game) {
  if (IsWindowResized()) {
    GameRecalculateViewport(game);
  }
}

static void GameInitialize(game_t * game) {
  /* Strict parameters */
#define DEFAULT(a, b) ((b) ? (b) : (a))
  game->config.resolution_x = MAX(200, DEFAULT(600, game->config.resolution_x));
  game->config.resolution_y = MAX(200, DEFAULT(600, game->config.resolution_y));
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

  {
    int t = game->config.spritesheet_scale;
    /* better, but not really good, it's FINE */
    Rectangle wall[2] = // group 0
      {(Rectangle){    0,  0, t, t},
       (Rectangle){t - 1,  0, t, t}};

    Rectangle explosion[2] = // group 1
      {(Rectangle){t * 2 - 1,  0, t, t},
       (Rectangle){t * 3 - 1,  0, t, t}};

    Rectangle powerup[8] = // group 2
      {(Rectangle){        0,  t, t, t},
       (Rectangle){t     - 1,  t, t, t},
       (Rectangle){t * 2 - 1,  t, t, t},
       (Rectangle){t * 3 - 1,  t, t, t},
       (Rectangle){        0,  t * 2, t, t},
       (Rectangle){t     - 1,  t * 2, t, t},
       (Rectangle){t * 2 - 1,  t * 2, t, t},
       (Rectangle){t * 3 - 1,  t * 2, t, t}};

    Rectangle player[4] = // group 3
      {(Rectangle){        0,  t * 3, t, t},
       (Rectangle){t     - 1,  t * 3, t, t},
       (Rectangle){t * 2 - 1,  t * 3, t, t},
       (Rectangle){t * 3 - 1,  t * 3, t, t}};

    Rectangle bomb[4] = // group 4
      {(Rectangle){        0,  t * 4, t, t},
       (Rectangle){t     - 1,  t * 4, t, t},
       (Rectangle){t * 2 - 1,  t * 4, t, t},
       (Rectangle){t * 3 - 1,  t * 4, t, t}};

    Rectangle enemy[4] = // group 5
      {(Rectangle){        0,  t * 5, t, t},
       (Rectangle){t     - 1,  t * 5, t, t},
       (Rectangle){t * 2 - 1,  t * 5, t, t},
       (Rectangle){t * 3 - 1,  t * 5, t, t}};

    memcpy(game->tiles.wall, wall, sizeof(wall));
    memcpy(game->tiles.explosion, explosion, sizeof(explosion));
    memcpy(game->tiles.powerup, powerup, sizeof(powerup));
    memcpy(game->players.player, player, sizeof(player));
    memcpy(game->bombs.bomb, bomb, sizeof(bomb));
    memcpy(game->enemies.enemy, enemy, sizeof(enemy));
  }

  MultiPlayer(game);

  game->tiles.color = (rand() % 4) | ((rand() % 4) << 2) | ((rand() % 4) << 4) | GAME_OPAQUE;
  if (game->tiles.color == GAME_OPAQUE) { game->tiles.color |= GAME_WHITE; }

  game->bombs.color[0] = GAME_WHITE | GAME_OPAQUE;
  game->bombs.color[1] = GAME_RED | GAME_OPAQUE;

  /* :config */
  game->font = DefaultFont(game->config.font);

  RaylibInitialize(game->config.resolution_x, game->config.resolution_y, game->config.window_name, game->font);
  GameRecalculateViewport(game);

  game->spritesheet = LoadTexture(game->config.spritesheet);
  if (game->spritesheet.id <= 0) { abort(); }

  ClearWindowState(FLAG_WINDOW_HIDDEN);
}

static void GameDeinitialize(game_t * game) {
  UnloadTexture(game->spritesheet);
  if (GetFontDefault().texture.id != game->font.texture.id) { UnloadFont(game->font); }
  RaylibDeinitialize();
}

static void GameLoop(game_t * game) {

  timespec_t
    now,
    update_interval = {0, (f64) TIMESPEC_HZ / game->config.ups},
    frame_interval = {0, (f64) TIMESPEC_HZ / game->config.fps},
    print_interval = {0, (f64) TIMESPEC_HZ },
    update_last, frame_last, print_last,
    update_delta, frame_delta, print_delta,
    wait;

  u16
    updates_per_second = 0, frames_per_second = 0;

  u32
    frame_total = 0, update_total = 0;

  f64 interpolation = 0.;

  clock_gettime(CLOCK_MONOTONIC, &now);
  update_last = frame_last = print_last = now;

  while (1) {
    StepStart(update);
    updates_per_second++;
    update_total++;
    if (Update(game, now)) { return; }
    StepStop(update);

    StepStart(frame);
    frames_per_second++;
    frame_total++;
    clock_gettime(CLOCK_MONOTONIC, &now);
    interpolation =
      CLAMP(
        TIMESPEC_TO_F64(update_delta)
        / TIMESPEC_TO_F64(update_interval),
        0.0, 1.0f);
    Render(game, interpolation);
    StepStop(frame);

    clock_gettime(CLOCK_MONOTONIC, &now);

    wait = timespec_sub(
      timespec_min(
        timespec_add(update_last, update_interval),
        timespec_add(frame_last, frame_interval)),
      now);

    if (timespec_cmp(wait, zero_seconds) > 0) {
      nanosleep(&wait, NULL);
    }

    StepStart(print);
    GameReport(game,
	   round(frames_per_second / TIMESPEC_TO_F64(print_delta)),
	   round(updates_per_second / TIMESPEC_TO_F64(print_delta)),
	   frame_total,
	   update_total);
    frames_per_second = updates_per_second = 0;
    StepStop(print);

    clock_gettime(CLOCK_MONOTONIC, &now);
  }
}

static void GameReport(game_t * game, f32 fps, f32 ups, u32 total_fps, u32 total_ups) {
  (void)game;
#ifndef NDEBUG
  printf("[FPS|UPS|Total] (%3.0f : %3.0f) | [%7u/%7u]\n",
         fps, ups, total_fps, total_ups);
#endif
}
