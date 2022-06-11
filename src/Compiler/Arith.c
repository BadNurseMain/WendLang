#include "Arith.h"
#include "Func.h"

#ifndef GLOBALS
#define GLOBALS

#include "ArithGlobals.h"

#endif

//----------------------------------------------------------- INIT ------------------------------------------------------

#ifndef ERR_DBG
#define ERR_DBG

#pragma warning(disable : 6386)
#pragma warning(disable : 6385)
#pragma warning(disable : 4996)

//Definitions for NameBuffer.
#define FUNCTIONNAME (uint8_t)0
#define VARIABLENAME (uint8_t)1

//Error Codes.
#define INVALID_NAME_CHAR (uint8_t)2
#define NAME_ELSEWHERE (uint8_t)3
#define BUFFER_INIT_ERROR (uint8_t)4
#define NO_VALID_VARIABLE (uint8_t)5

#endif


#define ORDER_SIZE (uint8_t)20
#define OPERATOR_ERROR (uint8_t) ~1


//IntermediateFile.
extern FILE* IntermediateFile;

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

    //Bitwise.
    BitwiseAND = 5,
    BitwiseOR = 6,
    BitwiseXOR = 7,

    BitwiseSHL = 8,
    BitwiseSHR = 9,

    //Control flow.
    LessThan = 10,
    GreaterThan = 11,
    Equality = 12
};

uint8_t PublicTabOffset = 0;

//TODO: Write Cases for Other Operators, including
//      bitwise, functions and pointers.            (Done bitwise and function calls.)

extern uint16_t getFunctionParams(uint32_t Location);

void createTabOffset()
{
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    return;
}


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

        if (TokenBuffer[Loop][0] == '|')
        {

            if (!Orders[BitwiseOR].Count)
                Orders[BitwiseOR].Locations = calloc(10, sizeof(uint32_t));

            if (!Orders[BitwiseOR].Locations) return 0;
            Orders[BitwiseOR].Locations[Orders[BitwiseOR].Count++] = Loop;
        }

        if (TokenBuffer[Loop][0] == '&')
        {

            if (!Orders[BitwiseAND].Count)
                Orders[BitwiseAND].Locations = calloc(10, sizeof(uint32_t));

            if (!Orders[BitwiseAND].Locations) return 0;
            Orders[BitwiseAND].Locations[Orders[BitwiseAND].Count++] = Loop;
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

        //Bitwise.
    case '&': return BitwiseAND;
    case '|': return BitwiseOR;

        //Bit Shifting.
    case '<':
    {
        if (TokenBuffer[OperatorLocation][1] == '<') return BitwiseSHL;
        else return GreaterThan;
    }

    case '>':
    {
        if (TokenBuffer[OperatorLocation][1] == '>') return BitwiseSHR;
        else return LessThan;
    }
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
                for (uint8_t x = 0; x < PublicTabOffset; x++)
                    fwrite("\t", 1, 1, IntermediateFile);

                fwrite(String, strlen(String), 1, IntermediateFile);
                free(String);
                return 0;
            }
    }

    return 1;
}

