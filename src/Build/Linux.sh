#!/bin/bash
gcc -m32 -ffreestanding Start/Main.c Compiler/Arith.c Compiler/Compile.c Compiler/Func.c -O2 -o WendLang
