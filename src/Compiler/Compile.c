//TODO: Tokenize everything. (Done)
//TODO: Check for Variables. (Done but does not have Scope)
//TODO: Design Scope. (Mostly done)
//TODO: Design Stack for local Variables. (Done)
//TODO: Create AST (Need to add Precedence).

//What to do:
//Make String ASM code more modular. (Done).
//Introduce Functions. (Done Mostly, need improve)
//Design Precedence
//Introduce Pointers.
//Introduce Arrays.
//Create a Standard.


#include "Compile.h"
#include "Arith.h"
#include "Func.h"

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


//Getting Tokens.
uint8_t** TokenBuffer = 0;
uint32_t TokenCount = 0;

//Struct for Name Referencing.
typedef struct
{
    uint8_t* Name;
    uint32_t Location;
} NameStruct;

//Sorting Tokens
NameStruct** PublicNameBuffer = 0;
uint32_t PublicFunctionCount = 0;
uint32_t PublicVariableCount = 0;

//Output File.
FILE* OutputFile = 0;

/*
⣿⡟⠙⠛⠋⠩⠭⣉⡛⢛⠫⠭⠄⠒⠄⠄⠄⠈⠉⠛⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⡇⠄⠄⠄⠄⣠⠖⠋⣀⡤⠄⠒⠄⠄⠄⠄⠄⠄⠄⠄⠄⣈⡭⠭⠄⠄⠄⠉⠙
⣿⡇⠄⠄⢀⣞⣡⠴⠚⠁⠄⠄⢀⠠⠄⠄⠄⠄⠄⠄⠄⠉⠄⠄⠄⠄⠄⠄⠄⠄
⣿⡇⠄⡴⠁⡜⣵⢗⢀⠄⢠⡔⠁⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄
⣿⡇⡜⠄⡜⠄⠄⠄⠉⣠⠋⠠⠄⢀⡄⠄⠄⣠⣆⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⢸
⣿⠸⠄⡼⠄⠄⠄⠄⢰⠁⠄⠄⠄⠈⣀⣠⣬⣭⣛⠄⠁⠄⡄⠄⠄⠄⠄⠄⢀⣿
⣏⠄⢀⠁⠄⠄⠄⠄⠇⢀⣠⣴⣶⣿⣿⣿⣿⣿⣿⡇⠄⠄⡇⠄⠄⠄⠄⢀⣾⣿
⣿⣸⠈⠄⠄⠰⠾⠴⢾⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⢁⣾⢀⠁⠄⠄⠄⢠⢸⣿⣿
⣿⣿⣆⠄⠆⠄⣦⣶⣦⣌⣿⣿⣿⣿⣷⣋⣀⣈⠙⠛⡛⠌⠄⠄⠄⠄⢸⢸⣿⣿
⣿⣿⣿⠄⠄⠄⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠇⠈⠄⠄⠄⠄⠄⠈⢸⣿⣿
⣿⣿⣿⠄⠄⠄⠘⣿⣿⣿⡆⢀⣈⣉⢉⣿⣿⣯⣄⡄⠄⠄⠄⠄⠄⠄⠄⠈⣿⣿
⣿⣿⡟⡜⠄⠄⠄⠄⠙⠿⣿⣧⣽⣍⣾⣿⠿⠛⠁⠄⠄⠄⠄⠄⠄⠄⠄⠃⢿⣿
⣿⡿⠰⠄⠄⠄⠄⠄⠄⠄⠄⠈⠉⠩⠔⠒⠉⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠐⠘⣿
⣿⠃⠃⠄⠄⠄⠄⠄⠄⣀⢀⠄⠄⡀⡀⢀⣤⣴⣤⣤⣀⣀⠄⠄⠄⠄⠄⠄⠁⢹
*/

//Instruction related Functions.
extern uint8_t* stringifyInstruction(uint8_t StringCount, ...);

