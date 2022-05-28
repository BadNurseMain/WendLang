#include "Arith.h"
#include "ArithGlobals.h"
#include "Func.h"

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
#define OPERATOR_ERROR (uint8_t) ~1


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

//Storing Local Variables.
typedef struct
{
    uint32_t Count;
    uint32_t* Locations;
} OrderStruct;

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

//TODO: Fix getPosition so that its actually useful
//      which in turn will fix Offsetting.

//TODO: Write Cases for Other Operators, including
//      bitwise, functions and pointers.

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

OrderStruct* getOrder(uint32_t* StartLocation)
{
    uint32_t Loop = *StartLocation;
    OrderStruct* Orders = calloc(ORDER_SIZE, sizeof(OrderStruct));
    if (!Orders) return 0;

    do
    {
        if (TokenBuffer[Loop][0] == '*')
        {

            if (!Orders[Multiply].Count)
                Orders[Multiply].Locations = calloc(10, sizeof(uint32_t));

            if (!Orders[Multiply].Locations) return 0;
            Orders[Multiply].Locations[Orders[Multiply].Count++] = Loop;
        }

        if (TokenBuffer[Loop][0] == '+')
        {

            if (!Orders[Addition].Count)
                Orders[Addition].Locations = calloc(10, sizeof(uint32_t));

            if (!Orders[Addition].Locations) return 0;
            Orders[Addition].Locations[Orders[Addition].Count++] = Loop;
        }

        if (TokenBuffer[Loop][0] == '-')
        {

            if (!Orders[Subtraction].Count)
                Orders[Subtraction].Locations = calloc(10, sizeof(uint32_t));

            if (!Orders[Subtraction].Locations) return 0;
            Orders[Subtraction].Locations[Orders[Subtraction].Count++] = Loop;
        }

        if (TokenBuffer[Loop][0] == '/')
        {

            if (!Orders[Division].Count)
                Orders[Division].Locations = calloc(10, sizeof(uint32_t));

            if (!Orders[Division].Locations) return 0;
            Orders[Division].Locations[Orders[Division].Count++] = Loop;
        }

        ++Loop;
    } while (TokenBuffer[Loop][0] != ')' && TokenBuffer[Loop][0] != ';' && TokenBuffer[Loop][0] != '(');

    //Change Loop Location.
    *StartLocation = Loop;
    return Orders;
}

uint8_t getOperator(uint32_t OperatorLocation)
{
    switch (TokenBuffer[OperatorLocation][0])
    {
    case '*': return Multiply;
    case '/': return Division;
    case '+': return Addition;
    case '-': return Subtraction;
    }

    return OPERATOR_ERROR;
}

uint32_t getTempOffset(OrderStruct** Orders, uint32_t OrderCount, uint32_t Position)
{
    uint32_t Value = 0;

    for (uint32_t x = 0; x < OrderCount; x++)
        for (uint32_t y = 0; y < ORDER_SIZE; y++)
            for (uint32_t z = 0; z < Orders[x][y].Count; z++)
                if (Position == Orders[x][y].Locations[z]) return ++Value;
                else ++Value;

    return 0;
}

uint32_t getLocalVariable(uint32_t Location, LocalNameStruct* LocalVarBuffer, uint32_t LocalVarCount)
{
    for (uint32_t x = 0; x < LocalVarCount; x++)
        if (!strcmp(TokenBuffer[Location], LocalVarBuffer[x].Name)) return x + 1;

    return 0;
}

