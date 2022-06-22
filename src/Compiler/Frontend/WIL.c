#include "WIL.h"
#include "Arith.h"
#include "Func.h"

#define WIL_PROP_FUNCTION (uint8_t) 0xE0
#define WIL_PROP_VARIABLE (uint8_t) 0xE1

typedef struct
{
	uint8_t* Name;
	uint32_t Location;
} NameStruct;

extern NameStruct** PublicNameBuffer;
extern uint32_t PublicFunctionCount;
extern uint32_t PublicVariableCount;

//TokenBuffer.
extern uint8_t** TokenBuffer;
extern uint32_t TokenCount;

FILE* IntermediateFile = 0;

extern uint8_t* stringifyInstruction(uint8_t StringCount, ...);
extern uint8_t getVariableSize(const uint8_t* Type);

void createHeader(uint8_t* Name, uint8_t Params, uint8_t Type)
{
	//Writing the Properties of the Function.
	uint8_t NameLength = strlen(Name);
	fwrite(&NameLength, 1, 1, IntermediateFile);

	fwrite(&Params, 1, 1, IntermediateFile);

	fwrite(&Type, 1, 1, IntermediateFile);

	//Writing the Function Name.
	fwrite(Name, 1, NameLength, IntermediateFile);
	return;
}

void generateGlobals(NameStruct* Names)
{
	for (uint32_t x = 0; x < PublicVariableCount; x++)
	{
		uint32_t Location = Names[x].Location;
	}
	return;
}

void writeFunctionParams(uint32_t ParamCount, uint32_t StartLocation)
{
	//Writing Total Amount of Params.
	fwrite(&ParamCount, 1, 4, IntermediateFile);

	for(uint8_t x = 0; x < ParamCount; x++)
	{
		//Writing the Length.
		uint8_t ParamLength = (uint8_t)strlen(TokenBuffer[x * 4 + StartLocation]);
		fwrite(&ParamLength, 1, 1, IntermediateFile);

		//The return Type.
		uint8_t Type = getVariableSize(TokenBuffer[x * 4 + 2 + StartLocation]);
		fwrite(&Type, 1, 1, IntermediateFile);

		//Writing the Name of the Parameter.
		fwrite(TokenBuffer[x * 4 + StartLocation], 1, ParamLength, IntermediateFile);
	}

	//This is just to be able to view it easier when debugging file.
	uint32_t Padding = 0;
	fwrite(&Padding, 1, 4, IntermediateFile);

	uint8_t NewLine = '\n';
	fwrite(&NewLine, 1, 1, IntermediateFile);
	return;
}

void generateVariable(uint8_t* Name, uint8_t* Type)
{
	uint8_t* String = stringifyInstruction(5, "; ", Name, " : ", Type, "\n\0");
	fwrite(String, 1, strlen(String), IntermediateFile);
	free(String);
	return;
}

void generateFunctions(NameStruct* FunctionNames)
{
	//Going through each function.
	for (uint32_t x = 0; x < PublicFunctionCount; x++)
	{
		uint32_t Location = FunctionNames[x].Location;

		//Writing the Properties/Name.
		createHeader(FunctionNames[x].Name, 1, 0);

		//Parameters.
		writeFunctionParams(getParamCount(Location), Location + 2);

		//Instructions.
		makeFunction(Location);

		//Writing End Signature.
		int8_t Signature = '$';
		fwrite(&Signature, 1, 1, IntermediateFile);
	}

	return;
}

uint8_t generateIntermediateLanguage(FILE* ReturnFile)
{
	IntermediateFile = ReturnFile;

	//Write the Signature.
	fwrite("WIL", 1, strlen("WIL"), IntermediateFile);

	//Writing the Amount of Descriptions.
	uint32_t DescriptionCount = PublicVariableCount + PublicFunctionCount;
	fwrite(&DescriptionCount, 1, 4, IntermediateFile);

	//Generate Globals.

	//Generate Functions.
	generateFunctions(PublicNameBuffer[0]);
	return 0;
}

