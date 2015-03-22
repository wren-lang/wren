#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"
#include "wren.h"

#define MAX_LINE_LENGTH 1024 // TODO: Something less arbitrary.

static int runRepl()
{
  WrenVM* vm = createVM();

  printf("\\\\/\"-\n");
  printf(" \\_/   wren v0.0.0\n");

  char line[MAX_LINE_LENGTH];

  for (;;)
  {
    printf("> ");

    if (fgets(line, MAX_LINE_LENGTH, stdin))
    {
      // TODO: Handle failure.
      WrenInterpretResult interpretResult = wrenInterpret(vm, "Prompt", line);

      // TODO: Automatically print the result of expressions.
      printf("=> %s\n", interpretResult.value);
    }
    else
    {
      printf("\n");
      return 0;
    }
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
    runFile(argv[1]);
  }

  return 0;
}