uint8_t ModifyStack(uint8_t Instruction, uint8_t* Value)
{
    uint8_t MOVE[] = "mov ";
    uint8_t PUSH[] = "push ";
    uint8_t POP[] = "pop ";

    uint8_t REGISTERS[5][4] =
    {
        "eax",
        "ebx",
        "ecx",
        "edx",
        "esp"
    };

    uint8_t START[] = ", ";
    uint8_t END[] = "\n\0";

    uint8_t* String = 0;

    switch (Instruction)
    {
    case 'P':
    {
        String = stringifyInstruction(5, MOVE, REGISTERS[0], START, Value, END);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);

        String = stringifyInstruction(3, PUSH, REGISTERS[0], END);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        break;
    }

    case 'p':
    {
        String = stringifyInstruction(3, POP, REGISTERS[0], END);
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);
        break;
    }
    }
    return 0;
}

//Initialisation related.
uint8_t initFunctions()
{
    for (uint8_t x = 0; x < PublicFunctionCount; x++)
    {
        //Variable for the Location of the Function.
        uint32_t Location = PublicNameBuffer[FUNCTIONNAME][x].Location;

        //Writing the Name of the Buffer and translating it to a function in asm.
        uint8_t* String = stringifyInstruction(2, PublicNameBuffer[FUNCTIONNAME][x].Name, ":\n\0");
        fwrite(String, 1, strlen(String), OutputFile);
        free(String);

        makeFunction(Location);
    }

    return 0;
}

uint8_t initVariables()
{
    if (!OutputFile)
    {
        OutputFile = fopen("src.asm", "rb");
        if (OutputFile)
        {
            fclose(OutputFile);
            remove("src.asm");
        }

        OutputFile = fopen("src.asm", "ab");
    }

    uint8_t BSSSection[] = "Section .bss\n\0";
    fwrite(BSSSection, 1, strlen(BSSSection), OutputFile);

    for (uint32_t x = 0; x < PublicVariableCount; x++)
    {
        uint8_t VARRESB[] = ": resb ";
        uint8_t END[] = "\n\0";
        size_t Length = (uint16_t)strlen(PublicNameBuffer[VARIABLENAME][x].Name) + strlen(VARRESB) + strlen(END) + 1;

        uint8_t* Buffer = malloc(Length + 1);
        if (!Buffer) return BUFFER_INIT_ERROR;

        strcpy(Buffer, PublicNameBuffer[VARIABLENAME][x].Name);
        strcat(Buffer, VARRESB);

        if (!strcmp(TokenBuffer[PublicNameBuffer[VARIABLENAME][x].Location + 2], "u1"))
            strcat(Buffer, "1");
        else if (!strcmp(TokenBuffer[PublicNameBuffer[VARIABLENAME][x].Location + 2], "u2"))
            strcat(Buffer, "2");
        else if (!strcmp(TokenBuffer[PublicNameBuffer[VARIABLENAME][x].Location + 2], "u4"))
            strcat(Buffer, "4");

        strcat(Buffer, END);

        Buffer[Length] = '\0';
        fwrite(Buffer, 1, Length, OutputFile);
        free(Buffer);
    }

    //Set up Section .text
    uint8_t TXTSection[] = "\n\n\nSection .text\n\n\0";
    fwrite(TXTSection, 1, strlen(TXTSection), OutputFile);

    //Assign Values to Global Variables.
    for (uint32_t x = 0; x < PublicVariableCount; x++)
    {
        if (TokenBuffer[PublicNameBuffer[VARIABLENAME][x].Location + 3][0] == '=')
        {
            uint8_t MOV1[] = "mov [";
            uint8_t MOV2[] = "], ";
            uint8_t END[] = "\n\0";

            uint8_t* Buffer = stringifyInstruction(5, MOV1, PublicNameBuffer[VARIABLENAME][x].Name, MOV2, TokenBuffer[PublicNameBuffer[VARIABLENAME][x].Location + 4], END);
            fwrite(Buffer, 1, strlen(Buffer), OutputFile);
            free(Buffer);
        }
        else
        {
            uint8_t MOV1[] = "mov [";
            uint8_t MOV2[] = "], 0\n\0";

            uint8_t* Buffer = stringifyInstruction(3, MOV1, PublicNameBuffer[VARIABLENAME][x].Name, MOV2);
            fwrite(Buffer, 1, strlen(Buffer), OutputFile);
            free(Buffer);
        }
    }

    //Adding some padding between functions and init variables.
    fwrite("\n", 1, 1, OutputFile);

    //Calculating Logic of Functions.
    if (initFunctions()) return 1;

    fclose(OutputFile);
    return 0;
}

