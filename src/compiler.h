#ifndef wren_parser_h
#define wren_parser_h

#include "lexer.h"
#include "vm.h"

Block* compile(Buffer* source, Token* tokens);

#endif
