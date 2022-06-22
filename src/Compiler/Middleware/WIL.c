#include "WIL.h"
/*
 * 	Byte		Description
 * 	3			Signature "WIL"
 *
 *	4 			Function/Global Count.
 *
 * 	1			Length of Name
 * 	1 			Properties (Function/Container/Global)
 * 	1			Type / Return Type
 * 				Name
 *	1			Param Count (If Function)
 *
 *	ParamCount:
 *	1			Length of Param Name
 *	1 			Type
 *				Param Name
 *
 *	4			Padding (Used of Debugging)
 *	1			New Line '\n'
 *
 *	Instructions:
 *	2			Equation Length.
 *				; Name : Type {Equation} \n
 *				{Instruction Length}
 *				{Instructions}
 *	2			End of Instructions"~~"
 *
 *	End:
 *	1			End of Function '$'
 */

int8_t getInstructionInfo(FILE* File, InstructionDescription* WIL)
{
	//Getting the String length of Equation.
	uint16_t EquationLength = 0;
	fread(&EquationLength, 1, 2, File);

	//Reading the Equation.
	WIL->Equation = malloc(EquationLength + 1);
	fread(WIL->Equation, 1, EquationLength, File);
	WIL->Equation[EquationLength] = '\0';

	printf("Equation: %s \n", WIL->Equation);

	WIL->Instructions = malloc(20 * sizeof(int8_t*));

	for(uint16_t InstructionLength = 0; fread(&InstructionLength, 1, 2, File);)
		{
			//Is Indication of End of Instructions.
			if(InstructionLength == 0x7e7e) break;

			//Reading Instruction.
			WIL->Instructions[WIL->InstructionCount] = malloc(InstructionLength + 1);
			fread(WIL->Instructions[WIL->InstructionCount++], 1, InstructionLength, File);

			WIL->Instructions[WIL->InstructionCount - 1][InstructionLength] = '\0';
			printf("Instruction: %s\n", WIL->Instructions[WIL->InstructionCount - 1]);
		}

	return 0;
}

int8_t getDescriptionInfo(FILE* File, WILDescription* WIL, uint32_t Count)
{
	//Getting Intial Properties.
	fread(&WIL->Lengths[Count], 1, 1, File);
	fread(&WIL->Properties[Count], 1, 1, File);
	fread(&WIL->Types[Count], 1, 1, File);

	//Writing the Name.
	WIL->Names[Count] = malloc(WIL->Lengths[Count] + 1);
	fread(WIL->Names[Count], 1, WIL->Lengths[Count], File);
	WIL->Names[Count][WIL->Lengths[Count]] = '\0';

	//Is a Function.
	if(WIL->Properties[Count] == 1)
	{
		WIL->AdditionalProperties = malloc(sizeof(ParameterDescription));
		ParameterDescription* Params = (ParameterDescription*)WIL->AdditionalProperties;

		fread(&Params->Count, 1, 4, File);
		if(!Params->Count) return 0;

		//Allocating Memory.
		Params->Names = malloc(sizeof(int8_t*) * Params->Count);

		Params->Lengths = malloc(Params->Count);
		Params->Types = malloc(Params->Count);

		//Looping Through All Parameters.
		for(uint8_t x = 0; x < Params->Count; x++)
		{
			fread(&Params->Lengths[x], 1, 1, File);
			fread(&Params->Types[x], 1, 1, File);

			Params->Names[x] = malloc(Params->Lengths[x] + 1);
			fread(Params->Names[x], 1, Params->Lengths[x], File);
		}

	}

	return 0;
}

void displayFunctionContents(WILDescription* WIL, uint32_t Count)
{
	printf("NAME: %s\nTYPE: %d\n", WIL->Names[Count], WIL->Properties[Count]);

	for(uint32_t x = 0; x < WIL->InstructionCount[Count]; x++)
		printf("EQUATION: %s", WIL->Instructions[Count][x].Equation);

	return;
}

int8_t readWILFile(int8_t* FileLocation)
{
	FILE* File = fopen(FileLocation, "rb");
	if(!File) return FILE_OPEN_ERROR;

	//Getting the Signature of the File.
	int8_t WILSignature[4] = {0};
	fread(WILSignature, 1, 3, File);

	if(strcmp(WILSignature, "WIL")) return SIGNATURE_ERROR;

	//Reading Amount of Descriptions Mentioned.
	uint32_t TotalCount = 0;
	fread(&TotalCount, 1, 4, File);

	//Allocating Memory to Store them.
	WILDescription* WIL = malloc(sizeof(WILDescription));

	WIL->Names = malloc(TotalCount * sizeof(int8_t*));

	WIL->Lengths = malloc(TotalCount);
	WIL->Properties = malloc(TotalCount);
	WIL->Types = malloc(TotalCount);

	WIL->InstructionCount = malloc(TotalCount);
	WIL->Instructions = malloc(TotalCount * sizeof(InstructionDescription*));


	for(uint32_t x = 0; x < TotalCount; x++)
	{
		//Getting the Initial Descriptor Information.
		getDescriptionInfo(File, WIL, x);

		//Sanity Check.
		int8_t AssuranceSignature[5];
		fread(AssuranceSignature, 1, 5, File);

		for(int8_t y = 0; y < 4; y++)
			if(AssuranceSignature[y] != 0x00) return SIGNATURE_ERROR;

		if(AssuranceSignature[4] != '\n') return SIGNATURE_ERROR;

		WIL->Instructions[x] = malloc(128 * sizeof(InstructionDescription));

		//Getting Instructions Associated with the Descriptor.
		for(int8_t Check = 0, Count = 0; fread(&Check, 1, 1, File); Count++)
		{
			if(Check == '$') break;

			WIL->InstructionCount[x] = Count + 1;

			fseek(File, -1, SEEK_CUR);
			getInstructionInfo(File, &WIL->Instructions[x][Count]);
		}
	}

	displayFunctionContents(WIL, 0);
	return 0;
}
