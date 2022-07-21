#include "Translate.h"
#include "Function.h"

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
int8_t** TokenBuffer = 0;
uint32_t TokenCount = 0;


#ifndef PUBLICNAMES
#define PUBLICNAMES
//Struct for Name Referencing.
typedef struct
{
    int8_t* Name;
    uint32_t Location;
} NameStruct;

#endif

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
uint8_t getTokens(int8_t* Buffer, uint32_t Size)
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
            //Double Comment.
            if(Buffer[y + 1] == '#')
            {
                x += 2;
                do
                {
                    if (Buffer[x] == '#')
                        if (Buffer[x + 1] == '#')break;
                    x++;
                } while (1);

                x++;
                continue;
            }

            while (Buffer[x] != '\n') x++;
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

                //Arrays & Pointers.
                case '[': goto Syntax;
                case ']': goto Syntax;

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
            switch (Buffer[y - 1])
            {
                case ' ': goto SyntaxStore;
                case '(': goto SyntaxStore;
                case ')': goto SyntaxStore;
                case '{': goto SyntaxStore;
                case '}': goto SyntaxStore;
                case '[': goto SyntaxStore;
                case ']': goto SyntaxStore;
                case '\r': goto SyntaxStore;
                case '\n': goto SyntaxStore;
                case '\t': goto SyntaxStore;
            }

            if(Buffer[y] == '*')
            {
                if (Buffer[y - 1] == '4' || Buffer[y - 1] == '2' || Buffer[y - 1] == '1')
                {
                    if (Buffer[y - 2] == 'u')
                    {
                        TokenBuffer[TokenCount] = malloc(4);
                        TokenBuffer[TokenCount][0] = Buffer[y - 2];
                        TokenBuffer[TokenCount][1] = Buffer[y - 1];
                        TokenBuffer[TokenCount][2] = Buffer[y];
                        TokenBuffer[TokenCount++][3] = '\0';

                        x = ++y;
                        goto LoopStart;
                    }

                }
            }

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
            int8_t Char = TokenBuffer[x + 1][0];
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
            int8_t DataTypes[3][3] =
            {
                "u1",
                "u2",
                "u4"
            };

            //Make sure valid name.
            int8_t Char = TokenBuffer[x - 1][0];
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

uint8_t generateIntermediateLanguage(FILE* File)
{
	//WIL File signature.
	fwrite("WIL", 1, 3, File);

	//Listing all Descriptors.
	uint32_t TotalDescriptorCount = PublicFunctionCount + PublicVariableCount;
	fwrite(&TotalDescriptorCount, 1, 4, File);

	//Starting with Globals.
	//Doing it later.

	//Functions.
	for(uint32_t x = 0; x < PublicFunctionCount; x++)
	{
		//Writing Name Length.
		uint8_t NameLength = strlen(PublicNameBuffer[FUNCTIONNAME][x].Name);
		fwrite(&NameLength, 1, 1, File);

		//Specifying it is a function.
		uint8_t Properties = 1;
		fwrite(&Properties, 1, 1, File);

		//Return Type of the Function.
		uint8_t ReturnType = getFunctionReturnType(PublicNameBuffer[FUNCTIONNAME][x].Location);
		fwrite(&ReturnType, 1, 1, File);

		//Writing Name.
		fwrite(PublicNameBuffer[FUNCTIONNAME][x].Name, 1, NameLength, File);

		//Writing the Parameters of the Function.
		writeFunctionParameters(File, PublicNameBuffer[FUNCTIONNAME][x].Location);

		//Padding for Signature.
		uint32_t Padding = 0;
		fwrite(&Padding, 1, 4, File);

		int8_t NewLine = '\n';
		fwrite(&NewLine, 1, 1, File);

		getFunctionStatements(File, PublicNameBuffer[FUNCTIONNAME][x].Location);

		NewLine = '~';
		fwrite(&NewLine, 1, 1, File);
	}

	return 0;
}

uint8_t compile(const int8_t* FileLocation, const int8_t* OutputLocation)
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
    FILE* GamerFile = fopen("/home/badnursemain/eclipse-workspace/Wendigo/UWU.wil", "rb");
    if (GamerFile)
    {
        remove("/home/badnursemain/eclipse-workspace/Wendigo/UWU.wil");
        fclose(GamerFile);
    }

    //Turn Frontend into Intermediate Language.
    GamerFile = fopen("/home/badnursemain/eclipse-workspace/Wendigo/UWU.wil", "ab");
    if (generateIntermediateLanguage(GamerFile)) return 10;
    return 0;
}


int main()
{
	compile("/home/badnursemain/eclipse-workspace/Wendigo/Main.wl", 0);
	return 0;
}

