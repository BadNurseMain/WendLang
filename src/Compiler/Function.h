#ifndef STDLIB
#define STDLIB

#include <stdlib.h>
#include <stdio.h>

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#endif

uint8_t getFunctionReturnType(uint32_t Location);

uint8_t writeFunctionParameters(FILE* File, uint32_t Location);

uint8_t getFunctionStatements(FILE* File, uint32_t Location);