uint32_t getLocalVariable(uint32_t Location, LocalNameStruct* LocalVarBuffer, uint32_t LocalVarCount)
{
    for (uint32_t x = 0; x < LocalVarCount; x++)
        if (!strcmp(TokenBuffer[Location], LocalVarBuffer[x].Name)) return x + 1;

    return 0;
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

    //Writing Original Equation.
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    if (TokenBuffer[StartLocation - 2][0] == ':')
    {
        String = stringifyInstruction(5, "; ", TokenBuffer[StartLocation - 3], " : ", TokenBuffer[StartLocation - 1], " {\0");
        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
    }
    else
    {
        uint8_t VariableType[3] = { 0 };

        switch (ReturnVar.Type)
        {
        case 1:
            strcpy(VariableType, "u1");
            break;

        case 2:
            strcpy(VariableType, "u2");
            break;

        case 3:
            strcpy(VariableType, "u4");
            break;
        }

        String = stringifyInstruction(5, "; ", ReturnVar.Name, " : ", VariableType, "{\0");
        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
    }

    for (uint32_t x = StartLocation + 1; TokenBuffer[x][0] != ';'; x++)
        fwrite(TokenBuffer[x], 1, strlen(TokenBuffer[x]), IntermediateFile);

    fwrite("}\n\0", 1, 2, IntermediateFile);

    //Checking if it is a Number.
    if (TokenBuffer[StartLocation + 1][0] >= '0' && TokenBuffer[StartLocation + 1][0] <= '9')
    {
        String = stringifyInstruction(5, MOVE, REGISTERS[3][0], START, TokenBuffer[StartLocation + 1], END);

        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, 1, strlen(String), IntermediateFile);
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
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
        goto assignReturn;
    }

    //Is a Function.
    for (uint32_t x = 0; x < PublicFunctionCount; x++)
        if (!strcmp(TokenBuffer[StartLocation + 1], PublicNameBuffer[FUNCTIONNAME][x].Name))
        {
            if (TokenBuffer[StartLocation + 2][0] != '(') return 1;
            uint16_t ParamCount = getFunctionParams(StartLocation + 1);

            if (ParamCount)
            {
                uint8_t** Buffer = malloc(sizeof(uint8_t*) * ParamCount);
                if (!Buffer) return 1;

                for (uint16_t y = 0; y < ParamCount; y++)
                    Buffer[y] = TokenBuffer[StartLocation + 3 + (y * 2)];

                callFunction(PublicNameBuffer[FUNCTIONNAME][x].Name, 1, ParamCount, Buffer);
                goto assignReturn;
            }

            callFunction(PublicNameBuffer[FUNCTIONNAME][x].Name, 1, 0, 0);
            goto assignReturn;
        }

    return 1;

    //Assigning the Values from the Returns.
assignReturn:
    if (Type == ARTH_TYPE_STACK)
    {
        String = stringifyInstruction(3, PUSH, REGISTERS[3][0], END);
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);

        //Adding Spacing.
        fwrite("\n", 1, 1, IntermediateFile);
        return 0;
    }

    uint8_t ReturnStack[12] = { 0 };
    sprintf(ReturnStack, "%d", LocalVar[VarCount - 1].StackOffset - ReturnVar.StackOffset);

    String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], PLUS, ReturnStack, NEWVAREND, REGISTERS[3][0], END);
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    fwrite(String, 1, strlen(String), IntermediateFile);
    free(String);

    //Adding Spacing.
    fwrite("\n", 1, 1, IntermediateFile);
    return 0;
}

uint8_t writeOperation(uint32_t OperatorLocation, OrderStruct** Orders, uint32_t OrderCount, LocalNameStruct* LocalVarBuffer, uint32_t LocalVarCount, uint8_t Type)
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
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
        goto initValueTwoStart;
    }

    VariableLocation = 0;

    //Just a constant.
    String = stringifyInstruction(5, MOVE, REGISTERS[Type][0], START, TokenBuffer[OperatorLocation - 1], END);
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    fwrite(String, strlen(String), 1, IntermediateFile);
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
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
        goto beginCalculation;
    }

    //Is a Function.
    for (uint32_t x = 0; x < PublicFunctionCount; x++)
        if (!strcmp(TokenBuffer[OperatorLocation + 1], PublicNameBuffer[FUNCTIONNAME][x].Name))
        {
            if (TokenBuffer[OperatorLocation + 2][0] != '(') return 1;
            uint16_t ParamCount = getFunctionParams(OperatorLocation + 1);

            if (ParamCount)
            {
                uint8_t** Buffer = malloc(sizeof(uint8_t*) * ParamCount);
                if (!Buffer) return 1;

                for (uint16_t y = 0; y < ParamCount; y++)
                    Buffer[y] = TokenBuffer[OperatorLocation + 3 + (y * 2)];

                callFunction(PublicNameBuffer[FUNCTIONNAME][x].Name, 4, ParamCount, Buffer);
                goto beginCalculation;
            }

            callFunction(PublicNameBuffer[FUNCTIONNAME][x].Name, 4, 0, 0);
            goto beginCalculation;
        }

    //Just a constant.
    String = stringifyInstruction(5, MOVE, REGISTERS[Type][3], START, TokenBuffer[OperatorLocation + 1], END);
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    fwrite(String, strlen(String), 1, IntermediateFile);
    free(String);

