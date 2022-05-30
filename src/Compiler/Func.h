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
        * To Call Functions.
        * Handle Parameters of Functions.
        * Handle Return Values of Functions.
        * Make sure functions of type match.
        * Check return type.
*/

uint8_t makeFunction(uint32_t Location);

uint8_t callFunction(uint8_t* FunctionName, uint8_t ReturnRegister, uint16_t ParameterCount, uint8_t** Buffer);
