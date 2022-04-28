//TODO: Tokenize everything. (Done)
//TODO: Check for Variables. (Done but does not have Scope)
//TODO: Design Scope.
//TODO: Design Stack for local Variables.
//TODO: Create AST
//TODO: Design Assembler

#include "Compile.h"

//Getting Tokens.
uint8_t** TokenBuffer = 0;
uint32_t TokenCount = 0;

#define FUNCTIONNAME (uint8_t)0
#define VARIABLENAME (uint8_t)1

//Error Codes.
#define INVALID_NAME_CHAR (uint8_t)4
#define NAME_ELSEWHERE (uint8_t)5

//Struct for Name Referencing.
typedef struct
{
    uint8_t* Name;
    uint32_t Location;
} NameStruct;

//Sorting Tokens 
NameStruct** NameBuffer = 0;
uint32_t FunctionCount = 0;
uint32_t VariableCount = 0;


//Output File.
FILE* OutputFile = 0;

uint8_t getTokens(uint8_t* Buffer, uint32_t Size)
{
    //Create Initial Token Buffer.
    TokenBuffer = malloc(sizeof(uint8_t*) * 1000);
    if(!TokenBuffer) return 1;

    for(uint32_t x = 0; x < Size;)
    {
        uint32_t y = x;
        
        LoopStart:
        
        if (Buffer[y] == ' ')
        {
            x++;
            continue;
        }

        //Looping over non whitespace.
        while (Buffer[y] != ' ' && y < Size)
        {
            if (Buffer[y] == '\r' || Buffer[y] == '\n') break;

            //Searching for Syntax.
            switch(Buffer[y])
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

                //Declaration.
                case ';': goto Syntax;
            }
            y++;
            continue;
        
        Syntax:
            //Get Token Before Syntax.
            if (Buffer[y - 1] == ' ') goto SyntaxStore;

            TokenBuffer[TokenCount] = malloc(y - x);
            if (!TokenBuffer[TokenCount]) return 2;

            for (uint32_t z = 0; z < y - x; z++)
                TokenBuffer[TokenCount][z] = Buffer[z + x];

            TokenBuffer[TokenCount][y - x] = '\0';
            TokenCount++;

        SyntaxStore:
            //Storing Syntax Token.
            TokenBuffer[TokenCount] = malloc(2);
            if (!TokenBuffer[TokenCount]) return 3;

            TokenBuffer[TokenCount][0] = Buffer[y];
            TokenBuffer[TokenCount][1] = '\0';
            TokenCount++;

            x = ++y;

            goto LoopStart;
        }
        
        if (Buffer[y] == '\n' || Buffer[y] == '\r')
        {
            x = ++y;
            continue;
        }

        //Using Offset from While Loop to Store Token.
        TokenBuffer[TokenCount] = malloc(y - x + 1);
        if (!TokenBuffer[TokenCount]) return 2;

        for(uint32_t z = 0; z < y - x + 1; z++)
            TokenBuffer[TokenCount][z] = Buffer[z + x];
        
        TokenBuffer[TokenCount][y - x] = '\0';
        
        //Incrementing X to Start at Y.
        TokenCount++;
        x = ++y;
    }

    TokenBuffer[TokenCount] = 0;

    for (uint32_t x = 0; x < TokenCount; x++)
        printf("Name: %s Count: %d\n", TokenBuffer[x], x);

    printf("\n\n\nTotal Count: %d\n", TokenCount);

    return 0;
}

uint8_t sortNames()
{
    NameBuffer = malloc(sizeof(uint8_t*) * 2);
    if (!NameBuffer) return 1;

    NameBuffer[FUNCTIONNAME] = malloc(sizeof(NameStruct) * 100);
    if (!NameBuffer[FUNCTIONNAME]) return 2;

    NameBuffer[VARIABLENAME] = malloc(sizeof(NameStruct) * 100);
    if (!NameBuffer[VARIABLENAME]) return 3;

    for(uint8_t x = 0; x < TokenCount; x++)
    {
        //Check if it is a function.
        if(!strcmp(TokenBuffer[x], "fn\0"))
        {
            //Ensure valid name.
            uint8_t Char = TokenBuffer[x + 1][0];
            if (Char < 'A' || Char > 'z') return INVALID_NAME_CHAR;
            if (Char >= ':' && Char <= '@') return INVALID_NAME_CHAR;
            if (Char == '(' || Char == ')' || Char == '{' || Char == '}') return INVALID_NAME_CHAR;

            NameStruct Function = { TokenBuffer[x + 1], x + 1};

            if (!FunctionCount) goto AddFunction;
            //Make sure function name doesn't exist elsewhere.
            for (uint32_t y = 0; y < FunctionCount; y++)
                if (!strcmp(NameBuffer[FUNCTIONNAME][y].Name, TokenBuffer[x + 1])) return NAME_ELSEWHERE;

        AddFunction:
            //Add Function to List.
            NameBuffer[FUNCTIONNAME][FunctionCount++] = Function;
            continue;
        }

        //Variable Declaration.
        if(TokenBuffer[x][0] == ':')
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

            NameStruct VariableName = { TokenBuffer[x - 1], x -1};
            if (!VariableCount) goto AddVariable;

            //Make sure Variable Name doesn't exist elsewhere.
            for (uint32_t y = 0; y < FunctionCount; y++)
                if (!strcmp(NameBuffer[VARIABLENAME][y].Name, TokenBuffer[x - 1])) return NAME_ELSEWHERE;

        AddVariable:
            //Add Variable to List.
            NameBuffer[VARIABLENAME][VariableCount++] = VariableName;
            continue;
        }
    }

    for (uint32_t x = 0; x < FunctionCount; x++)
        printf("Function Name: %s Location: %d\n", NameBuffer[FUNCTIONNAME][x].Name, NameBuffer[FUNCTIONNAME][x].Location);
 
    for (uint32_t x = 0; x < VariableCount; x++)
        printf("Variable Name: %s Location: %d\n", NameBuffer[VARIABLENAME][x].Name, NameBuffer[VARIABLENAME][x].Location);

    return 0;
}

