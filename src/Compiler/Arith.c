#include "Arith.h"
#include "ArithGlobals.h"

//----------------------------------------------------------- INIT ------------------------------------------------------

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

#endif


#define ORDER_SIZE (uint8_t)12

//OutputFile.
extern FILE* OutputFile;

//Global Variables.
typedef struct
{
    uint8_t* Name;
    uint32_t Location;
} NameStruct;

extern NameStruct** NameBuffer;
extern uint32_t FunctionCount;
extern uint32_t VariableCount;

//TokenBuffer.
extern uint8_t** TokenBuffer;
extern uint32_t TokenCount;

//Storing Local Variables.
typedef struct
{
    uint8_t* Name;
    uint8_t Scope[16];
    uint8_t ScopeCount;
    uint32_t StackOffset;
} LocalNameStruct;


//Table of Precedence
enum PrecedenceTable
{
    Multiply = 0,
    Division = 1,
    Modulus = 2,
    Addition = 3,
    Subtraction = 4,
    BitShift = 5,
    Relational = 6,
    Equals = 7
};

//VA For Creating ASM File.
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

uint8_t isNotComplex(uint32_t StartLocation, void* LocalVarBuffer, uint32_t VarCount)
{
    //Setup.
    uint8_t* String = 0;
    LocalNameStruct* LocalVar = (LocalNameStruct*)LocalVarBuffer;

    LocalNameStruct ReturnVar = { 0 };

    //Getting the Return Variable.
    for (uint32_t x = 0; x < VarCount; x++)
        if (!strcmp(TokenBuffer[StartLocation - 1], LocalVar[x].Name))
            ReturnVar = LocalVar[x];

    if (!ReturnVar.Name) return 1;

    //Checking if it is a Number.
    if (TokenBuffer[StartLocation + 1][0] >= '0' && TokenBuffer[StartLocation + 1][0] <= '9')
    {
        String = stringifyInstruction(5, MOVE, REGISTERS[3][0], START, TokenBuffer[StartLocation + 1], END);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        goto assignReturn;
    }

    //Check for Global Variable.
    for (uint32_t x = 0; x < VariableCount; x++)
        if (!strcmp(TokenBuffer[StartLocation + 1], NameBuffer[1][x].Name))
        {

        }


    //Check for Local Variable.
    for (uint32_t x = 0; x < VarCount; x++)
        if (!strcmp(TokenBuffer[StartLocation + 1], LocalVar[x].Name))
        {
            //Stringify Stack Offset.
            uint8_t StackOffset[12] = { 0 };
            sprintf(StackOffset, "%d", LocalVar[x].StackOffset);

            String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], PLUS, StackOffset, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            goto assignReturn;
        }

    return 1;

    //Assigning the Values from the Returns.
assignReturn:
    uint8_t ReturnStack[12] = { 0 };
    sprintf(ReturnStack, "%d", VarCount * 4 - ReturnVar.StackOffset);

    String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], PLUS, ReturnStack, NEWVAREND, REGISTERS[3][0], END);
    fwrite(String, 1, strlen(String), OutputFile);
    free(String);
    return 0;
}

uint32_t* getOrder(uint32_t StartLocation)
{
    uint32_t* Orders = calloc(ORDER_SIZE, sizeof(uint32_t));
    if (!Orders) return 0;

    for (uint32_t x = StartLocation; TokenBuffer[x][0] != ')' && TokenBuffer[x][0] != ';' && TokenBuffer[x][0] != '('; x++)
    {
        if (TokenBuffer[x][0] == '*')
        {
            if (Orders[Multiply]) return 0;
            Orders[Multiply] = x;
            continue;
        }

        if (TokenBuffer[x][0] == '+')
        {
            if (Orders[Addition]) return 0;
            Orders[Addition] = x;
            continue;
        }

        if (TokenBuffer[x][0] == '-')
        {
            if (Orders[Subtraction]) return 0;
            Orders[Subtraction] = x;
            continue;
        }

        if (TokenBuffer[x][0] == '/')
        {
            if (Orders[Division]) return 0;
            Orders[Division] = x;
            continue;
        }
    }
    return Orders;
}

