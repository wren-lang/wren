#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "test.h"

static void expectNormalize(const char* input, const char* expected)
{
  Path* path = pathNew(input);
  Path* result = pathNormalize(path);
  
  if (strcmp(result->chars, expected) != 0)
  {
    printf("FAIL %-30s Want %s\n", input, expected);
    printf("                                     Got %s\n\n", result->chars);
    fail();
  }
  else
  {
#if SHOW_PASSES
    printf("PASS %-30s   -> %s\n", input, result->chars);
#endif
    pass();
  }
  
  pathFree(path);
  pathFree(result);
}

static void testNormalize()
{
  // simple cases
  expectNormalize("", ".");
  expectNormalize(".", ".");
  expectNormalize("..", "..");
  expectNormalize("a", "a");
  expectNormalize("/", "/");
  
  // collapses redundant separators
  expectNormalize("a/b/c", "a/b/c");
  expectNormalize("a//b///c////d", "a/b/c/d");
  
  // eliminates "." parts
  expectNormalize("./", ".");
  expectNormalize("/.", "/");
  expectNormalize("/./", "/");
  expectNormalize("./.", ".");
  expectNormalize("a/./b", "a/b");
  expectNormalize("a/.b/c", "a/.b/c");
  expectNormalize("a/././b/./c", "a/b/c");
  expectNormalize("././a", "a");
  expectNormalize("a/./.", "a");
  
  // eliminates ".." parts
  expectNormalize("..", "..");
  expectNormalize("../", "..");
  expectNormalize("../../..", "../../..");
  expectNormalize("../../../", "../../..");
  expectNormalize("/..", "/");
  expectNormalize("/../../..", "/");
  expectNormalize("/../../../a", "/a");
  expectNormalize("a/..", ".");
  expectNormalize("a/b/..", "a");
  expectNormalize("a/../b", "b");
  expectNormalize("a/./../b", "b");
  expectNormalize("a/b/c/../../d/e/..", "a/d");
  expectNormalize("a/b/../../../../c", "../../c");
  
  // does not walk before root on absolute paths
  expectNormalize("..", "..");
  expectNormalize("../", "..");
  expectNormalize("/..", "/");
  expectNormalize("a/..", ".");
  expectNormalize("../a", "../a");
  expectNormalize("/../a", "/a");
  expectNormalize("/../a", "/a");
  expectNormalize("a/b/..", "a");
  expectNormalize("../a/b/..", "../a");
  expectNormalize("a/../b", "b");
  expectNormalize("a/./../b", "b");
  expectNormalize("a/b/c/../../d/e/..", "a/d");
  expectNormalize("a/b/../../../../c", "../../c");
  expectNormalize("a/b/c/../../..d/./.e/f././", "a/..d/.e/f.");
  
  // removes trailing separators
  expectNormalize("./", ".");
  expectNormalize(".//", ".");
  expectNormalize("a/", "a");
  expectNormalize("a/b/", "a/b");
  expectNormalize("a/b///", "a/b");
  
  expectNormalize("foo/bar/baz", "foo/bar/baz");
  expectNormalize("foo", "foo");
  expectNormalize("foo/bar/", "foo/bar");
  expectNormalize("./foo/././bar/././", "foo/bar");
}

void testPath()
{
  // TODO: Test other functions.
  testNormalize();
}