uint8_t getTempValue(uint8_t ValueNumber, OrderStruct** Orders, uint32_t OrderCount, uint32_t OperatorLocation, int8_t OperatorOffset)
{
    uint8_t* String = 0;
    uint8_t Operation = getOperator(OperatorLocation + OperatorOffset);

    uint32_t OrderLocation = 0;

    for (uint32_t x = 0; x < OrderCount; x++)
    {
        for (uint32_t y = 0; y < Orders[x][Operation].Count; y++)
            if ((OperatorLocation + OperatorOffset) == Orders[x][Operation].Locations[y])
            {
                uint32_t Value = getTempOffset(Orders, OrderCount, OperatorLocation + OperatorOffset);

                uint8_t TempStack[12] = { 0 };
                sprintf(TempStack, "%d", Value * 4);

                String = stringifyInstruction(7, MOVE, REGISTERS[3][ValueNumber], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                fwrite(String, strlen(String), 1, OutputFile);
                free(String);
                return 0;
            }
    }

    return 1;
}

uint8_t isNotComplex(uint32_t StartLocation, void* LocalVarBuffer, uint32_t VarCount, uint8_t Type)
{
    //Setup.
    uint8_t* String = 0;
    LocalNameStruct* LocalVar = (LocalNameStruct*)LocalVarBuffer;

    LocalNameStruct ReturnVar = { 0 };

    uint32_t Offset = 1;

    if (TokenBuffer[StartLocation - 2][0] == ':') Offset = 3;

    //Getting the Return Variable.
    for (uint32_t x = 0; x < VarCount; x++)
    {
        if (!strcmp(TokenBuffer[StartLocation - Offset], LocalVar[x].Name))
            ReturnVar = LocalVar[x];
    }


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
    for (uint32_t x = 0; x < PublicVariableCount; x++)
        if (!strcmp(TokenBuffer[StartLocation + 1], PublicNameBuffer[1][x].Name))
        {

        }


    uint32_t VariableLocation = getLocalVariable(StartLocation + 1, LocalVar, VarCount);

    //Is a Local Variable.
    if (VariableLocation)
    {
        //Stringify Stack Offset.
        uint8_t StackOffset[12] = { 0 };
        sprintf(StackOffset, "%d", LocalVar[VariableLocation - 1].StackOffset);

        String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], PLUS, StackOffset, VAREND);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        goto assignReturn;
    }

    return 1;

    //Assigning the Values from the Returns.
assignReturn:
    if (Type == 1)
    {
        String = stringifyInstruction(3, PUSH, REGISTERS[3][0], END);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        return 0;
    }

    uint8_t ReturnStack[12] = { 0 };
    sprintf(ReturnStack, "%d", (VarCount - 1 - ReturnVar.StackOffset) * 4);

    String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], PLUS, ReturnStack, NEWVAREND, REGISTERS[3][0], END);
    fwrite(String, 1, strlen(String), OutputFile);
    free(String);
    return 0;
}

uint8_t writeOperation(uint32_t OperatorLocation, OrderStruct** Orders, uint8_t OrdersOffset, uint32_t OrderCount, LocalNameStruct* LocalVarBuffer, uint32_t LocalVarCount)
{
    uint8_t* String = 0;
    uint32_t OperationType = getOperator(OperatorLocation), VariableLocation = 0;

    //Has Brackets Before.
    if (TokenBuffer[OperatorLocation - 1][0] == ')')
    {
        getTempValue(0, Orders, OrderCount, OperatorLocation, -3);
        goto initValueTwoStart;
    }

    //Has an Operation Right Before.
    if (getOperator(OperatorLocation - 2) != OPERATOR_ERROR)
        if (getOperator(OperatorLocation - 2) < OperationType)
        {
            getTempValue(0, Orders, OrderCount, OperatorLocation, -2);
            goto initValueTwoStart;
        }

    VariableLocation = getLocalVariable(OperatorLocation - 1, LocalVarBuffer, LocalVarCount);

    //Is a Local Variable.
    if (VariableLocation)
    {
        //Stringify Stack Offset.
        uint8_t StackOffset[12] = { 0 };
        sprintf(StackOffset, "%d", LocalVarBuffer[VariableLocation - 1].StackOffset);
        
        String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], PLUS, StackOffset, VAREND);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        goto initValueTwoStart;
    }

    VariableLocation = 0;

    //Just a constant.
    String = stringifyInstruction(5, MOVE, REGISTERS[3][0], START, TokenBuffer[OperatorLocation - 1], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

initValueTwoStart:
    //Has Brackets in Front.
    if (TokenBuffer[OperatorLocation + 1][0] == '(')
    {
        getTempValue(3, Orders, OrderCount, OperatorLocation, 3);
        goto beginCalculation;
    }

    //Has an Operation Straight After.
    if (getOperator(OperatorLocation + 2))
        if (getOperator(OperatorLocation + 2) < OperationType)
        {
            getTempValue(3, Orders, OrderCount, OperatorLocation, 2);
            goto beginCalculation;
        }

    VariableLocation = getLocalVariable(OperatorLocation + 1, LocalVarBuffer, LocalVarCount);

    //Is a Local Variable.
    if (VariableLocation)
    {

        //Stringify Stack Offset.
        uint8_t StackOffset[12] = { 0 };
        sprintf(StackOffset, "%d", LocalVarBuffer[VariableLocation - 1].StackOffset);

        String = stringifyInstruction(7, MOVE, REGISTERS[3][3], VARSTART, REGISTERS[3][4], PLUS, StackOffset, VAREND);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        goto beginCalculation;
    }

    VariableLocation = 0;

    //Just a constant.
    String = stringifyInstruction(5, MOVE, REGISTERS[3][3], START, TokenBuffer[OperatorLocation + 1], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

beginCalculation:
    if (OperationType == Multiply || OperationType == Division)
    {
        String = stringifyInstruction(3, INSTRUCTIONS[OperationType], REGISTERS[3][3], END);
        fwrite(String, strlen(String), 1, OutputFile);
        free(String);
    }
    else
    {
        String = stringifyInstruction(5, INSTRUCTIONS[OperationType], REGISTERS[3][0], START, REGISTERS[3][3], END);
        fwrite(String, strlen(String), 1, OutputFile);
        free(String);
    }

    uint32_t Value = getTempOffset(Orders, OrderCount, OperatorLocation);
    uint8_t TempStack[12] = { 0 };
    sprintf(TempStack, "%d", Value * 4);

    String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], MINUS, TempStack, NEWVAREND, REGISTERS[3][0], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);
    return 0;
}

