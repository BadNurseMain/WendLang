#ifndef STDLIB
#define STDLIB

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#endif

/*
    Purpose:
        * Perform Arithmetic, both complex and simple.
        * Assess Precedence.
        * Perform Pointer Logic.
        * Allow Variables self-reference themselves.

*/

typedef struct
{
    uint8_t* Name;
    uint8_t Scope[16];
    uint8_t ScopeCount;
    uint32_t StackOffset;
} LocalNameStruct;

uint8_t complexArith(uint32_t StartLocation, LocalNameStruct* Variables, uint32_t VariableCount, uint8_t OptionalParam);
