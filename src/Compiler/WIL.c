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

void writeFunctionParams(uint32_t ParamCount, uint32_t StartLocation)
{	
	uint8_t* String = "\tParams:\n\0";
	fwrite(String, 1, strlen(String), IntermediateFile);

	//Writing Parameters
	for(uint8_t x = 0; x < ParamCount; x++)
	{
		String = stringifyInstruction(5, "\t", TokenBuffer[x * 4 + StartLocation], " : ", TokenBuffer[x * 4 + 2 + StartLocation], "\n\0");
		fwrite(String, 1, strlen(String), IntermediateFile);
		free(String);
	}

	fwrite("\n\0", 1, 1, IntermediateFile);
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
	for(uint32_t x = 0; x < PublicFunctionCount; x++)
	{
		uint32_t Location = FunctionNames[x].Location;
		
		printf("Location: %s \n", TokenBuffer[Location]);	

		//Extra Setup.
		createHeader(FunctionNames[x].Name);
		createMetadata("Function ", 0);

		uint8_t* String = "{\n\0";
		fwrite(String, 1, strlen(String), IntermediateFile);
		
		//Parameters.
		writeFunctionParams(getParamCount(Location), Location + 2);
		
		//Instructions.
		fwrite("\tText:\n\0", 1, strlen("\tText:\n\0"), IntermediateFile);
		//makeFunction(Location);

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
