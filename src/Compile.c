//TODO: Tokenize everything. (Done)
//TODO: Check for Variables. (Done but does not have Scope)
//TODO: Design Scope.
//TODO: Design Stack for local Variables.
//TODO: Create AST (Need to add Precedence.

//What to do:
//Make String ASM code more modular. (Done).
//Introduce Functions.

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

uint8_t createInstruction(uint8_t Instruction, uint8_t* VarA, uint8_t* VarB)
{
    //Strings to apply Instructions.
    uint8_t MOVE[] = "mov ";
    uint8_t ADDITION[] = "add ";
    uint8_t SUBTRACT[] = "sub ";
    uint8_t MULTIPLY[] = "mul ";
    uint8_t DIVIDE[] = "div ";
    uint8_t AND[] = "and ";
    uint8_t XOR[] = "xor ";
    uint8_t OR[] = "or ";

    uint8_t REGISTERS[4][4] =
    {
        "eax",
        "ebx",
        "ecx",
        "edx"
    };

    uint8_t START[] = ", ";
    uint8_t END[] = "\n\0";

    uint8_t VAREND[] = "]\n\0";
    uint8_t VARSTART[] = ", [";

    //Checking all Operators.
    switch (Instruction)
    {
        case '+':
        {
            uint8_t* String = stringifyInstruction(5, MOVE, REGISTERS[0], VARSTART, VarA, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            String = stringifyInstruction(5, MOVE, REGISTERS[3], VARSTART, VarB, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);


            String = stringifyInstruction(5, ADDITION, REGISTERS[0], START, REGISTERS[3], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            uint8_t NEWVARSTART[] = "[";
            uint8_t NEWVAREND[] = "], ";

            String = stringifyInstruction(6, MOVE, NEWVARSTART, VarA, NEWVAREND, REGISTERS[0], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);
            break;
        }

        case '-':
        {
            uint8_t* String = stringifyInstruction(5, MOVE, REGISTERS[0], VARSTART, VarA, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            String = stringifyInstruction(5, MOVE, REGISTERS[3], VARSTART, VarB, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);


            String = stringifyInstruction(5, SUBTRACT, REGISTERS[0], START, REGISTERS[3], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            uint8_t NEWVARSTART[] = "[";
            uint8_t NEWVAREND[] = "], ";

            String = stringifyInstruction(6, MOVE, NEWVARSTART, VarA, NEWVAREND, REGISTERS[0], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);
            break;
        }

        case '&':
        {
            uint8_t* String = stringifyInstruction(5, MOVE, REGISTERS[0], VARSTART, VarA, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            String = stringifyInstruction(5, MOVE, REGISTERS[3], VARSTART, VarB, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);


            String = stringifyInstruction(5, AND, REGISTERS[0], START, REGISTERS[3], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            uint8_t NEWVARSTART[] = "[";
            uint8_t NEWVAREND[] = "], ";

            String = stringifyInstruction(6, MOVE, NEWVARSTART, VarA, NEWVAREND, REGISTERS[0], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);
            break;
        }

        case '^':
        {
            uint8_t* String = stringifyInstruction(5, MOVE, REGISTERS[0], VARSTART, VarA, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            String = stringifyInstruction(5, MOVE, REGISTERS[3], VARSTART, VarB, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);


            String = stringifyInstruction(5, XOR, REGISTERS[0], START, REGISTERS[3], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            uint8_t NEWVARSTART[] = "[";
            uint8_t NEWVAREND[] = "], ";

            String = stringifyInstruction(6, MOVE, NEWVARSTART, VarA, NEWVAREND, REGISTERS[0], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);
            break;
        }


        case '|':
        {
            uint8_t* String = stringifyInstruction(5, MOVE, REGISTERS[0], VARSTART, VarA, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            String = stringifyInstruction(5, MOVE, REGISTERS[3], VARSTART, VarB, VAREND);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);


            String = stringifyInstruction(5, OR, REGISTERS[0], START, REGISTERS[3], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);

            uint8_t NEWVARSTART[] = "[";
            uint8_t NEWVAREND[] = "], ";

            String = stringifyInstruction(6, MOVE, NEWVARSTART, VarA, NEWVAREND, REGISTERS[0], END);
            fwrite(String, 1, strlen(String), OutputFile);
            free(String);
            break;
        }
    }

    return 0;
}


//Function related.
uint8_t createFunction()
{

    return 0;
}

//Tokenizer.
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
    uint8_t Section[] = "\n\n\nSection .text\n\0";
    fwrite(Section, 1, strlen(Section), OutputFile);

    //Do Initialise First.
    for (uint32_t x = 0; x < VariableCount; x++)
    {
        if (TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 3][0] == '=')
        {
            uint8_t MOV1[] = "mov [";
            uint8_t MOV2[] = "], ";
            uint8_t END[] = "\n\0";
            
            uint8_t* Buffer = stringifyInstruction(5, MOV1, NameBuffer[VARIABLENAME][x].Name, MOV2, TokenBuffer[NameBuffer[VARIABLENAME][x].Location + 4], END);
            fwrite(Buffer, 1, strlen(Buffer), OutputFile);
            free(Buffer);
        }
        else
        {
            uint8_t MOV1[] = "mov [";
            uint8_t MOV2[] = "], 0\n\0";

            uint8_t* Buffer = stringifyInstruction(3, MOV1, NameBuffer[VARIABLENAME][x].Name, MOV2);
            fwrite(Buffer, 1, strlen(Buffer), OutputFile);
            free(Buffer);
        }
    }

    //Scan Tokens and create Arithmetic Operations.
    for(uint32_t x = 0; x < TokenCount; x++)
    {
        if (TokenBuffer[x][0] == '+')
            createInstruction('+', TokenBuffer[x - 1], TokenBuffer[x + 1]);
        if (TokenBuffer[x][0] == '-')
            createInstruction('-', TokenBuffer[x - 1], TokenBuffer[x + 1]);
        if (TokenBuffer[x][0] == '&')
            createInstruction('&', TokenBuffer[x - 1], TokenBuffer[x + 1]);
        if (TokenBuffer[x][0] == '|')
            createInstruction('|', TokenBuffer[x - 1], TokenBuffer[x + 1]);
        if (TokenBuffer[x][0] == '^')
            createInstruction('^', TokenBuffer[x - 1], TokenBuffer[x + 1]);
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

    uint8_t Section[] = "Section .bss\n\0";
    fwrite(Section, 1, strlen(Section), OutputFile);

    for(uint32_t x = 0; x < VariableCount; x++)
    {
        uint8_t VARRESB[] = ": resb ";
        uint8_t END[] = "\n\0";
        size_t Length = (uint16_t)strlen(NameBuffer[VARIABLENAME][x].Name) + strlen(VARRESB) + strlen(END) + 1;

        uint8_t* Buffer = malloc(Length + 1);
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
