#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "primitives.h"
#include "vm.h"

static Value primitive_metaclass_new(VM* vm, Fiber* fiber, Value* args,
                                     int numArgs);

// Pushes [value] onto the top of the stack.
static void push(Fiber* fiber, Value value);

// Removes and returns the top of the stack.
static Value pop(Fiber* fiber);

VM* newVM()
{
  VM* vm = malloc(sizeof(VM));
  initSymbolTable(&vm->symbols);
  initSymbolTable(&vm->globalSymbols);
  registerPrimitives(vm);

  return vm;
}

void freeVM(VM* vm)
{
  clearSymbolTable(&vm->symbols);
  free(vm);
}

Value makeBool(int value)
{
  Obj* obj = malloc(sizeof(Obj));
  obj->type = value ? OBJ_TRUE : OBJ_FALSE;
  obj->flags = 0;
  return obj;
}

ObjBlock* makeBlock()
{
  ObjBlock* block = malloc(sizeof(ObjBlock));
  block->obj.type = OBJ_BLOCK;
  block->obj.flags = 0;
  return block;
}

ObjClass* makeSingleClass()
{
  ObjClass* obj = malloc(sizeof(ObjClass));
  obj->obj.type = OBJ_CLASS;
  obj->obj.flags = 0;

  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    obj->methods[i].type = METHOD_NONE;
  }

  return obj;
}

ObjClass* makeClass()
{
  ObjClass* classObj = makeSingleClass();

  // Make its metaclass.
  // TODO(bob): What is the metaclass's metaclass?
  classObj->metaclass = makeSingleClass();

  return classObj;
}

ObjInstance* makeInstance(ObjClass* classObj)
{
  ObjInstance* instance = malloc(sizeof(ObjInstance));
  instance->obj.type = OBJ_INSTANCE;
  instance->obj.flags = 0;
  instance->classObj = classObj;

  return instance;
}

Value makeNull()
{
  Obj* obj = malloc(sizeof(Obj));
  obj->type = OBJ_NULL;
  obj->flags = 0;
  return obj;
}

ObjNum* makeNum(double number)
{
  ObjNum* num = malloc(sizeof(ObjNum));
  num->obj.type = OBJ_NUM;
  num->obj.flags = 0;
  num->value = number;
  return num;
}

ObjString* makeString(const char* text)
{
  ObjString* string = malloc(sizeof(ObjString));
  string->obj.type = OBJ_STRING;
  string->obj.flags = 0;
  string->value = text;
  return string;
}

void initSymbolTable(SymbolTable* symbols)
{
  symbols->count = 0;
}

void clearSymbolTable(SymbolTable* symbols)
{
  for (int i = 0; i < symbols->count; i++)
  {
    free(symbols->names[i]);
  }
}

int addSymbolUnchecked(SymbolTable* symbols, const char* name, size_t length)
{
  symbols->names[symbols->count] = malloc(length + 1);
  strncpy(symbols->names[symbols->count], name, length);
  symbols->names[symbols->count][length] = '\0';

  return symbols->count++;
}

int addSymbol(SymbolTable* symbols, const char* name, size_t length)
{
  // If already present, return an error.
  if (findSymbol(symbols, name, length) != -1) return -1;

  return addSymbolUnchecked(symbols, name, length);
}

int ensureSymbol(SymbolTable* symbols, const char* name, size_t length)
{
  // See if the symbol is already defined.
  int existing = findSymbol(symbols, name, length);
  if (existing != -1) return existing;

  // New symbol, so add it.
  return addSymbolUnchecked(symbols, name, length);
}

int findSymbol(SymbolTable* symbols, const char* name, size_t length)
{
  // See if the symbol is already defined.
  // TODO(bob): O(n). Do something better.
  for (int i = 0; i < symbols->count; i++)
  {
    if (strlen(symbols->names[i]) == length &&
        strncmp(symbols->names[i], name, length) == 0) return i;
  }

  return -1;
}

const char* getSymbolName(SymbolTable* symbols, int symbol)
{
  return symbols->names[symbol];
}

