#include "inotify.h"

int
read_int(char * path)
{
  FILE * f;
  int i;
  f = fopen(path, "rb");
  if (f == NULL)
  {
    die("error: unable to open \"%s\"", path);
  }
  fscanf(f, "%d", &i);
  fclose(f);
  return i;
}
