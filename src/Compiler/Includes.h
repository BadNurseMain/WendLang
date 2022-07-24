#pragma once

//Universal Headers.
#include <stdlib.h>
#include <stdio.h>

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

//Structs Used Throughout.
typedef struct
{
	uint8_t* Name;
	uint32_t StackOffset;
	uint8_t Type;
} VariableDescriptor;

typedef struct
{
    int8_t* Name;
    uint32_t Location;
} NameStruct;


//--- Widely Used Globals ---//

//Output File.
extern FILE* OutputFile;


//Tokens.
extern int8_t** TokenBuffer;
extern uint32_t TokenCount;


//Sorting Tokens.
extern NameStruct** PublicNameBuffer;
extern uint32_t PublicFunctionCount;
extern uint32_t PublicVariableCount;


//Definitions for NameBuffer.
#define FUNCTIONNAME (uint8_t)0
#define VARIABLENAME (uint8_t)1


//--- Error Codes ---//
#pragma warning(disable : 6386)
#pragma warning(disable : 6385)

//Error Codes.
#define INVALID_NAME_CHAR (uint8_t)2
#define NAME_ELSEWHERE (uint8_t)3
#define MALLOC_ERROR (uint8_t)4
#define NO_VALID_VARIABLE (uint8_t)5