// TODO(bob): For debugging. Move to separate module.
/*
void dumpCode(ObjBlock* block)
{
  unsigned char* bytecode = block->bytecode;
  int done = 0;
  int i = 0;
  while (!done)
  {
    printf("%04d  ", i);
    unsigned char code = bytecode[i++];

    switch (code)
    {
      case CODE_CONSTANT:
      {
        int constant = bytecode[i++];
        printf("CONSTANT %d (", constant);
        printValue(block->constants[constant]);
        printf(")\n");
        printf("%04d  (constant %d)\n", i - 1, constant);
        break;
      }

      case CODE_NULL:
        printf("NULL\n");
        break;

      case CODE_FALSE:
        printf("FALSE\n");
        break;

      case CODE_TRUE:
        printf("TRUE\n");
        break;

      case CODE_CLASS:
        printf("CLASS\n");
        break;

      case CODE_METHOD:
      {
        int symbol = bytecode[i++];
        int constant = bytecode[i++];
        printf("METHOD symbol %d constant %d\n", symbol, constant);
        printf("%04d  (symbol %d)\n", i - 2, symbol);
        printf("%04d  (constant %d)\n", i - 1, constant);
        break;
      }

      case CODE_LOAD_LOCAL:
      {
        int local = bytecode[i++];
        printf("LOAD_LOCAL %d\n", local);
        printf("%04d  (local %d)\n", i - 1, local);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = bytecode[i++];
        printf("STORE_LOCAL %d\n", local);
        printf("%04d  (local %d)\n", i - 1, local);
        break;
      }

      case CODE_LOAD_GLOBAL:
      {
        int global = bytecode[i++];
        printf("LOAD_GLOBAL %d\n", global);
        printf("%04d  (global %d)\n", i - 1, global);
        break;
      }

      case CODE_STORE_GLOBAL:
      {
        int global = bytecode[i++];
        printf("STORE_GLOBAL %d\n", global);
        printf("%04d  (global %d)\n", i - 1, global);
        break;
      }

      case CODE_DUP:
        printf("DUP\n");
        break;

      case CODE_POP:
        printf("POP\n");
        break;

      case CODE_CALL_0:
      case CODE_CALL_1:
      case CODE_CALL_2:
      case CODE_CALL_3:
      case CODE_CALL_4:
      case CODE_CALL_5:
      case CODE_CALL_6:
      case CODE_CALL_7:
      case CODE_CALL_8:
      case CODE_CALL_9:
      case CODE_CALL_10:
      {
        // Add one for the implicit receiver argument.
        int numArgs = bytecode[i - 1] - CODE_CALL_0;
        int symbol = bytecode[i++];
        printf("CALL_%d %d\n", numArgs, symbol);
        printf("%04d  (symbol %d)\n", i - 1, symbol);
        break;
      }

      case CODE_JUMP:
      {
        int offset = bytecode[i++];
        printf("JUMP %d\n", offset);
        printf("%04d  (offset %d)\n", i - 1, offset);
        break;
      }

      case CODE_JUMP_IF:
      {
        int offset = bytecode[i++];
        printf("JUMP_IF %d\n", offset);
        printf("%04d  (offset %d)\n", i - 1, offset);
        break;
      }

      case CODE_END:
      {
        printf("CODE_END\n");
        done = 1;
        break;
      }

      default:
        printf("[%d]\n", bytecode[i - 1]);
        break;
    }
  }
}
*/

