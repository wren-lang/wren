#include "wren_common.h"
#include "wren_math.h"

#if WREN_USE_LIB_MATH

#include <math.h>
#include <stdint.h>
#include <time.h>

static int32_t _seed = 0x5752454e; // WREN :)

static void mathAbs(WrenVM* vm)
{
  double value = wrenGetArgumentDouble(vm, 1);
  double result = fabs(value);
  wrenReturnDouble(vm, result);
}

static void mathCeil(WrenVM* vm)
{
  double value = wrenGetArgumentDouble(vm, 1);
  double result = ceil(value);
  wrenReturnDouble(vm, result);
}

static void mathFloor(WrenVM* vm)
{
  double value = wrenGetArgumentDouble(vm, 1);
  double result = floor(value);
  wrenReturnDouble(vm, result);
}

static void mathInt(WrenVM* vm)
{
  double value = wrenGetArgumentDouble(vm, 1);
  double integer;
  double fractional = modf(value, &integer);
  wrenReturnDouble(vm, integer);
}

static void mathFrac(WrenVM* vm)
{
  double value = wrenGetArgumentDouble(vm, 1);
  double integer;
  double fractional = modf(value, &integer);
  wrenReturnDouble(vm, fractional);
}

static void mathSin(WrenVM* vm)
{
  double angle = wrenGetArgumentDouble(vm, 1);
  double sine = sin(angle);
  wrenReturnDouble(vm, sine);
}

static void mathCos(WrenVM* vm)
{
  double angle = wrenGetArgumentDouble(vm, 1);
  double cosine = cos(angle);
  wrenReturnDouble(vm, cosine);
}

static void mathTan(WrenVM* vm)
{
  double angle = wrenGetArgumentDouble(vm, 1);
  double tangent = tan(angle);
  wrenReturnDouble(vm, tangent);
}

static void mathDeg(WrenVM* vm)
{
  double radians = wrenGetArgumentDouble(vm, 1);
  double degrees = floor(radians * 57.2957795130823208768);
  wrenReturnDouble(vm, degrees);
}

static void mathRad(WrenVM* vm)
{
  double degrees = wrenGetArgumentDouble(vm, 1);
  double radians = floor(degrees / 57.2957795130823208768);
  wrenReturnDouble(vm, radians);
}

static void mathSrand(WrenVM* vm)
{
  time_t now = time(NULL);
  _seed = (int32_t)now;
  wrenReturnNull(vm);
}

static void mathRand(WrenVM* vm)
{
  // https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor/
  _seed = (214013 * _seed + 2531011);
  int16_t value = (_seed >> 16) & 0x7FFF;
  double result = (double)value / (double)(INT16_MAX - 1);
  wrenReturnDouble(vm, result);
}

void wrenLoadMathLibrary(WrenVM* vm)
{
  wrenDefineStaticMethod(vm, "Math", "abs", 1, mathAbs);
  wrenDefineStaticMethod(vm, "Math", "ceil", 1, mathCeil);
  wrenDefineStaticMethod(vm, "Math", "floor", 1, mathFloor);
  wrenDefineStaticMethod(vm, "Math", "int", 1, mathInt);
  wrenDefineStaticMethod(vm, "Math", "frac", 1, mathFrac);
  wrenDefineStaticMethod(vm, "Math", "sin", 1, mathSin);
  wrenDefineStaticMethod(vm, "Math", "cos", 1, mathCos);
  wrenDefineStaticMethod(vm, "Math", "tan", 1, mathTan);
  wrenDefineStaticMethod(vm, "Math", "deg", 1, mathDeg);
  wrenDefineStaticMethod(vm, "Math", "rad", 1, mathRad);
  wrenDefineStaticMethod(vm, "Math", "srand", 0, mathSrand);
  wrenDefineStaticMethod(vm, "Math", "rand", 0, mathRand);
}

#endif
