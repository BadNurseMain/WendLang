#include "Tokenizer.h"

//-- Preprocessor Types --//
#define PREPROCESS_DEFINITION (uint8_t) 1
#define PREPROCESS_INCLUDE (uint8_t) 2
#define PREPROCESS_PRAGMA (uint8_t) 3 
#define PREPROCESS_IF (uint8_t) 4

typedef struct _PreprocessIdentifier
{
    uint32_t Location;
    uint8_t Type;
    
    uint8_t ParameterCount;
    int8_t** Parameters;
} PreprocessIdentifier;

//Getting Tokens.
int8_t** TokenBuffer = 0;
uint32_t TokenCount = 0;

uint8_t _tokenExecutePreprocess(uint32_t StartLocation, uint8_t Type, int8_t** Parameters)
{
    switch(Type)
    {
        case PREPROCESS_INCLUDE:

            break;

        case PREPROCESS_DEFINITION:
            
            //Replacing Definition with Value.
            for(uint32_t x = StartLocation; x < TokenCount; x++)
                if (!strcmp(TokenBuffer[x], Parameters[0]))
                {
                    printf("TokenBuffer: %s, Parameters: %s \n", TokenBuffer[x], Parameters[1]);
                    TokenBuffer[x] = Parameters[1];
                }
            
            break;
    }

    return 1;
}

int8_t** _tokenGetPreprocessParams(uint8_t PreprocessType, uint32_t* TokenLocation)
{
    int8_t** String;

    switch(PreprocessType)
    {
        case PREPROCESS_INCLUDE:

            break;

        case PREPROCESS_DEFINITION:
            String = malloc(2 * sizeof(int8_t**));
            if (!String) return MALLOC_ERROR;

            String[0] = TokenBuffer[*TokenLocation + 0];
            String[1] = TokenBuffer[*TokenLocation + 1];

            *TokenLocation += 2;
            return String;

        default: return (void**)0;
    }
    
    //Hasn't found valid type.
    return (void**)0;
}

uint8_t _tokenTranslatePreprocess(int8_t* String)
{
    if (!strcmp(String, "include")) return PREPROCESS_INCLUDE;
    else if (!strcmp(String, "define")) return PREPROCESS_DEFINITION;

    return 0;
}

uint8_t _tokenHandlePreprocess()
{
    for(uint32_t x = 0; x < TokenCount; x++)
    {
        //Is a preprocessor.
        if(TokenBuffer[x][0] == '%')
        {
            //Getting the Type.
            uint8_t TypeResult = _tokenTranslatePreprocess(TokenBuffer[x + 1]);
            if (!TypeResult) return 1;

            //Getting the Parameters.
            uint32_t StartLocation = x + 2;

            int8_t** ParameterResult = _tokenGetPreprocessParams(TypeResult, &StartLocation);

            _tokenExecutePreprocess(StartLocation, TypeResult, ParameterResult);
        }

    }

    return 0;
}

uint8_t getTokens(int8_t* Buffer, uint32_t Size)
{
    uint8_t Scope = 0;

    //Create Initial Token Buffer.
    TokenBuffer = malloc(sizeof(uint8_t*) * 1000);
    if (!TokenBuffer) return MALLOC_ERROR;

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
            if (Buffer[y + 1] == '#')
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
                //Preprocessor.
                case '%': break;

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

                //String.
                case '"': goto Syntax;

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

            if (Buffer[y] == '*')
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
            if (!TokenBuffer[TokenCount]) return MALLOC_ERROR;

            for (uint32_t z = 0; z < y - x; z++)
                TokenBuffer[TokenCount][z] = Buffer[z + x];

            TokenBuffer[TokenCount++][y - x] = '\0';

        SyntaxStore:
            //Storing Syntax Token.
            TokenBuffer[TokenCount] = malloc(2);
            if (!TokenBuffer[TokenCount]) return MALLOC_ERROR;

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
    
    //Once done Getting Tokens,
    //Handle Preprocessor.
    _tokenHandlePreprocess();
    return 0;
}
