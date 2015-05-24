#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"
#include "wren.h"

#define MAX_LINE_LENGTH 1024 // TODO: Something less arbitrary.

static int runRepl()
{
  WrenVM* vm = createVM(NULL);

  printf("\\\\/\"-\n");
  printf(" \\_/   wren v0.0.0\n");

  char line[MAX_LINE_LENGTH];

  for (;;)
  {
    printf("> ");

    if (!fgets(line, MAX_LINE_LENGTH, stdin))
    {
      printf("\n");
      break;
    }

    // TODO: Handle failure.
    wrenInterpret(vm, "Prompt", line);

    // TODO: Automatically print the result of expressions.
  }

  wrenFreeVM(vm);
  return 0;
}

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
    runFile(NULL, argv[1]);
  }

  return 0;
}