uint32_t complexArith(uint32_t StartLocation, LocalNameStruct* Variables, uint32_t VariableCount, uint8_t OptionalParam)
{
    uint8_t* String = 0;

    //Not a Complex Equation.
    if (TokenBuffer[StartLocation + 2][0] == ';')
    {
        if (isNotComplex(StartLocation, Variables, VariableCount, OptionalParam)) return 1;
        else return StartLocation + 3;;
    }

    //Loop Variables.
    uint8_t ReferencedItself = 0;
    uint32_t PrecedenceCount = 0, PrecedenceMax = 0, MaxCount = StartLocation;

    //For Use with GetOrder();
    uint32_t OrderCount = 1;

    if (TokenBuffer[++MaxCount][0] == '(') OrderCount--;

    //Looping to Get Precedence and until the End of the Statement.
    do
    {
        //Getting Increase in Precedence and Checking if its max.
        if (TokenBuffer[MaxCount][0] == '(')
        {
            if (PrecedenceCount == PrecedenceMax) PrecedenceMax++;

            PrecedenceCount++;
            MaxCount++;
            OrderCount++;
            continue;
        }

        //Decreasing Precedence.
        if (TokenBuffer[MaxCount][0] == ')')
        {
            if (TokenBuffer[MaxCount + 1][0] != ';') OrderCount++;
            PrecedenceCount--;
        }

        MaxCount++;
    } while (TokenBuffer[MaxCount][0] != ';');

    //Allocating Memory for OrderCount.
    uint32_t OrderNum = 0;

    OrderStruct** OrderBuffer = calloc(OrderCount + 1, sizeof(OrderStruct**));
    if (!OrderBuffer) return 1;

    //Resetting Variables.
    MaxCount = StartLocation + 1, PrecedenceCount = 0;

    //Going Through Precedence.
    do
    {
        //If Reached the End, reduce Precedence.
        if (TokenBuffer[MaxCount][0] == ';')
        {
            --PrecedenceMax;
            MaxCount = StartLocation + 1;
            PrecedenceCount = 0;
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
        if (TokenBuffer[MaxCount][0] == ')')
        {
            PrecedenceCount--;
            MaxCount++;
            continue;
        }

        //If Both are Equal.
        if (PrecedenceMax == PrecedenceCount)
        {
            OrderBuffer[OrderNum++] = getOrder(&MaxCount);

            for (uint8_t x = 0; x < ORDER_SIZE; x++)
                if (OrderBuffer[OrderNum - 1][x].Count)
                    for (uint32_t z = 0; z < OrderBuffer[OrderNum - 1][x].Count; z++)
                        writeOperation(OrderBuffer[OrderNum - 1][x].Locations[z], OrderBuffer, OrderNum - 1, OrderCount, Variables, VariableCount);
            continue;
        }

        MaxCount++;
    } while (PrecedenceMax || TokenBuffer[MaxCount][0] != ';');



    if (OptionalParam == 1)
    {
        String = stringifyInstruction(3, PUSH, REGISTERS[3][0], END);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        return MaxCount;
    }

    for (uint32_t x = 0; x < VariableCount; x++)
        if (!strcmp(TokenBuffer[StartLocation - 1], Variables[x].Name))
        {
            uint8_t TempStack[12] = { 0 };
            sprintf(TempStack, "%d", (Variables[VariableCount - 1].StackOffset * 4) - (Variables[x].StackOffset * 4));

            String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], PLUS, TempStack, NEWVAREND, REGISTERS[3][0], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);
            return MaxCount;
        }

    //Cleanup.
    for (uint8_t x = 0; x < OrderCount; x++)
    {
        for (uint8_t y = 0; y < ORDER_SIZE; y++)
            free(OrderBuffer[x][y].Locations);

        free(OrderBuffer[x]);
    }

    free(OrderBuffer);
    return 0;
}
