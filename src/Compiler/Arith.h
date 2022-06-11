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

#define ARTH_INVALID_SIZE (uint8_t) 0xC0
#define ARTH_UNKNOWN_VAR (uint8_t) 0xC1

#define ARTH_TYPE_STACK (uint8_t) (1 << 7)
#define ARTH_TYPE_STORE_A (uint8_t) 1
#define ARTH_TYPE_STORE_B (uint8_t) 2

#ifndef NAMESTRUCT
#define NAMESTRUCT

typedef struct
{
    uint8_t* Name;
    uint8_t Type;
    uint32_t StackOffset;
} LocalNameStruct;
#endif

uint32_t writeConditionalOperations(uint8_t* FunctionName, uint32_t StartLocation, LocalNameStruct* Variables, uint32_t VariableCount, uint32_t ConditionalCount, uint8_t OptionalParam);

uint32_t writeArithmeticOperations(uint8_t TabOffset, uint32_t StartLocation, LocalNameStruct* Variables, uint32_t VariableCount, uint8_t OptionalParam);