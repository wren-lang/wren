#include "wren_aux_random.h"

#if WREN_AUX_RANDOM

#include <string.h>
#include <time.h>

#include "wren.h"
#include "wren_vm.h"

#include "wren_aux_random.wren.inc"

// Implements the well equidistributed long-period linear PRNG (WELL512a).
//
// https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear
typedef struct
{
  uint32_t state[16];
  uint32_t index;
} Well512;

// Code from: http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
static uint32_t advanceState(Well512* well)
{
  uint32_t a, b, c, d;
  a = well->state[well->index];
  c = well->state[(well->index + 13) & 15];
  b =  a ^ c ^ (a << 16) ^ (c << 15);
  c = well->state[(well->index + 9) & 15];
  c ^= (c >> 11);
  a = well->state[well->index] = b ^ c;
  d = a ^ ((a << 5) & 0xda442d24U);
  
  well->index = (well->index + 15) & 15;
  a = well->state[well->index];
  well->state[well->index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
  return well->state[well->index];
}

static void randomAllocate(WrenVM* vm)
{
  Well512* well = (Well512*)wrenAllocateForeign(vm, sizeof(Well512));
  well->index = 0;
}

static void randomSeed0(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetArgumentForeign(vm, 0);
  
  srand((uint32_t)time(NULL));
  for (int i = 0; i < 16; i++)
  {
    well->state[i] = rand();
  }
}

static void randomSeed1(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetArgumentForeign(vm, 0);
  
  srand((uint32_t)wrenGetArgumentDouble(vm, 1));
  for (int i = 0; i < 16; i++)
  {
    well->state[i] = rand();
  }
}

static void randomSeed16(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetArgumentForeign(vm, 0);
  
  for (int i = 0; i < 16; i++)
  {
    well->state[i] = (uint32_t)wrenGetArgumentDouble(vm, i + 1);
  }
}

static void randomFloat(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetArgumentForeign(vm, 0);
  
  // A double has 53 bits of precision in its mantissa, and we'd like to take
  // full advantage of that, so we need 53 bits of random source data.

  // First, start with 32 random bits, shifted to the left 21 bits.
  double result = (double)advanceState(well) * (1 << 21);
  
  // Then add another 21 random bits.
  result += (double)(advanceState(well) & ((1 << 21) - 1));

  // Now we have a number from 0 - (2^53). Divide be the range to get a double
  // from 0 to 1.0 (half-inclusive).
  result /= 9007199254740992.0;
  
  wrenReturnDouble(vm, result);
}

static void randomInt0(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetArgumentForeign(vm, 0);
  
  wrenReturnDouble(vm, (double)advanceState(well));
}

// TODO: The way these are wired up is pretty verbose and tedious. Also, the
// CLI has its own separate way of handling this. Figure out something cleaner.
static WrenForeignMethodFn bindForeignMethods(WrenVM* vm,
                                              const char* module,
                                              const char* className,
                                              bool isStatic,
                                              const char* signature)
{
  ASSERT(strcmp(module, "random") == 0, "Should be in random module.");
  ASSERT(strcmp(className, "Random") == 0, "Should be in Random class.");
  
  if (strcmp(signature, "<allocate>") == 0) return randomAllocate;
  if (strcmp(signature, "seed_()") == 0) return randomSeed0;
  if (strcmp(signature, "seed_(_)") == 0) return randomSeed1;

  if (strcmp(signature, "seed_(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)") == 0)
  {
    return randomSeed16;
  }
  
  if (strcmp(signature, "float()") == 0) return randomFloat;
  if (strcmp(signature, "int()") == 0) return randomInt0;

  ASSERT(false, "Unknown method.");
  return NULL;
}

static WrenForeignClassMethods bindForeignClass(WrenVM* vm, const char* module,
                                                const char* className)
{
  WrenForeignClassMethods methods;
  methods.allocate = randomAllocate;
  methods.finalize = NULL;
  return methods;
}

// TODO: Lots of duplication between here and meta.
void wrenLoadRandomModule(WrenVM* vm)
{
  WrenBindForeignMethodFn previousBindMethodFn = vm->config.bindForeignMethodFn;
  vm->config.bindForeignMethodFn = bindForeignMethods;
  
  WrenBindForeignClassFn previousBindClassFn = vm->config.bindForeignClassFn;
  vm->config.bindForeignClassFn = bindForeignClass;
  
  wrenInterpretInModule(vm, "random", randomModuleSource);
  
  vm->config.bindForeignMethodFn = previousBindMethodFn;
  vm->config.bindForeignClassFn = previousBindClassFn;
}

#endif
