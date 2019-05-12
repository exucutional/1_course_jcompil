#if (defined (_WIN32) || defined (_WIN64))
#include "Windows.h"
#elif (defined (LINUX) || defined (__linux__))
#include <sys/mman.h>
#include <limits.h>
#endif
#include <iostream>
#include <cassert>
#include "jcompil.hpp"
#include "cpu/cpu_t.h"
#ifdef NDEBUG
#define __DEBUG_EXEC(code) ;
#else
#define __DEBUG_EXEC(code) code
#endif
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif
Jcompil::Jcompil(size_t mcap)
{
	#if (defined (_WIN32) || defined (_WIN64))
        __DEBUG_EXEC(std::cout << "I'm on Windows!" << std::endl);
        memory = static_cast<uint8_t*>(VirtualAlloc(NULL, mcap, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        assert(memory);
    #elif (defined (LINUX) || defined (__linux__))
       	__DEBUG_EXEC(std::cout << "I'm on Linux!" << std::endl);
       	memory = new uint8_t[2 * mcap + PAGESIZE - 1];
       	assert(memory);
       	memdel = memory;
       	memory = (uint8_t*)(((size_t)memory + PAGESIZE - 1) & ~(PAGESIZE - 1));
       	syscallmem = memory + mcap;
       	if (!mprotect(memory, mcapacity + PAGESIZE, PROT_READ | PROT_WRITE | PROT_EXEC)) {
       		fprintf(stderr, "MPROTECT MEMORY ERROR\n");
       		assert(0);
       	}
    #endif
	mcapacity = mcap;
}
Jcompil::~Jcompil()
{
	#if (defined (_WIN32) || defined (_WIN64))
   		VirtualFree(memory, mcapacity, MEM_RELEASE);
    #elif (defined (LINUX) || defined (__linux__))
   		delete[] memdel;
    #endif
}
int Jcompil::assembl(FILE* fin) const
{
	if (!fin) {
		fprintf(stderr, "ASM: No input .masm file\n");
		return EXIT_FAILURE;
	}
	fseek(fin, 0, SEEK_END);
	size_t size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	auto text = new char[size];
	auto code = new uint8_t[8 * size];
	assert(text);
	assert(code);
	fread(text, size, 1, fin);
	code_asm(text, &code);
	//__DEBUG_EXEC(for (uint8_t* iter = code; iter - code < 256; iter += sizeof(uint8_t))
	//	printf("%x ", static_cast<uint8_t>(*iter)));
	FILE* fout = nullptr;
	if (!(fout = fopen("./jfiles/bitmap.rjit", "wb"))) {
		fprintf(stderr, "ASM: Couldn't create .rjit file\n");
		return EXIT_FAILURE;
	}
	fwrite(code, 8 * size, 1, fout);
	fclose(fout);
	delete[] text;
	delete[] code;
	return EXIT_SUCCESS;
}
int Jcompil::translate() const
{
	FILE* fin = nullptr;
	if (!(fin = fopen("./jfiles/bitmap.rjit", "rb"))) {
		fprintf(stderr, "JIT: No input .rjit file\n");
		return EXIT_FAILURE;
	}
	fseek(fin, 0, SEEK_END);
	size_t size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	auto code = new uint8_t[size];
	fread(code, size, 1, fin);
	fclose(fin);
	//CPU HANDLING CODE
	struct cpu_t cpu = {};
	cpu_init(&cpu, &code);
	auto finalcode = static_cast<uint8_t*>(cpu.newrip);
	//2x running. (1st collecting labels, 2st putting labels)
	cpu_run(&cpu);
	cpu.rip = cpu.mem_min;
	cpu.newrip = cpu.newmem_min;
	cpu.trap = TRAP_NO_TRAP;
	cpu_run(&cpu);
	FILE* fout = nullptr;
	if (!(fout = fopen("./jfiles/bitcode.fjit", "wb"))) {
		fprintf(stderr, "JIT: Couldn't create .fjit file\n");
		return EXIT_FAILURE;
	}
	fwrite(finalcode, 4096, 1, fout);
	cpu_dtor(&cpu);
	delete[] code;
	fclose(fout);
	return EXIT_SUCCESS;
}
int Jcompil::load()
{
	FILE* fincode = nullptr;
	FILE* finsyscall = nullptr;
	if (!(fincode = fopen("./jfiles/bitcode.fjit", "rb"))) {
		fprintf(stderr, "JIT: No input .fjit file\n");
		return EXIT_FAILURE;
	}
	if (!(finsyscall = fopen("./jfiles/syscalls.out", "rb"))) {
		fprintf(stderr, "JIT: No input syscall file\n");
		return EXIT_FAILURE;
	}
	fread(memory, mcapacity, 1, fincode);
	fread(syscallmem, mcapacity, 1, finsyscall);
	syscallmem += 0x80;
	fclose(fincode);
	fclose(finsyscall);
	return EXIT_SUCCESS;
}
int Jcompil::run() const
{
	int (*run) (void);
	void (*syscall) (void);
	run = reinterpret_cast<int(*)()>(memory);
	syscall = reinterpret_cast<void(*)()>(syscallmem);
	asm(".intel_syntax noprefix\n\t"
		"push rcx\n\t"
		"push rdx\n\t"
		"push rbx\n\t"
		"push rsp\n\t"
		"push rbp\n\t"
		"push rdi\n\t"
		"push rsi\n\t"
		"push r10\n\t"
		"push r11\n\t"
		"push r12\n\t"
		"mov r12, %0\n\t"
		".att_syntax noprefix\n\t"
		:
		: "r"(syscall)
	);
	int temp = run();
	asm(".intel_syntax noprefix\n\t"
		"pop r12\n\t"
		"pop r11\n\t"
		"pop r10\n\t"
		"pop rsi\n\t"
		"pop rdi\n\t"
		"pop rbp\n\t"
		"pop rsp\n\t"
		"pop rbx\n\t"
		"pop rdx\n\t"
		"pop rcx\n\t"
		".att_syntax noprefix\n\t"
	);
	return temp;
}