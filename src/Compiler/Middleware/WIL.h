#ifndef STDLIB
#define STDLIB

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#endif

#define METADATA_ERROR 0x31
#define SIGNATURE_ERROR 0x30

#define MALLOC_ERROR 0x11
#define FILE_OPEN_ERROR 0x10

typedef struct
{
	int32_t Count;

	int8_t** Names;

	int8_t* Lengths;
	int8_t* Types;
} ParameterDescription;

typedef struct
{
	int8_t* Equation;

	uint8_t InstructionCount;
	int8_t** Instructions;
} InstructionDescription;

typedef struct
{
	int8_t** Names;

	int8_t* Lengths;
	int8_t* Properties;
	int8_t* Types;

	void* AdditionalProperties;

	uint32_t* InstructionCount;
	InstructionDescription** Instructions;
} WILDescription;

int8_t readWILFile(int8_t* FileLocation);

uint8_t WILtoASM(int8_t* Location);
