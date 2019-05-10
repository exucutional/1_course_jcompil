#include <iostream>
#include <cassert>
#include "cpu/cpu_t.h"
#include "jcompil.hpp"

void testasm()
{
	asm(".intel_syntax noprefix\n\t"
		"cmp r10, r11\n\t"
		"cmp r11, r10\n\t"
		".att_syntax noprefix\n\t"
	);
}

const size_t MEMORY_SIZE = 4096;
int main()
{
    Jcompil jcompil(MEMORY_SIZE);
    FILE* fasm = fopen("./jfiles/code.masm", "r");
    //testasm();
    jcompil.assembl(fasm);
    jcompil.translate();
    jcompil.load();
    int temp = jcompil.run();
    printf("%d\n", temp);
    fclose(fasm);
	return EXIT_SUCCESS;
}