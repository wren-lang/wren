#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "test.h"

static void expectNormalize(const char* input, const char* expected)
{
  Path* path = pathNew(input);
  pathNormalize(path);
  
  if (strcmp(path->chars, expected) != 0)
  {
    printf("FAIL %-30s Want %s\n", input, expected);
    printf("                                     Got %s\n\n", path->chars);
    fail();
  }
  else
  {
#if SHOW_PASSES
    printf("PASS %-30s   -> %s\n", input, path->chars);
#endif
    pass();
  }
  
  pathFree(path);
}

static void testNormalize()
{
  // Simple cases.
  expectNormalize("", ".");
  expectNormalize(".", ".");
  expectNormalize("..", "..");
  expectNormalize("a", "a");
  expectNormalize("/", "/");
  
  // Collapses redundant separators.
  expectNormalize("a/b/c", "a/b/c");
  expectNormalize("a//b///c////d", "a/b/c/d");
  
  // Eliminates "." parts, except one at the beginning.
  expectNormalize("./", ".");
  expectNormalize("/.", "/");
  expectNormalize("/./", "/");
  expectNormalize("./.", ".");
  expectNormalize("a/./b", "a/b");
  expectNormalize("a/.b/c", "a/.b/c");
  expectNormalize("a/././b/./c", "a/b/c");
  expectNormalize("././a", "./a");
  expectNormalize("a/./.", "a");
  
  // Eliminates ".." parts.
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
  
  // Does not walk before root on absolute paths.
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
  
  // Removes trailing separators.
  expectNormalize("./", ".");
  expectNormalize(".//", ".");
  expectNormalize("a/", "a");
  expectNormalize("a/b/", "a/b");
  expectNormalize("a/b///", "a/b");
  
  expectNormalize("foo/bar/baz", "foo/bar/baz");
  expectNormalize("foo", "foo");
  expectNormalize("foo/bar/", "foo/bar");
  expectNormalize("./foo/././bar/././", "./foo/bar");
}

void testPath()
{
  // TODO: Test other functions.
  testNormalize();
}

