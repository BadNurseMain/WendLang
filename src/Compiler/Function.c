#include "Function.h"
#include "Includes.h"

uint8_t* stringifyInstruction(uint8_t StringCount, ...)
{
	//Get List.
	va_list List;
	va_start(List, StringCount);

	size_t Length = 0;

	uint8_t** Buffer = malloc(sizeof(uint8_t*) * StringCount);
	if (!Buffer) return NULL;

	for (uint8_t x = 0; x < StringCount; x++)
	{
		Buffer[x] = va_arg(List, uint8_t*);
		Length += strlen(Buffer[x]);
	}

	//Setting up new String.
	uint8_t* String = malloc(Length + 1);
	if (!String) return NULL;

	strcpy(String, Buffer[0]);

	for (uint8_t x = 1; x < StringCount; x++)
		strcat(String, Buffer[x]);

	free(Buffer);
	return String;
}

uint8_t getFunctionReturnType(uint32_t Location)
{
	uint8_t ReturnType = 0, Scope = 0;

	//Looping until start of function.
	while (TokenBuffer[Location][0] != '{') Location++;

	Scope++;
	Location++;

	while (Scope)
	{
		if (TokenBuffer[Location][0] == '{')
		{
			Scope++;
			Location++;
			continue;
		}
		else if (TokenBuffer[Location][0] == '}')
		{
			Scope--;
			Location++;
			continue;
		}

		//If a return Type is found.
		if (!strcmp("ret ", TokenBuffer[Location]))
		{
			if (TokenBuffer[Location][0] == ';') return 3;
			else return 0;
		}

		Location++;
	}

	//No return present, creating void.
	return 0;
}

uint8_t writeFunctionParameters(FILE* File, uint32_t Location)
{
	//Making sure its a Function.
	if (TokenBuffer[++Location][0] != '(') return 0xFF;


	uint8_t Count = 0;
	if (TokenBuffer[++Location][0] == ')')
	{
		fwrite(&Count, 1, 1, File);
		return 0;
	}

	//Running Through Params.
	uint32_t Loop = Location;

	do
	{
		if (TokenBuffer[Location + 1][0] == ':')
		{
			//Is valid.
			if (TokenBuffer[Location + 3][0] != ',' && TokenBuffer[Location + 3][0] != ')') return 0xFF;
			Location += 4;
			Count++;
			continue;
		}

	} while (TokenBuffer[Location][0] != '{');


	//Writing the Amount.
	fwrite(&Count, 1, 1, File);

	Location = Loop;

	do
	{
		if (TokenBuffer[Location + 1][0] == ':')
		{
			//Is valid.
			if (TokenBuffer[Location + 3][0] != ',' && TokenBuffer[Location + 3][0] != ')') return 0xFF;

			uint8_t ParamLength = strlen(TokenBuffer[Location]);
			fwrite(&ParamLength, 1, 1, File);

			uint8_t Type = 0;

			if (!strcmp("u4", TokenBuffer[Location + 2])) Type = 3;
			else if (!strcmp("u2", TokenBuffer[Location + 2])) Type = 2;
			else if (!strcmp("u1", TokenBuffer[Location + 2])) Type = 1;

			if (!Type) return 0xFF;
			fwrite(&Type, 1, 1, File);

			fwrite(TokenBuffer[Location], 1, ParamLength, File);
			Location += 4;
			continue;
		}

	} while (TokenBuffer[Location][0] != '{');

	return 0;
}

