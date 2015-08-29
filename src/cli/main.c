#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"
#include "wren.h"

int main(int argc, const char* argv[])
{
  if (argc < 1 || argc > 2)
  {
    fprintf(stderr, "Usage: wren [file]\n");
    return 64; // EX_USAGE.
  }

  if (argc == 1)
  {
    runRepl();
  }
  else if (argc == 2)
  {
    runFile(argv[1]);
  }

  return 0;
}
