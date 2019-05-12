#include <iostream>
#include <cassert>
#include "cpu/cpu_t.h"
#include "jcompil.hpp"

void testasm()
{
	asm(".intel_syntax noprefix\n\t"
        "add rax, byte [rsp - 1]\n\t"
        "mov rcx, byte [rsp]\n\t"
        "syscall\n\t"
        "call r12\n\t"
		".att_syntax noprefix\n\t"
	);
}
extern void _testasm();
const size_t MEMORY_SIZE = 4096;
int main()
{
    Jcompil jcompil(MEMORY_SIZE);
    FILE* fasm = fopen("./jfiles/code.masm", "r");
    jcompil.assembl(fasm);
    jcompil.translate();
    jcompil.load();
    int temp = jcompil.run();
    //printf("\n%d\n", temp);
    fclose(fasm);
    //testasm();
	return EXIT_SUCCESS;
}