uint8_t getFunctionStatements(FILE* File, uint32_t Location)
{
	uint32_t Loop = Location + 1, VariableCount = 0;

	VariableDescriptor* Variables = malloc(32 * sizeof(VariableDescriptor));

	do
	{
		if (TokenBuffer[Loop + 1][0] == ':')
		{
			//Is valid.
			if (TokenBuffer[Loop + 3][0] != ',' && TokenBuffer[Loop + 3][0] != ')') return 0xFF;

			Variables[VariableCount].Name = TokenBuffer[Loop];

			uint8_t Type = 0;

			if (!strcmp("u4", TokenBuffer[Loop + 2])) Type = 4;
			else if (!strcmp("u2", TokenBuffer[Loop + 2])) Type = 2;
			else if (!strcmp("u1", TokenBuffer[Loop + 2])) Type = 1;

			if (!VariableCount)Variables[VariableCount++].StackOffset = 0;
			else Variables[VariableCount++].StackOffset = Variables[VariableCount - 1].StackOffset + Type;

			Loop += 4;
			continue;
		}

		Loop++;
	} while (TokenBuffer[Loop][0] != '{');

	Variables[VariableCount].Name = malloc(3);
	strncpy(Variables[0].Name, "fn\0", 3);
	Variables[VariableCount].Type = 0;

	if (!VariableCount)Variables[VariableCount++].StackOffset = 0;
	else Variables[VariableCount++].StackOffset = Variables[VariableCount - 1].StackOffset + 4;


	uint32_t Scope = 1;
	Loop++;

	do
	{
		if (TokenBuffer[Loop][0] == '{')
		{
			int8_t* String = "; BLOCK CREATE";
			uint16_t StatementLength = strlen(String);

			fwrite(&StatementLength, 1, 2, File);
			fwrite(String, 1, StatementLength, File);

			Scope++;
			Loop++;
			continue;
		}
		else if (TokenBuffer[Loop][0] == '}')
		{
			Scope--;
			Loop++;

			if (!Scope) continue;

			int8_t* String = "; BLOCK REMOVE";
			uint16_t StatementLength = strlen(String);

			fwrite(&StatementLength, 1, 2, File);
			fwrite(String, 1, StatementLength, File);

			continue;
		}

		//Making of a Variable.
		if (TokenBuffer[Loop][0] == ':')
		{
			uint8_t* String = 0;
			uint16_t StatementLength = 0;

			//No Initial Value.
			if (TokenBuffer[Loop + 2][0] == ';')
			{
				Variables[VariableCount++].Name = TokenBuffer[Loop - 1];

				//Creating Statement.
				String = stringifyInstruction(5, "; CREATE ", TokenBuffer[Loop - 1], " AS ", TokenBuffer[Loop + 1], "\n\0");
				StatementLength = strlen(String);

				//Writing Size and Statement to File.
				fwrite(&StatementLength, 1, 2, File);
				fwrite(String, 1, StatementLength, File);

				Loop += 3;
				continue;
			}

			Variables[VariableCount++].Name = TokenBuffer[Loop - 1];

			String = stringifyInstruction(5, "; CREATE ", TokenBuffer[Loop - 1], " AS ", TokenBuffer[Loop + 1], " {\0");

			//Getting the Equation Length.
			StatementLength = strlen(String);

			for (uint32_t x = Loop + 3; TokenBuffer[x][0] != ';'; x++)
				StatementLength += strlen(TokenBuffer[x]);

			StatementLength += 2;
			fwrite(&StatementLength, 1, 2, File);


			//Writing Out Equation.
			fwrite(String, 1, strlen(String), File);

			for (uint32_t x = Loop + 3; TokenBuffer[x][0] != ';'; x++)
				fwrite(TokenBuffer[x], 1, strlen(TokenBuffer[x]), File);

			String = "}\n\0";
			fwrite(String, 1, 2, File);

			//Moving on.
			while (TokenBuffer[Loop][0] != ';') Loop++;

			Loop++;
			continue;
		}

		//Assignment on Existing Variable.
		if (TokenBuffer[Loop][0] == '=')
		{
			for (uint32_t x = 0; x < VariableCount; x++)
				if (!strcmp(TokenBuffer[Loop - 1], Variables[x].Name))
				{
					uint8_t* String = stringifyInstruction(3, "; CHANGE ", TokenBuffer[Loop - 1], " TO {");

					//Getting the Equation Length.
					uint16_t StatementLength = strlen(String);

					for (uint32_t x = Loop + 1; TokenBuffer[x][0] != ';'; x++)
						StatementLength += strlen(TokenBuffer[x]);

					StatementLength += 2;
					fwrite(&StatementLength, 1, 2, File);

					//Writing Out Equation.
					fwrite(String, 1, strlen(String), File);

					for (uint32_t x = Loop + 1; TokenBuffer[x][0] != ';'; x++)
						fwrite(TokenBuffer[x], 1, strlen(TokenBuffer[x]), File);

					String = "}\n\0";
					fwrite(String, 1, 2, File);

					//Moving on.
					while (TokenBuffer[Loop][0] != ';') Loop++;

					Loop++;
					break;
				}

			continue;
		}

		if (!strcmp("if", TokenBuffer[Loop]))
		{
			int8_t* String = 0;
			uint16_t StatementLength = 0;

			//Gathering Lengths.
			uint32_t LoopOffset = Loop + 1;
			while (TokenBuffer[LoopOffset][0] != '{') LoopOffset++;

			for (uint32_t y = Loop + 2; y < LoopOffset - 1; y++) StatementLength += strlen(TokenBuffer[y]);

			String = "; CONDITION IF {";

			//Extra One is for End Bracket.
			StatementLength += strlen(String) + 1;
			fwrite(&StatementLength, 1, 2, File);

			//Writing to File.
			fwrite(String, 1, strlen(String), File);

			for (uint32_t y = Loop + 2; y < LoopOffset - 1; y++)
				fwrite(TokenBuffer[y], 1, strlen(TokenBuffer[y]), File);

			int8_t EndBracket = '}';
			fwrite(&EndBracket, 1, 1, File);

			Loop = LoopOffset;
			continue;
		}

		if (!strcmp("ret", TokenBuffer[Loop]))
		{
			int8_t* String = 0;
			uint16_t StatementLength = 0;

			//Empty Return.
			if (TokenBuffer[Loop][0] == ';')
			{
				String = "; RETURN\n\0";

				//Getting Statement Size.
				StatementLength = strlen(String);

				fwrite(&StatementLength, 1, 2, File);
				fwrite(String, 1, strlen(String), File);

				Loop += 2;
				continue;
			}

			//Has a Return Value.
			String = "; RETURN {";

			//Getting Statement Size.
			StatementLength = strlen(String);

			for (uint32_t x = Loop + 1; TokenBuffer[x][0] != ';'; x++)
				StatementLength += strlen(TokenBuffer[x]);

			StatementLength += 2;
			fwrite(&StatementLength, 1, 2, File);

			//Writing out Statement.
			fwrite(String, 1, strlen(String), File);

			for (uint32_t x = Loop + 1; TokenBuffer[x][0] != ';'; x++)
				fwrite(TokenBuffer[x], 1, strlen(TokenBuffer[x]), File);

			String = "}\n\0";
			fwrite(String, 1, 2, File);

			//Moving on.
			while (TokenBuffer[Loop][0] != ';') Loop++;

			Loop++;
			continue;
		}

		//Getting Functions.
		for (uint32_t w = 0; w < PublicFunctionCount; w++)
			if (!strcmp(PublicNameBuffer[0][w].Name, TokenBuffer[Loop]))
			{
				uint16_t Offset = Loop + 2;

				int8_t* String = stringifyInstruction(3, "; FUNCTION CALL ", TokenBuffer[Loop], " {");

				for (uint16_t Precedence = 1; Precedence; Offset++)
				{
					if (TokenBuffer[Offset][0] == '(') Precedence++;
					else if (TokenBuffer[Offset][0] == ')') Precedence--;
				}

				uint16_t StatementLength = strlen(String) + 2;

				for (uint16_t z = Loop + 2; z < Offset - 1; z++)
					StatementLength += strlen(TokenBuffer[z]);

				fwrite(&StatementLength, 1, 2, File);

				fwrite(String, 1, strlen(String), File);
				free(String);

				for (uint16_t z = Loop + 2; z < Offset - 1; z++)
					fwrite(TokenBuffer[z], 1, strlen(TokenBuffer[z]), File);

				String = "}\n\0";
				fwrite(String, 1, 2, File);

				Loop = Offset;
				break;

			}

		Loop++;
	} while (Scope);
	return 0;
}