uint8_t calculateArithmetic()
{
    //Set up Section .text
    char Section[] = "\n\n\nSection .text\n\0";
    fwrite(Section, 1, strlen(Section), OutputFile);

    //Do Initialise First.
    for (uint32_t x = 0; x < VariableCount; x++)
    {
        if (TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 3][0] == '=')
        {
            char MOV1[] = "mov [";
            char MOV2[] = "], ";
            char END[] = "\n\0";
            uint16_t Length = strlen(MOV1) + strlen(MOV2) + strlen(END);
            Length += strlen(NameBuffer[VARIABLENAME][x].Name);
            Length += strlen(TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 4]);

            uint8_t* Buffer = malloc(Length + 1);
            if (!Buffer) return 1;

            strcpy(Buffer, MOV1);
            strcat(Buffer, NameBuffer[VARIABLENAME][x].Name);
            strcat(Buffer, MOV2);
            strcat(Buffer, TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 4]);
            strcat(Buffer, END);

            Buffer[Length] = '\0';
            fwrite(Buffer, 1, Length, OutputFile);
            free(Buffer);
        }
        else
        {
            char MOV1[] = "mov [";
            char MOV2[] = "], 0\n\0";

            uint16_t Length = strlen(MOV1) + strlen(MOV2);
            Length += strlen(NameBuffer[VARIABLENAME][x].Name);

            uint8_t* Buffer = malloc(Length + 1);
            if (!Buffer) return 1;

            strcpy(Buffer, MOV1);
            strcat(Buffer, NameBuffer[VARIABLENAME][x].Name);
            strcat(Buffer, MOV2);

            Buffer[Length] = '\0';
            fwrite(Buffer, 1, Length, OutputFile);
            free(Buffer);
        }
    }

    return 0;
}

//This sets up Section .BSS
uint8_t writeVariables()
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

    char Section[] = "Section .bss\n\0";
    fwrite(Section, 1, strlen(Section), OutputFile);


    for(uint32_t x = 0; x < VariableCount; x++)
    {
        char VARRESB[] = ": resb ";
        char END[] = "\n\0";
        uint16_t Length = strlen(NameBuffer[VARIABLENAME][x].Name) + strlen(VARRESB) + strlen(END) + 1;

        char* Buffer = malloc(Length + 1);
        if (!Buffer) return 1;

        strcpy(Buffer, NameBuffer[VARIABLENAME][x].Name);
        strcat(Buffer, VARRESB);

        if (!strcmp(TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 2], "u1"))
            strcat(Buffer, "1");
        else if (!strcmp(TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 2], "u2"))
            strcat(Buffer, "2");
        else if (!strcmp(TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 2], "u4"))
            strcat(Buffer, "4");

        strcat(Buffer, END);

        Buffer[Length] = '\0';
        fwrite(Buffer, 1, Length, OutputFile);

        free(Buffer);
    }

    calculateArithmetic();
    fclose(OutputFile);
    return 0;
}


uint8_t compile(const uint8_t* FileLocation)
{
    FILE* File = fopen(FileLocation, "rb");
    if(!File) return 1;

    //Get Total Size of File.
    fseek(File, 0, SEEK_END);
    uint32_t Size = (uint32_t)ftell(File);
    fseek(File, 0, SEEK_SET);

    //Create Buffer to hold file.
    uint8_t* Buffer = malloc(Size);
    if(!Buffer) return 2;

    fread(Buffer, 1, Size, File);

    if(getTokens(Buffer, Size)) return 3;

    if (sortNames()) return 9;

    if (writeVariables()) return 10;
    return 0;
}
