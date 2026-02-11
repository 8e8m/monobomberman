#include "all.h"

int Main(int count, char ** arguments)
{
  (void)count;
  char * program_name = arguments[0];
  srand(time(NULL));
  Root(program_name);
  GameStart(program_name);
  return 0;
}

int main (int count, char ** arguments) __attribute__((alias("Main")));
