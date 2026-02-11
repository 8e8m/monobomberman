#include "all.h"

void GameInitialize(game_t * game, char * window_name) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_NONE);
  /* :config */
  game->horizontal = 1920;
  game->vertical = 1080;
  InitWindow(game->horizontal, game->vertical, window_name);
  game->ups = 60;
  game->fps = 30;
  game->font = DefaultFont("fonts/Atkinson/mono/AtkinsonHyperlegibleMono-Bold.otf");
  /* :setup */
  SetWindowState(FLAG_WINDOW_HIDDEN);
  InitAudioDevice();
  SetWindowPosition(0, 0);
  GuiLoadStyleDarkSimple();
  GuiSetFont(game->font);
  /* --- */
}

void GameDeinitialize(game_t * game) {
  SetWindowState(FLAG_WINDOW_HIDDEN);
  UnloadFont(game->font);
  CloseAudioDevice();
  CloseWindow();
}

void GameLoop(game_t * game) {

  #define StepStart(prefix)                                     \
  prefix##_delta = timespec_sub(now, prefix##_last);            \
  if (timespec_cmp(prefix##_delta, prefix##_interval) >= 0) { (void) 0

  #define StepStop(prefix)                                              \
    prefix##s_per_second++;                                             \
    prefix##_total++;                                                   \
    prefix##_last = timespec_add(prefix##_last, prefix##_interval);     \
    if (timespec_cmp(prefix##_last, now) < 0) {                         \
      prefix##_last = now;                                              \
    }                                                                   \
  }

  #define StepSimpleStart(prefix,linear)                                \
    prefix##_delta = timespec_sub(now, prefix##_last);                  \
    if (timespec_cmp(prefix##_delta, (timespec_t){1.,0.}) >= 0) {  \
      (void)0

  #define StepSimpleStop(prefix)                        \
    prefix##_last = now;                                \
  }

  timespec_t
    now,
    update_interval = {0, (f64) TIMESPEC_HZ / game->ups},
    frame_interval = {0, (f64) TIMESPEC_HZ / game->fps},
    update_last, frame_last, print_last,
    update_delta, frame_delta, print_delta,
    wait;

  u16
    updates_per_second = 0, frames_per_second = 0;

  u32
    frame_total = 0, update_total = 0;

  f64 interpolation = 0.;

  ClearWindowState(FLAG_WINDOW_HIDDEN);

  clock_gettime(CLOCK_MONOTONIC, &now);
  update_last = frame_last = print_last = now;

  while (1) {
    StepStart(update);
    if (GameUpdate(game, now)) { return; }
    StepStop(update);

    StepStart(frame);
    clock_gettime(CLOCK_MONOTONIC, &now);
    interpolation =
      CLAMP(
	TIMESPEC_TO_F64(update_delta)
	/ TIMESPEC_TO_F64(update_interval),
	0.0, 1.0f);
    GameRender(game, interpolation);
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

    StepSimpleStart(print, one_second);
    GameReport(game,
	       round(frames_per_second / TIMESPEC_TO_F64(print_delta)),
	       round(updates_per_second / TIMESPEC_TO_F64(print_delta)),
	       frame_total,
	       update_total);
    frames_per_second = updates_per_second = 0;
    StepSimpleStop(print);

    clock_gettime(CLOCK_MONOTONIC, &now);
  }
}

i16 GameUpdate(game_t * game, timespec_t now) {
  (void) now;

  PollInputEvents();
  if (IsWindowResized()) {
    game->horizontal = GetScreenWidth();
    game->vertical = GetScreenHeight();
  }
  switch (GetKeyPressed()) {
  case KEY_ESCAPE: return 1;
  }
  return 0;
}

void GameRender(game_t * game, f64 interpolation) {
  (void)interpolation;

  BeginDrawing();
  ClearBackground(BLACK);
  /* >>> */
  
  /* --- */
  rlDrawRenderBatchActive();
  SwapScreenBuffer();
}

void GameReport(game_t * game, f32 fps, f32 ups, u32 total_fps, u32 total_ups) {
  printf("[FPS|UPS|Total] (%3.0f : %3.0f) | [%7u/%7u]\n",
	 fps, ups, total_fps, total_ups);
}
