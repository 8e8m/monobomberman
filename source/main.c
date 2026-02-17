#include "all.h"
#include <options.h>

#define arg if (++arguments, !--count) { goto help; } else
config_t Arguments(int count, char ** arguments) {
  config_t config = {0};
  while (++arguments, --count > 0) {
    while (**arguments == '-') { ++*arguments; }
    struct options * option = options_lookup(*arguments,strlen(*arguments));
    if (!option) {
      printf("Unknown option '%s', try 'help'\n", *arguments);
      goto abort;
    }
    switch (option->number) {
    case OPTION_HELP: {
      help:
      printf("help/h/?          --            -- This.\n"
             "resolution        -- <x>        -- The on-display resolution squared\n"
             "fps               -- <x>        -- Framerate\n"
             "ups               -- <x>        -- Updates\n"
             "font              -- </path/to> -- Glorious Font\n"
             "spritesheet       -- </path/to> -- The spritesheet for displaying all of reality\n"
             "spritesheet_scale -- <x>        -- The square scale of the above\n"
             "players           -- <x>        -- N players\n"
             "map/map_size      -- <x y>      -- the in-game map size\n"
        );
      exit(0); }
    case OPTION_RESOLUTION: {
      arg { config.resolution_x = atoi(*arguments); }
      break; }
    case OPTION_FPS: {
      arg { config.fps = atoi(*arguments); }
      break; }
    case OPTION_UPS: {
      arg { config.ups = atoi(*arguments); }
      break; }
    case OPTION_PLAYER_COUNT: {
      arg { config.player_count = atoi(*arguments); }
      break; }
    case OPTION_MAP_SIZE: {
      arg { config.map_x = atoi(*arguments); }
      arg { config.map_y = atoi(*arguments); }
      break; }
    case OPTION_SPRITESHEET: {
      arg { strlcpy(config.spritesheet, *arguments, CONFIG_STRING_LIMIT); }
      break; }
    case OPTION_SPRITESHEET_SCALE: {
      arg { config.spritesheet_scale = atoi(*arguments); }
      break; }
    case OPTION_FONT: {
      arg { strlcpy(config.font, *arguments, CONFIG_STRING_LIMIT); }
      break; }
    }
  }
  return config;
abort:
  exit(1);
}

int Main(int count, char ** arguments)
{
  char * window_name = arguments[0];
  config_t config = Arguments(count, arguments);
  char * p = strchr(window_name, '/');
  strlcpy(config.window_name, p ? p+1 : window_name, CONFIG_STRING_LIMIT);
  srand(time(NULL));
  Root(window_name);
  GameStart(config);
  return 0;
}

int main (int count, char ** arguments) __attribute__((alias("Main")));
