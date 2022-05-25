#include "Func.h"
#include "Arith.h"

#ifndef ERR_DBG
#define ERR_DBG

#pragma warning(disable : 6386)
#pragma warning(disable : 6385)

//Definitions for NameBuffer.
#define FUNCTIONNAME (uint8_t)0
#define VARIABLENAME (uint8_t)1

//Error Codes.
#define INVALID_NAME_CHAR (uint8_t)2
#define NAME_ELSEWHERE (uint8_t)3
#define BUFFER_INIT_ERROR (uint8_t)4
#define NO_VALID_VARIABLE (uint8_t)5


//Function Error Codes.
#define RETURN_TYPE_INVALID (uint8_t) 6


//STD Error Codes.
#define MALLOC_NO_MEM (uint8_t)7

#endif

#define RETURN_TYPE_VOID (uint8_t) 1
#define RETURN_TYPE_VAL (uint8_t) 1 << 1

//OutputFile.
extern FILE* OutputFile;

//Global Variables.
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

//Externs.
extern uint8_t ModifyStack(uint8_t Instruction, uint8_t* Value);
extern uint8_t* stringifyInstruction(uint8_t StringCount, ...);

uint8_t makeFunction(uint32_t Location)
{
    uint8_t* String = 0;

    uint32_t ParameterCount = 0, VariableCount = 0, StackOffset = 0, Scope = 1;
    LocalNameStruct* Variables = malloc(sizeof(LocalNameStruct) * 10);
    if (!Variables) return MALLOC_NO_MEM;

    uint32_t LoopOffset = Location;
    uint8_t ReturnType = 0;

    do
    {
        if (TokenBuffer[LoopOffset + 1][0] == ':')
        {
            //Making Sure Doesn't Abruptly End.
            if (TokenBuffer[LoopOffset + 2][0] == ')') return NO_VALID_VARIABLE;

            //Get First Letter.
            uint8_t Character = TokenBuffer[LoopOffset + 2][0];
            if (Character < 'A' || Character > 'z') return INVALID_NAME_CHAR;
            if (Character >= ':' && Character <= '@') return INVALID_NAME_CHAR;
            if (Character == '(' || Character == ')' || Character == '{' || Character == '}') return INVALID_NAME_CHAR;

            //Make Sure isn't also Public Variable.
            for (uint32_t y = 0; y < PublicVariableCount; y++)
                if (!strcmp(TokenBuffer[LoopOffset], PublicNameBuffer[VARIABLENAME][0].Name)) return NO_VALID_VARIABLE;

            //Checking Memory.
            if (!(VariableCount % 10) && VariableCount)
            {
                void* TempBuffer = realloc(Variables, sizeof(LocalNameStruct) * (VariableCount / 10 + 1));
                if (!TempBuffer) return MALLOC_NO_MEM;

                Variables = TempBuffer;
            }

            LocalNameStruct TempStruct = { 0 };
            TempStruct.Name = TokenBuffer[LoopOffset];
            TempStruct.StackOffset = 4 * StackOffset++;

            Variables[VariableCount++] = TempStruct;
            LoopOffset += 3;
            continue;
        }

        LoopOffset++;
    } while (TokenBuffer[LoopOffset][0] != ')');

    LoopOffset += 2;

    do
    {
        //Changing Existing Variable.
        if (TokenBuffer[LoopOffset][0] == '=')
        {
            uint8_t Type = 0;
            if (TokenBuffer[LoopOffset - 2][0] == ':')
            {
                LocalNameStruct TempStruct = { 0 };
                TempStruct.Name = TokenBuffer[LoopOffset - 3];
                TempStruct.StackOffset = 4 * StackOffset++;

                Variables[VariableCount++] = TempStruct;
                Type = 1;

                for (uint32_t y = 0; y < VariableCount; y++)
                    if (!strcmp(TokenBuffer[LoopOffset - 3], Variables[y].Name))
                        LoopOffset = complexArith(LoopOffset, Variables, VariableCount, Type);
                continue;
            }

            for (uint32_t y = 0; y < VariableCount; y++)
            {
                printf("Variable Name: %s \n", Variables[y].Name);

                if (!strcmp(TokenBuffer[LoopOffset - 1], Variables[y].Name))
                    LoopOffset = complexArith(LoopOffset, Variables, VariableCount, Type);
            }

            continue;
        }

        if (TokenBuffer[LoopOffset][0] == '{' || TokenBuffer[LoopOffset][0] == '}')
        {
            Scope += (TokenBuffer[LoopOffset][0] - 124) * -1;
            LoopOffset++;
            continue;
        }

        if (!strcmp(TokenBuffer[LoopOffset], "ret"))
        {
            printf("Return: %s Value: %d \n", TokenBuffer[LoopOffset + 1], TokenBuffer[LoopOffset + 1][0]);

            if (TokenBuffer[LoopOffset + 1][0] >= '0' && TokenBuffer[LoopOffset + 1][0] <= '9')
            {
                if (ReturnType)
                    if (ReturnType != RETURN_TYPE_VAL) return RETURN_TYPE_INVALID;

                String = stringifyInstruction(5, "mov eax, ", TokenBuffer[LoopOffset + 1], "\n\0", "ret ", "\n\0");
                fwrite(String, 1, strlen(String), OutputFile);
                free(String);

                ReturnType = RETURN_TYPE_VAL;
                LoopOffset += 3;
                continue;
            }

            if (ReturnType)
                if (ReturnType != RETURN_TYPE_VOID) return RETURN_TYPE_INVALID;

            String = stringifyInstruction(2, TokenBuffer[LoopOffset], "\n\0");
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            ReturnType = RETURN_TYPE_VOID;
            LoopOffset += 2;
            continue;
        }

        LoopOffset++;
    } while (Scope && TokenBuffer[LoopOffset][0] != '}');


    free(Variables);
    
    fwrite("\n\n\n\n\0", 1, strlen("\n\n\n\n\0"), OutputFile);
    return 0;
}

uint8_t callFunction(uint8_t ReturnRegister)
{

    return 0;
}
