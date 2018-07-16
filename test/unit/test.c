#include <stdio.h>

#include "test.h"

int passes = 0;
int failures = 0;

void pass()
{
  passes++;
}

void fail()
{
  failures++;
}

int showTestResults()
{
  if (failures > 0)
  {
    printf("%d out of %d tests failed. :(\n", failures, passes + failures);
    return 1;
  }
  
  printf("All %d tests passed!\n", passes + failures);
  return 0;
}