//Tokenizer.
uint8_t getTokens(uint8_t* Buffer, uint32_t Size)
{
    uint8_t Scope = 0;

    //Create Initial Token Buffer.
    TokenBuffer = malloc(sizeof(uint8_t*) * 1000);
    if (!TokenBuffer) return BUFFER_INIT_ERROR;

    for (uint32_t x = 0; x < Size;)
    {
        uint32_t y = x;

    LoopStart:

        if (Buffer[y] == ' ')
        {
            x++;
            continue;
        }

        if (Buffer[y] == '#')
        {
            while (Buffer[y] != '\n') x++;
            continue;
        }

        //Looping over non whitespace.
        while (Buffer[y] != ' ' && y < Size)
        {
            if (Buffer[y] == '\r' || Buffer[y] == '\n' || Buffer[y] == '\t') break;

            //Searching for Syntax.
            switch (Buffer[y])
            {
                //Priority.
            case '(': goto Syntax;
            case ')': goto Syntax;

            case '{': goto Syntax;
            case '}': goto Syntax;

                //Operators.
            case '+': goto Syntax;
            case '-': goto Syntax;
            case '/': goto Syntax;
            case '*': goto Syntax;

                //Bitwise
            case '&': goto Syntax;
            case '^': goto Syntax;
            case '|': goto Syntax;

                //Declaration.
            case ';': goto Syntax;
            }
            y++;
            continue;

        Syntax:
            //Get Token Before Syntax.
            if (Buffer[y - 1] == ' ' || Buffer[y - 1] == '(' || Buffer[y - 1] == '{' || Buffer[y - 1] == '\r' || Buffer[y - 1] == '\n' || Buffer[y - 1] == '\t') goto SyntaxStore;

            TokenBuffer[TokenCount] = malloc(y - x);
            if (!TokenBuffer[TokenCount]) return BUFFER_INIT_ERROR;

            for (uint32_t z = 0; z < y - x; z++)
                TokenBuffer[TokenCount][z] = Buffer[z + x];

            TokenBuffer[TokenCount++][y - x] = '\0';

        SyntaxStore:
            //Storing Syntax Token.
            TokenBuffer[TokenCount] = malloc(2);
            if (!TokenBuffer[TokenCount]) return BUFFER_INIT_ERROR;

            TokenBuffer[TokenCount][0] = Buffer[y];
            TokenBuffer[TokenCount++][1] = '\0';
            x = ++y;

            goto LoopStart;
        }

        if (Buffer[y] == '\n' || Buffer[y] == '\r' || Buffer[y] == '\t')
        {
            x = ++y;
            continue;
        }

        //Using Offset from While Loop to Store Token.
        TokenBuffer[TokenCount] = malloc(y - x + 1);
        if (!TokenBuffer[TokenCount]) return 2;

        for (uint32_t z = 0; z < y - x + 1; z++)
            TokenBuffer[TokenCount][z] = Buffer[z + x];

        TokenBuffer[TokenCount][y - x] = '\0';

        //Incrementing X to Start at Y.
        TokenCount++;
        x = ++y;
    }

    if (Scope) return 1;
    TokenBuffer[TokenCount] = 0;

    //for (uint32_t x = 0; x < TokenCount; x++)
      //  printf("Name: %s Count: %d\n", TokenBuffer[x], x);

    //printf("\n\n\nTotal Count: %d\n", TokenCount - 1);
    return 0;
}