Value interpret(VM* vm, ObjBlock* block)
{
  Fiber fiber;
  fiber.stackSize = 0;
  fiber.numFrames = 0;

  callBlock(&fiber, block, 0);

  for (;;)
  {
    CallFrame* frame = &fiber.frames[fiber.numFrames - 1];

    switch (frame->block->bytecode[frame->ip++])
    {
      case CODE_CONSTANT:
      {
        int constant = frame->block->bytecode[frame->ip++];
        Value value = frame->block->constants[constant];
        push(&fiber, value);
        break;
      }

      case CODE_NULL:
        push(&fiber, makeNull());
        break;

      case CODE_FALSE:
        push(&fiber, makeBool(0));
        break;

      case CODE_TRUE:
        push(&fiber, makeBool(1));
        break;

      case CODE_CLASS:
      {
        ObjClass* classObj = makeClass();

        // Define a "new" method on the metaclass.
        // TODO(bob): Can this be inherited?
        int newSymbol = ensureSymbol(&vm->symbols, "new", strlen("new"));
        classObj->metaclass->methods[newSymbol].type = METHOD_PRIMITIVE;
        classObj->metaclass->methods[newSymbol].primitive =
            primitive_metaclass_new;

        push(&fiber, (Value)classObj);
        break;
      }

      case CODE_METHOD:
      {
        int symbol = frame->block->bytecode[frame->ip++];
        int constant = frame->block->bytecode[frame->ip++];
        ObjClass* classObj = (ObjClass*)fiber.stack[fiber.stackSize - 1];

        ObjBlock* body = (ObjBlock*)frame->block->constants[constant];
        classObj->methods[symbol].type = METHOD_BLOCK;
        classObj->methods[symbol].block = body;
        break;
      }

      case CODE_LOAD_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        push(&fiber, fiber.stack[frame->locals + local]);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        fiber.stack[frame->locals + local] = fiber.stack[fiber.stackSize - 1];
        break;
      }

      case CODE_LOAD_GLOBAL:
      {
        int global = frame->block->bytecode[frame->ip++];
        push(&fiber, vm->globals[global]);
        break;
      }

      case CODE_STORE_GLOBAL:
      {
        int global = frame->block->bytecode[frame->ip++];
        vm->globals[global] = fiber.stack[fiber.stackSize - 1];
        break;
      }

      case CODE_DUP:
        push(&fiber, fiber.stack[fiber.stackSize - 1]);
        break;
        
      case CODE_POP:
        pop(&fiber);
        break;

      case CODE_CALL_0:
      case CODE_CALL_1:
      case CODE_CALL_2:
      case CODE_CALL_3:
      case CODE_CALL_4:
      case CODE_CALL_5:
      case CODE_CALL_6:
      case CODE_CALL_7:
      case CODE_CALL_8:
      case CODE_CALL_9:
      case CODE_CALL_10:
      {
        // Add one for the implicit receiver argument.
        int numArgs = frame->block->bytecode[frame->ip - 1] - CODE_CALL_0 + 1;
        int symbol = frame->block->bytecode[frame->ip++];

        Value receiver = fiber.stack[fiber.stackSize - numArgs];

        ObjClass* classObj;
        switch (receiver->type)
        {
          case OBJ_BLOCK:
            classObj = vm->blockClass;
            break;

          case OBJ_CLASS:
            classObj = ((ObjClass*)receiver)->metaclass;
            break;

          case OBJ_FALSE:
          case OBJ_TRUE:
            classObj = vm->boolClass;
            break;

          case OBJ_NULL:
            classObj = vm->nullClass;
            break;

          case OBJ_NUM:
            classObj = vm->numClass;
            break;

          case OBJ_STRING:
            classObj = vm->stringClass;
            break;

          case OBJ_INSTANCE:
            classObj = ((ObjInstance*)receiver)->classObj;
            break;
        }

        Method* method = &classObj->methods[symbol];
        switch (method->type)
        {
          case METHOD_NONE:
            printf("Receiver ");
            printValue(receiver);
            printf(" does not implement method \"%s\".\n",
                   vm->symbols.names[symbol]);
            // TODO(bob): Throw an exception or halt the fiber or something.
            exit(1);
            break;

          case METHOD_PRIMITIVE:
          {
            Value* args = &fiber.stack[fiber.stackSize - numArgs];
            Value result = method->primitive(vm, &fiber, args, numArgs);

            // If the primitive pushed a call frame, it returns NULL.
            if (result != NULL)
            {
              fiber.stack[fiber.stackSize - numArgs] = result;
              
              // Discard the stack slots for the arguments (but leave one for
              // the result).
              fiber.stackSize -= numArgs - 1;
            }
            break;
          }
            
          case METHOD_BLOCK:
            callBlock(&fiber, method->block, numArgs);
            break;
        }
        break;
      }

      case CODE_JUMP:
      {
        int offset = frame->block->bytecode[frame->ip++];
        frame->ip += offset;
        break;
      }

      case CODE_JUMP_IF:
      {
        int offset = frame->block->bytecode[frame->ip++];
        Value condition = pop(&fiber);

        // False is the only falsey value.
        if (condition->type == OBJ_FALSE)
        {
          frame->ip += offset;
        }
        break;
      }

      case CODE_END:
      {
        Value result = pop(&fiber);
        fiber.numFrames--;

        // If we are returning from the top-level block, just return the value.
        if (fiber.numFrames == 0) return result;

        // Store the result of the block in the first slot, which is where the
        // caller expects it.
        fiber.stack[frame->locals] = result;

        // Discard the stack slots for the locals (leaving one slot for the
        // result).
        fiber.stackSize = frame->locals + 1;
      }
    }
  }
}

void callBlock(Fiber* fiber, ObjBlock* block, int numArgs)
{
  fiber->frames[fiber->numFrames].block = block;
  fiber->frames[fiber->numFrames].ip = 0;
  fiber->frames[fiber->numFrames].locals = fiber->stackSize - numArgs;

  // Make empty slots for locals.
  // TODO(bob): Should we do this eagerly, or have the bytecode have pushnil
  // instructions before each time a local is stored for the first time?
  for (int i = 0; i < block->numLocals - numArgs; i++)
  {
    push(fiber, NULL);
  }

  fiber->numFrames++;
}

void printValue(Value value)
{
  // TODO(bob): Do more useful stuff here.
  switch (value->type)
  {
    case OBJ_BLOCK:
      printf("[block]");
      break;

    case OBJ_CLASS:
      printf("[class]");
      break;

    case OBJ_FALSE:
      printf("false");
      break;

    case OBJ_INSTANCE:
      printf("[instance]");
      break;

    case OBJ_NULL:
      printf("null");
      break;

    case OBJ_NUM:
      printf("%g", ((ObjNum*)value)->value);
      break;

    case OBJ_STRING:
      printf("%s", ((ObjString*)value)->value);
      break;

    case OBJ_TRUE:
      printf("true");
      break;
  }
}

Value primitive_metaclass_new(VM* vm, Fiber* fiber, Value* args, int numArgs)
{
  ObjClass* classObj = (ObjClass*)args[0];
  // TODO(bob): Invoke initializer method.
  return (Value)makeInstance(classObj);
}

void push(Fiber* fiber, Value value)
{
  // TODO(bob): Check for stack overflow.
  fiber->stack[fiber->stackSize++] = value;
}

Value pop(Fiber* fiber)
{
  return fiber->stack[--fiber->stackSize];
}