uint8_t writeOperation(uint32_t OperatorLocation, uint32_t* Orders, uint8_t Offset)
{
    uint8_t* String = 0;

    //Has an Operator in Front of First Value.
    for (uint8_t y = 0; y < ORDER_SIZE; y++)
        if (!(Orders[Offset] - Orders[y] - 2))
            if(Offset > y)
            {
                uint8_t TempStack[12] = {0};
                sprintf(TempStack, "%d", (y + 1) * 4);

                String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                fwrite(String, strlen(String), 1, OutputFile);
                free(String);

                goto secondOrderValue;
            }

    //Get First Value.
    String = stringifyInstruction(5, MOVE, REGISTERS[3][0], START, TokenBuffer[Orders[Offset] - 1], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

secondOrderValue:
    for (uint8_t y = 0; y < ORDER_SIZE; y++)
        if (!(Orders[Offset] - Orders[y] + 2))
            if (Offset > y)
            {
                uint8_t TempStack[12] = { 0 };
                sprintf(TempStack, "%d", (y + 1) * 4);

                String = stringifyInstruction(7, MOVE, REGISTERS[3][3], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                fwrite(String, strlen(String), 1, OutputFile);
                free(String);

                goto calculateValue;
            }

    //Get Second Value.
    String = stringifyInstruction(5, MOVE, REGISTERS[3][3], START, TokenBuffer[Orders[Offset] + 1], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);


calculateValue:
    //Specific to Multiply and Divide.
    if (Offset == Multiply || Offset == Division)
    {
        String = stringifyInstruction(3, INSTRUCTIONS[Offset], REGISTERS[3][3], END);
        fwrite(String, strlen(String), 1, OutputFile);
        free(String);
        goto storeValue;
    }

    //Calculate the Value.
    String = stringifyInstruction(5, INSTRUCTIONS[Offset], REGISTERS[3][0], START, REGISTERS[3][3], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

storeValue:
    //Store the new Value in front of Stack.
    uint8_t ValueStack[12] = { 0 };
    sprintf(ValueStack, "%d", (Offset + 1) * 4);

    String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], MINUS, ValueStack, NEWVAREND, REGISTERS[3][0], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);
    return 0;
}

uint8_t performArithmetic(uint32_t StartLocation, void* LocalVarBuffer, uint32_t VarCount)
{
    //Startup Info.
    uint8_t* String = 0;

    LocalNameStruct ReturnVar = { 0 };
    LocalNameStruct* LocalVar = (LocalNameStruct*)LocalVarBuffer;

    //Is not a complex equation.
    if (TokenBuffer[StartLocation + 2][0] == ';')
    {
        if (isNotComplex(StartLocation, LocalVarBuffer, VarCount)) return 1;
        else return 0;
    }

    //Getting the Return Variable.
    for (uint32_t x = 0; x < VarCount; x++)
        if (!strcmp(TokenBuffer[StartLocation - 1], LocalVar[x].Name))
            ReturnVar = LocalVar[x];

    if (!ReturnVar.Name) return 1;

    //Return Variable Stack.
    uint8_t ReturnVarStack[12] = { 0 };
    sprintf(ReturnVarStack, "%d", VarCount * 4 - ReturnVar.StackOffset);

    //Loop Variables.
    uint8_t ReferencedItself = 0;
    uint32_t PrecedenceCount = 1, PrecedenceMax = 1, MaxCount = StartLocation;

    //Looping to Get Precedence and until the End of the Statement.
    do
    {
        //Getting Increase in Precedence and Checking if its max.
        if (TokenBuffer[MaxCount][0] == '(')
        {
            if (PrecedenceCount == PrecedenceMax) PrecedenceMax++;

            PrecedenceCount++;
            MaxCount++;
            continue;
        }

        //Decreasing Precedence.
        if (TokenBuffer[MaxCount][0] == ')') PrecedenceCount--;

        if (!strcmp(TokenBuffer[MaxCount], ReturnVar.Name)) ReferencedItself = 1;

        MaxCount++;
    } while (TokenBuffer[MaxCount][0] != ';');

    //Resetting Variables.
    MaxCount = StartLocation, PrecedenceCount = 1;

    //Make Sure ESI is 0.
    String = stringifyInstruction(5, MOVE, REGISTERS[3][5], START, "0 ", END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

    //Storing Value.
    if (ReferencedItself)
    {
        String = stringifyInstruction(7, MOVE, REGISTERS[3][6], VARSTART, REGISTERS[3][4], PLUS, ReturnVarStack, VAREND);
        fwrite(String, strlen(String), 1, OutputFile);
        free(String);
    }

    //Going Through Precedence.
    do
    {
        //If Reached the End, reduce Precedence.
        if (TokenBuffer[MaxCount][0] == ';')
        {
            --PrecedenceMax;
            MaxCount = StartLocation;
            continue;
        }

        //If Both are Equal.
        if (PrecedenceMax == PrecedenceCount)
        {
            uint32_t* Orders = getOrder(MaxCount);

            for (uint8_t x = 0; x < ORDER_SIZE; x++)
            {
                if (Orders[x]) writeOperation(Orders[x], Orders, x);
            }

            while (TokenBuffer[MaxCount][0] != ')' && TokenBuffer[MaxCount][0] != ';') MaxCount++;

            PrecedenceCount--;
            MaxCount++;
            continue;
        }

        //Increasing Precedence.
        if (TokenBuffer[MaxCount][0] == '(')
        {
            PrecedenceCount++;
            MaxCount++;
            continue;
        }

        //Decreasing Precedence.
        if (TokenBuffer[MaxCount][0] == ')') PrecedenceCount--;

        MaxCount++;
    } while (PrecedenceMax);

    return 0;
}