uint8_t sortNames()
{
    PublicNameBuffer = malloc(sizeof(uint8_t*) * 2);
    if (!PublicNameBuffer) return BUFFER_INIT_ERROR;

    PublicNameBuffer[FUNCTIONNAME] = malloc(sizeof(NameStruct) * 100);
    if (!PublicNameBuffer[FUNCTIONNAME]) return BUFFER_INIT_ERROR;

    PublicNameBuffer[VARIABLENAME] = malloc(sizeof(NameStruct) * 100);
    if (!PublicNameBuffer[VARIABLENAME]) return BUFFER_INIT_ERROR;

    //Declaring Scope.
    uint8_t Scope = 0;

    for (uint8_t x = 0; x < TokenCount; x++)
    {
        if (TokenBuffer[x][0] == '{') ++Scope;
        else if (TokenBuffer[x][0] == '}') --Scope;

        //Check if it is a function.
        if (!strcmp(TokenBuffer[x], "fn\0"))
        {
            //Ensure valid name.
            uint8_t Char = TokenBuffer[x + 1][0];
            if (Char < 'A' || Char > 'z') return INVALID_NAME_CHAR;
            if (Char >= ':' && Char <= '@') return INVALID_NAME_CHAR;
            if (Char == '(' || Char == ')' || Char == '{' || Char == '}') return INVALID_NAME_CHAR;

            NameStruct Function = { TokenBuffer[x + 1], x + 1 };

            if (!PublicFunctionCount) goto AddFunction;
            //Make sure function name doesn't exist elsewhere.
            for (uint32_t y = 0; y < PublicFunctionCount; y++)
                if (!strcmp(PublicNameBuffer[FUNCTIONNAME][y].Name, TokenBuffer[x + 1])) return NAME_ELSEWHERE;

        AddFunction:
            //Add Function to List.
            PublicNameBuffer[FUNCTIONNAME][PublicFunctionCount++] = Function;
            continue;
        }

        if (Scope) continue;

        //Variable Declaration.
        if (TokenBuffer[x][0] == ':')
        {
            uint8_t DataTypes[3][3] =
            {
                "u1",
                "u2",
                "u4"
            };

            //Make sure valid name.
            uint8_t Char = TokenBuffer[x - 1][0];
            if (Char < 'A' || Char > 'z') return INVALID_NAME_CHAR;
            if (Char >= ':' && Char <= '@') return INVALID_NAME_CHAR;
            if (Char == '(' || Char == ')' || Char == '{' || Char == '}') return INVALID_NAME_CHAR;

            uint8_t Check = 0;

            //Make sure it has a datatype associated with it.
            for (uint8_t y = 0; y < 3; y++)
                if (!strcmp(DataTypes[y], TokenBuffer[x + 1]))
                    Check = 1;

            if (!Check) return 4;

            NameStruct VariableName = { TokenBuffer[x - 1], x - 1 };
            if (!PublicVariableCount) goto AddVariable;

            //Make sure Variable Name doesn't exist elsewhere.
            for (uint32_t y = 0; y < PublicFunctionCount; y++)
                if (!strcmp(PublicNameBuffer[VARIABLENAME][y].Name, TokenBuffer[x - 1])) return NAME_ELSEWHERE;

        AddVariable:
            //Add Variable to List.
            PublicNameBuffer[VARIABLENAME][PublicVariableCount++] = VariableName;
            continue;
        }
    }

    return 0;
}

uint8_t compile(const uint8_t* FileLocation, const uint8_t* OutputLocation)
{
    FILE* File = fopen(FileLocation, "rb");
    if (!File) return 1;

    //Get Total Size of File.
    fseek(File, 0, SEEK_END);
    uint32_t Size = (uint32_t)ftell(File);
    fseek(File, 0, SEEK_SET);

    //Create Buffer to hold file.
    uint8_t* Buffer = malloc(Size);
    if (!Buffer) return BUFFER_INIT_ERROR;

    fread(Buffer, 1, Size, File);

    if (OutputLocation)
    {
        OutputFile = fopen(OutputLocation, "rb");
        if (OutputFile)
        {
            fclose(OutputFile);
            remove(OutputLocation);
        }

        OutputFile = fopen(OutputLocation, "ab");
        if (!OutputFile) return 1;
    }

    if (getTokens(Buffer, Size)) return 3;
    if (sortNames()) return 9;
    if (initVariables()) return 10;
    return 0;
}
