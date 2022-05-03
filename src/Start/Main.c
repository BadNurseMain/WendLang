#include "../Compiler/Compile.h"

int main(int argc, char* argv[])
{
    uint8_t OutputLocation[64] = {0};
    uint8_t InputLocation[64] = {0};

    for(uint8_t x = 1; x < argc; x++)
    {
        if(argv[x][0] == '-')
        {
            if(!strcmp(argv[x], "-o"))
            {
                strcpy(OutputLocation, argv[++x]);
                continue;
            }
            return 1;
        }

        if(!InputLocation[0]) strcpy(InputLocation, argv[x]);
        else
        {
            printf("Multiple Files Defined as Input \n");
            return 1;
        }
    }

    if(!InputLocation[0]) return 2;

    if(OutputLocation[0]) compile(InputLocation, OutputLocation);
    else compile(InputLocation, 0);
    
    return 0;    
}
