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



//TODO: Get The Orders of All Brackets.




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

OrderStruct* getOrder(uint32_t* StartLocation)
{
    OrderStruct* Orders = calloc(ORDER_SIZE, sizeof(OrderStruct));
    if (!Orders) return 0;

    uint32_t Loop = *StartLocation;

    do
    {
        if (TokenBuffer[Loop][0] == '*')
        {

            if(!Orders[Multiply].Count)
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

inline uint8_t getOperator(uint32_t OperatorLocation)
{
    switch(TokenBuffer[OperatorLocation][0])
    {
        case '*': return Multiply;
        case '/': return Division;
        case '+': return Addition;
        case '-': return Subtraction;
    }

    return 0;
}

uint8_t writeOperation(uint32_t OperatorLocation, OrderStruct** Orders, uint8_t OrdersOffset, uint32_t OrderCount)
{
    uint8_t* String = 0;
    uint32_t OperatorBufferLocation = 0;

    //Get Buffer Location.
    for (uint32_t x = 0; x < Orders[OrdersOffset][getOperator(OperatorLocation)].Count; x++)
        if (OperatorLocation == Orders[OrdersOffset][getOperator(OperatorLocation)].Locations[x])
        {
            OperatorBufferLocation = x;
            break;
        }

    uint32_t ValueOneOperationLocation = 0, ValueOneOffset = 0, ValueTwoOperationLocation = 0, ValueTwoOffset = 0;

    if(TokenBuffer[OperatorLocation - 1][0] == ')')
    {

        for(uint32_t x = 0; x < OrderCount; x++)
            for (uint32_t y = 0; y < Orders[x][getOperator(OperatorLocation - 3)].Count; y++)
                if((OperatorLocation - 3) == Orders[x][getOperator(OperatorLocation - 3)].Locations[y])
                {
                    uint8_t TempStack[12] = { 0 };
                    sprintf(TempStack, "%d", (getOperator(OperatorLocation - 2) + 1) * 4 * (x + 1) + (y * 4));

                    String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                    fwrite(String, strlen(String), 1, OutputFile);
                    free(String);

                    ValueOneOffset = x;
                    ValueOneOperationLocation = y;
                    goto initValueTwoStart;
                }
    }

    printf("Char: %c \n", TokenBuffer[OperatorLocation - 2][0]);

    //If Operation Before comes First.
    if(getOperator(OperatorLocation - 2))
        if(getOperator(OperatorLocation - 2) < getOperator(OperatorLocation))
        {
            volatile uint32_t TempValue = Orders[OrdersOffset][getOperator(OperatorLocation - 2)].Count;

            //Fix this garbage thanks x
            for (uint32_t z = 0; z < Orders[OrdersOffset][getOperator(OperatorLocation - 2)].Count; z++)
                if (OperatorLocation - 2 == Orders[OrdersOffset][getOperator(OperatorLocation - 2)].Locations[z])
                {
                    uint8_t TempStack[12] = { 0 };
                    sprintf(TempStack, "%d", (getOperator(OperatorLocation - 2) + 1) * 4 * (OrdersOffset + 1) + (z * 4));

                    String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                    fwrite(String, strlen(String), 1, OutputFile);
                    free(String);

                    goto initValueTwoStart;
                }
        }

    String = stringifyInstruction(5, MOVE, REGISTERS[3][0], START, TokenBuffer[OperatorLocation - 1], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

initValueTwoStart:
    if (TokenBuffer[OperatorLocation + 1][0] == '(')
    {
        for (uint32_t x = 0; x < OrderCount; x++)
            for (uint32_t y = 0; y < Orders[x][getOperator(OperatorLocation + 3)].Count; y++)
                if ((OperatorLocation + 3) == Orders[x][getOperator(OperatorLocation + 3)].Locations[y])
                {
                    uint8_t TempStack[12] = { 0 };
                    sprintf(TempStack, "%d", (getOperator(OperatorLocation + 2) + 1) * 4 * x + (y * 4));

                    String = stringifyInstruction(7, MOVE, REGISTERS[3][3], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                    fwrite(String, strlen(String), 1, OutputFile);
                    free(String);

                    goto beginCalculation;

                    ValueTwoOffset = x;
                    ValueTwoOperationLocation = y;
                    goto beginCalculation;
                }
    }

    //If Operation After comes First.
    if(getOperator(OperatorLocation + 2))
        if(getOperator(OperatorLocation + 2) < getOperator(OperatorLocation))
        {
            for (uint32_t z = 0; z < Orders[OrdersOffset][getOperator(OperatorLocation + 2)].Count; z++)
                if (OperatorLocation + 2 == Orders[OrdersOffset][getOperator(OperatorLocation + 2)].Locations[z])
                {
                    uint8_t TempStack[12] = { 0 };
                    sprintf(TempStack, "%d", (getOperator(OperatorLocation + 2) + 1) * 4 * OrdersOffset + (z * 4));

                    String = stringifyInstruction(7, MOVE, REGISTERS[3][3], VARSTART, REGISTERS[3][4], MINUS, TempStack, VAREND);
                    fwrite(String, strlen(String), 1, OutputFile);
                    free(String);

                    goto beginCalculation;
                }
        }

    String = stringifyInstruction(5, MOVE, REGISTERS[3][3], START, TokenBuffer[OperatorLocation + 1], END);
    fwrite(String, strlen(String), 1, OutputFile);
    free(String);

beginCalculation:
    String = stringifyInstruction(5, INSTRUCTIONS[getOperator(OperatorLocation)], REGISTERS[3][0], START, REGISTERS[3][3], END);
    fwrite(String, strlen(String), 1, OutputFile);
    
    printf("String: %s\n", String);
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
    uint32_t PrecedenceCount = 0, PrecedenceMax = 0, MaxCount = StartLocation;
    
    //For Use with GetOrder();
    uint32_t OrderCount = 0;

    //Allocating Memory for OrderCount.
    uint32_t OrderNum = 0;
    OrderStruct** OrderBuffer = calloc(OrderCount, sizeof(OrderStruct**));

    if (!OrderBuffer) return 1;

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
        if (TokenBuffer[MaxCount][0] == ')') PrecedenceCount--;

        if (!strcmp(TokenBuffer[MaxCount], ReturnVar.Name)) ReferencedItself = 1;

        MaxCount++;
    } while (TokenBuffer[MaxCount][0] != ';');

    //Resetting Variables.
    MaxCount = StartLocation, PrecedenceCount = 0;

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
            PrecedenceCount = 1;
            continue;
        }

        //If Both are Equal.
        if (PrecedenceMax == PrecedenceCount)
        {
            //uint32_t* Orders = getOrder(&MaxCount);

            OrderBuffer[OrderNum++] = getOrder(&MaxCount);

            for (uint8_t x = 0; x < ORDER_SIZE; x++)
            {

                if(OrderBuffer[OrderNum - 1][x].Count)
                    for(uint32_t z = 0; z < OrderBuffer[OrderNum - 1][x].Count; z++)
                        writeOperation(OrderBuffer[OrderNum - 1][x].Locations[z], OrderBuffer, OrderNum - 1, OrderCount);

                //if (Orders[x])
                //{
                    //writeOperation(Orders[x], Orders, x);
                //}
            }

            //free(Orders);
            while (TokenBuffer[MaxCount][0] != ')' && TokenBuffer[MaxCount][0] != ';') MaxCount++;
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

        MaxCount++;
    } while (PrecedenceMax && TokenBuffer[MaxCount][0] != ';');

    return 0;
}
