#include "all.h"

int Main(int count, char ** arguments)
{
  (void)count;
  _Alignas(64) game_t game[1] = {0};
  char * program_name = arguments[0];
  srand(time(NULL));
  Root(program_name);
  GameInitialize(game, program_name);
  GameLoop(game);
  GameDeinitialize(game);
  return 0;
}

int main (int count, char ** arguments) __attribute__((alias("Main")));
