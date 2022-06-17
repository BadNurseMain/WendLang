#ifndef STDLIB
#define STDLIB

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#endif

//--------------------------------------------------------------- Globals -------------------------------------------------


//Strings to apply Instructions.
uint8_t MOVE[] = "mov ";
uint8_t ADDITION[] = "add ";
uint8_t SUBTRACT[] = "sub ";
uint8_t MULTIPLY[] = "mul ";
uint8_t DIVIDE[] = "div ";
uint8_t AND[] = "and ";
uint8_t XOR[] = "xor ";
uint8_t OR[] = "or ";



uint8_t INSTRUCTIONS[14][5] =
{
    "mul ",
    "div ",
    "mod ",
    "add ",
    "sub ",
    "and ",
    "or ",
    "xor ",
    "shl ",
    "shr ",
    "jl ",
    "jg ",
	"je ",
	"jne "
};

//Stack Specific.
uint8_t PUSH[] = "push ";
uint8_t POP[] = "pop ";
uint8_t PLUS[] = " + ";
uint8_t MINUS[] = " - ";
uint8_t OPENBRACKET[] = "[\0";
uint8_t CLOSEDBRACKET[] = "]\0";

uint8_t NEWVARSTART[] = "[";
uint8_t NEWVAREND[] = "], ";

uint8_t REGISTERS[4][7][4] =
{
    {
        "al",
        "bl",
        "cl",
        "dl",
    },

    {
        "ah",
        "bh",
        "ch",
        "dh",
    },

    {
        "ax",
        "bx",
        "cx",
        "dx",
        "sp",
        "si",
        "di"
    },

    {
        "eax",
        "ebx",
        "ecx",
        "edx",
        "esp",
        "esi",
        "edi"
    }
};

uint8_t START[] = ", ";
uint8_t END[] = "\n\0";

uint8_t VAREND[] = "]\n\0";
uint8_t VARSTART[] = ", [";
