#include "Translate.h"
#include "Function.h"
#include "Tokenizer.h"
#include "Includes.h"

//Sorting Tokens
NameStruct** PublicNameBuffer = 0;
uint32_t PublicFunctionCount = 0;
uint32_t PublicVariableCount = 0;

//Output File.
FILE* OutputFile = 0;

//Tokenizer.
uint8_t sortNames()
{
    PublicNameBuffer = malloc(sizeof(uint8_t*) * 2);
    if (!PublicNameBuffer) return MALLOC_ERROR;

    PublicNameBuffer[FUNCTIONNAME] = malloc(sizeof(NameStruct) * 100);
    if (!PublicNameBuffer[FUNCTIONNAME]) return MALLOC_ERROR;

    PublicNameBuffer[VARIABLENAME] = malloc(sizeof(NameStruct) * 100);
    if (!PublicNameBuffer[VARIABLENAME]) return MALLOC_ERROR;

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
    for (uint32_t x = 0; x < PublicFunctionCount; x++)
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
    if (!Buffer) return MALLOC_ERROR;

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
    
    for (uint32_t x = 0; x < TokenCount; x++)
        printf("%s \n", TokenBuffer[x]);


    if (sortNames()) return 9;

    //Generate IL for Middleware.
    FILE* GamerFile = fopen("D:/.WND/IL.wil", "rb");
    if (GamerFile)
    {
        remove("D:/.WND/IL.wil");
        fclose(GamerFile);
    }

    //Turn Frontend into Intermediate Language.
    GamerFile = fopen("D:/.WND/IL.wil", "ab");
    if (generateIntermediateLanguage(GamerFile)) return 10;
    return 0;
}

int main()
{
    compile("D:/.WND/Main.wl", 0);
    return 0;
}
