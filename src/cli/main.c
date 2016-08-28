#include <stdio.h>
#include <string.h>

#include "os.h"
#include "vm.h"
#include "wren.h"

int main(int argc, const char* argv[])
{
  if (argc == 2 && strcmp(argv[1], "--help") == 0)
  {
    printf("Usage: wren [file] [arguments...]\n");
    printf("  --help  Show command line usage\n");
    return 0;
  }
  
  if (argc == 2 && strcmp(argv[1], "--version") == 0)
  {
    printf("wren %s\n", WREN_VERSION_STRING);
    return 0;
  }
  
  osSetArguments(argc, argv);

  if (argc == 1)
  {
    runRepl();
  }
  else
  {
    runFile(argv[1]);
  }

  return 0;
}
