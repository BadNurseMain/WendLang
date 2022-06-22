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

//Function Error Codes.
#define FN_CALL_INVALID (uint8_t) 0xA0
#define FN_CALL_UNKNOWN (uint8_t) 0xA1

#define FN_LACK_PARAMS (uint8_t) 0xA2
#define FN_DIFF_PARAM (uint8_t) 0xA3

#define FN_RETURN_INVAL (uint8_t) 0xA4

uint8_t makeFunction(uint32_t Location);

uint32_t writeFunctionInstructions(uint8_t* FunctionName, uint32_t StartLocation, LocalNameStruct* Variables, uint32_t* VariableCount, uint32_t* ConditionalCount);

uint8_t callFunction(uint8_t* FunctionName, uint8_t ReturnRegister, uint16_t ParameterCount, uint8_t** Buffer);

uint16_t getParamCount(uint32_t Location);
