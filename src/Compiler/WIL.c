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

void createHeader(uint8_t* Name)
{
	Name = stringifyInstruction(3, "{ Name : ", Name, " }\n\0");
	fwrite(Name, 1, strlen(Name), IntermediateFile);
	free(Name);
	return;
}

void createMetadata(uint8_t* Type, uint16_t Properties)
{
	Type = stringifyInstruction(3, "{ Type : ", Type, "}\n\0");
	fwrite(Type, 1, strlen(Type), IntermediateFile);
	free(Type);

	return;
}

void generateTable(uint8_t* Name, uint8_t* Type, uint16_t Properties, uint32_t StartLocation)
{
	//Creates the Initial Header.
	createHeader(Name);

	//Create the Metadata.
	createMetadata(Type, Properties);

	if(Properties == WIL_PROP_VARIABLE)
	{
		return;
	}

	return;
}

void generateGlobals(NameStruct* Names)
{
	for(uint32_t x = 0; x < PublicVariableCount; x++)
	{
		uint32_t Location = Names[x].Location;
		createHeader(Names[x].Name);
		createMetadata(TokenBuffer[Location + 2], 0);
	}
	return;
}

void writeVariable(uint8_t TabOffset)
{

	return;
}

void writeFunctionParams(uint32_t ParamCount, uint32_t StartLocation)
{	
	uint8_t* String = "\tParams:\n\0";
	fwrite(String, 1, strlen(String), IntermediateFile);

	//Writing Parameters
	for(uint8_t x = 0; x < ParamCount; x++)
	{
		String = stringifyInstruction(5, "\t", TokenBuffer[x * 3], " : ", TokenBuffer[x * 3 + 1], "\n\0");
		fwrite(String, 1, strlen(String), IntermediateFile);
		free(String);
	}
	return;
}

void generateVariable(uint8_t* Name, uint8_t* Type)
{

	return;
}

void generateFunctions(NameStruct* FunctionNames)
{
	//Going through each function.
	for(uint32_t x = 0; x < PublicFunctionCount; x++)
	{
		uint32_t Location = FunctionNames[x].Location;
		
		//Extra Setup.
		createHeader(FunctionNames[x].Name);
		createMetadata("Function ", 0);

		uint8_t* String = "{\n\0";
		fwrite(String, 1, strlen(String), IntermediateFile);
		
		//Parameters.
		writeFunctionParams(0, Location);
		
		//Instructions.
		fwrite("\tText:\n\0", 1, strlen("\tText:\n\0"), IntermediateFile);
		makeFunction(Location);

		fwrite("}\n\n\0", 1, strlen("}\n\n\0"), IntermediateFile);
	}
	return;
}

uint8_t generateIntermediateLanguage(FILE* ReturnFile)
{
	IntermediateFile = ReturnFile;

	//Generate Globals.

	//Generate Functions.
	generateFunctions(PublicNameBuffer[0]);
	return 0;
}