beginCalculation:
    if (OperationType == Multiply || OperationType == Division)
    {
        String = stringifyInstruction(3, INSTRUCTIONS[OperationType], REGISTERS[Type][3], END);
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, strlen(String), 1, IntermediateFile);
        free(String);
    }
    else
    {
        String = stringifyInstruction(5, INSTRUCTIONS[OperationType], REGISTERS[Type][0], START, REGISTERS[Type][3], END);
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, strlen(String), 1, IntermediateFile);
        free(String);
    }

    uint32_t Value = getTempOffset(Orders, OrderCount, OperatorLocation);
    uint8_t TempStack[12] = { 0 };
    sprintf(TempStack, "%d", Value * 4);

    String = stringifyInstruction(8, MOVE, NEWVARSTART, REGISTERS[3][4], MINUS, TempStack, NEWVAREND, REGISTERS[3][0], END);
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    fwrite(String, strlen(String), 1, IntermediateFile);
    free(String);
    return 0;
}

uint32_t writeArithmeticOperations(uint8_t TabOffset, uint32_t StartLocation, LocalNameStruct* Variables, uint32_t VariableCount, uint8_t OptionalParam)
{
    PublicTabOffset = TabOffset;

    uint8_t* String = 0;

    //Not a Complex Equation.
    if (TokenBuffer[StartLocation + 2][0] == ';')
    {
        if (isNotComplex(StartLocation, Variables, VariableCount, OptionalParam)) return 1;
        else return StartLocation + 3;;
    }

    //Ensuring Return is Valid.
    LocalNameStruct ReturnStruct = { 0 };

    for (uint32_t x = 0; x < VariableCount; x++)
        if (!strcmp(TokenBuffer[StartLocation - 3], Variables[x].Name))
            ReturnStruct = Variables[x];

    if (!ReturnStruct.Name) return ARTH_UNKNOWN_VAR;

    //For writing out Original Equation.
    for (uint8_t x = 0; x < TabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    if (TokenBuffer[StartLocation - 2][0] == ':')
    {
        String = stringifyInstruction(5, "; ", TokenBuffer[StartLocation - 3], " : ", TokenBuffer[StartLocation - 1], " {\0");
        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
    }
    else
    {
        String = stringifyInstruction(5, "; ", ReturnStruct.Name, " : ", ReturnStruct.Type, "{\0");
        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
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

        fwrite(TokenBuffer[MaxCount], 1, strlen(TokenBuffer[MaxCount]), IntermediateFile);
        MaxCount++;
    } while (TokenBuffer[MaxCount][0] != ';');

    fwrite("}\n", 1, 2, IntermediateFile);

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
                        writeOperation(OrderBuffer[OrderNum - 1][x].Locations[z], OrderBuffer, OrderCount, Variables, VariableCount, ReturnStruct.Type);
            continue;
        }

        MaxCount++;
    } while (PrecedenceMax || TokenBuffer[MaxCount][0] != ';');

    if (OptionalParam == ARTH_TYPE_STACK)
    {
        String = stringifyInstruction(4, PUSH, REGISTERS[3][0], "\n", END);
        for (uint8_t x = 0; x < PublicTabOffset; x++)
            fwrite("\t", 1, 1, IntermediateFile);

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);
        goto cleanup;
    }

    //If No Conditions are Specified.
    uint8_t TempStack[12] = { 0 };
    sprintf(TempStack, "%d", (Variables[VariableCount - 1].StackOffset * 4) - (ReturnStruct.StackOffset * 4));

    String = stringifyInstruction(9, MOVE, NEWVARSTART, REGISTERS[3][4], PLUS, TempStack, NEWVAREND, REGISTERS[3][0], "\n", END);
    for (uint8_t x = 0; x < PublicTabOffset; x++)
        fwrite("\t", 1, 1, IntermediateFile);

    fwrite(String, 1, strlen(String), IntermediateFile);
    free(String);

cleanup:
    //Cleanup.
    for (uint8_t x = 0; x < OrderCount; x++)
    {
        for (uint8_t y = 0; y < ORDER_SIZE; y++)
            free(OrderBuffer[x][y].Locations);

        free(OrderBuffer[x]);
    }

    free(OrderBuffer);

    //Adding Spacing.
    fwrite("\n", 1, 1, IntermediateFile);
    return MaxCount;
}

//------------------------------------ CONDITIONALS -------------------------------------------------
extern uint8_t getVariableSize(const uint8_t* Type);

