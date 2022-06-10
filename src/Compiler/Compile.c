#include "Compile.h"
#include "WIL.h"

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

        //For comments to be ignored.
        if (Buffer[y] == '#')
        {
            while (Buffer[x] != '\n') {
                printf("%c", Buffer[x]);
                x++;
            }
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

            case ',': goto Syntax;

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
            if (Buffer[y - 1] == ' ' || Buffer[y - 1] == '(' || Buffer[y - 1] == ')' || Buffer[y - 1] == '{' || Buffer[y - 1] == '}' || Buffer[y - 1] == '\r' || Buffer[y - 1] == '\n' || Buffer[y - 1] == '\t') goto SyntaxStore;

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

    for (uint32_t x = 0; x < TokenCount; x++)
        printf("Name: %s Count: %d\n", TokenBuffer[x], x);

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
        if (TokenBuffer[x][0] == '{') Scope++;

        if (TokenBuffer[x][0] == '}') Scope--;

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

            while (TokenBuffer[x][0] != ')') x++;
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
            for (uint32_t y = 0; y < PublicVariableCount; y++)
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

    //Generate IL for Middleware.
    FILE* GamerFile = fopen("UWU.wil", "rb");
    if (GamerFile)
    {
        remove("UWU.wil");
        fclose(GamerFile);
    }

    GamerFile = fopen("C:/Users/joshm/Desktop/Pascal/UWU.wil", "ab");
    if (generateIntermediateLanguage(GamerFile)) return 10;

    return 0;
}
