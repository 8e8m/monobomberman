#include "all.h"

int Main(int count, char ** arguments)
{
  (void)count;
  char * window_name = arguments[0];
  config_t config = (config_t){0};
  char * p = strchr(window_name, '/');
  strlcpy(config.window_name, p ? p+1 : window_name, CONFIG_STRING_LIMIT);
  srand(time(NULL));
  Root(window_name);
  GameStart(config);
  return 0;
}

int main (int count, char ** arguments) __attribute__((alias("Main")));