uint8_t getConditionalOperator(uint8_t* Operator)
{
    switch (Operator[0])
    {
    case '<': return LessThan;
    case '>': return GreaterThan;
    case '=':
    {
        if (Operator[1] == '=') return Equality;
    }


    }
    return 0;
}

uint32_t writeConditionalOperations(uint8_t* FunctionName, uint32_t StartLocation, LocalNameStruct* Variables, uint32_t VariableCount, uint32_t ConditionalCount, uint8_t OptionalParam)
{
    uint8_t* String;

    //Jump Related Stuff.
    uint8_t ConditionalBuffer[20] = { 0 }, JumpName[64] = { 0 };

    strcpy(JumpName, FunctionName);

    sprintf(ConditionalBuffer, "%d", ConditionalCount);
    strcat(JumpName, ConditionalBuffer);

    uint32_t Loop = StartLocation;

    //Write Original IF Statement.
    do
    {
        Loop++;
    } while (TokenBuffer[Loop][0] != '{');

    //Writing Original.
    createTabOffset();
    fwrite("; ", 1, 2, IntermediateFile);

    for (uint32_t x = StartLocation - 1; x < Loop; x++)
        fwrite(TokenBuffer[x], 1, strlen(TokenBuffer[x]), IntermediateFile);

    fwrite("\n\0", 1, 1, IntermediateFile);

    //Not Complex.
    if (getConditionalOperator(TokenBuffer[StartLocation + 2]))
    {
        uint32_t VariableLocation = getLocalVariable(StartLocation + 1, Variables, VariableCount);
        if (VariableLocation)
        {
            //Stringify Stack Offset.
            uint8_t StackOffset[12] = { 0 };
            sprintf(StackOffset, "%d", Variables[VariableCount - 1].StackOffset - Variables[VariableLocation - 1].StackOffset);

            String = stringifyInstruction(7, MOVE, REGISTERS[3][0], VARSTART, REGISTERS[3][4], PLUS, StackOffset, VAREND);
            createTabOffset();

            fwrite(String, 1, strlen(String), IntermediateFile);
            free(String);
            goto notComplexAssignTwo;
        }

        //Is just a constant.
        String = stringifyInstruction(5, MOVE, REGISTERS[3][0], START, TokenBuffer[StartLocation + 1], END);
        createTabOffset();

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);

    notComplexAssignTwo:
        VariableLocation = getLocalVariable(StartLocation + 3, Variables, VariableCount);
        if (VariableLocation)
        {
            //Stringify Stack Offset.
            uint8_t StackOffset[12] = { 0 };
            sprintf(StackOffset, "%d", Variables[VariableCount - 1].StackOffset - Variables[VariableLocation - 1].StackOffset);

            String = stringifyInstruction(7, MOVE, REGISTERS[3][3], VARSTART, REGISTERS[3][4], PLUS, StackOffset, VAREND);
            createTabOffset();

            fwrite(String, 1, strlen(String), IntermediateFile);
            free(String);
            goto notComplexCompare;
        }

        String = stringifyInstruction(5, MOVE, REGISTERS[3][3], START, TokenBuffer[StartLocation + 3], END);
        createTabOffset();

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);

    notComplexCompare:
        String = stringifyInstruction(5, "cmp ", REGISTERS[3][0], START, REGISTERS[3][3], END);
        createTabOffset();

        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);

        uint8_t Value = getConditionalOperator(TokenBuffer[StartLocation + 2]);

        if (Value == LessThan)
        {
            String = stringifyInstruction(4, INSTRUCTIONS[GreaterThan], JumpName, "\n", END);
            createTabOffset();

            fwrite(String, 1, strlen(String), IntermediateFile);
            free(String);
        }

        if (Value == GreaterThan)
        {
            String = stringifyInstruction(4, INSTRUCTIONS[LessThan], JumpName, "\n", END);
            createTabOffset();

            fwrite(String, 1, strlen(String), IntermediateFile);
            free(String);
        }

        uint32_t ReturnValue = writeFunctionInstructions(FunctionName, StartLocation + 5, Variables, VariableCount, ConditionalCount);

        String = stringifyInstruction(3, JumpName, ": ", END);
        fwrite(String, 1, strlen(String), IntermediateFile);
        free(String);

        return ReturnValue;
    }

    return 0;
}
