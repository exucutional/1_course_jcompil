/*!
 * \brief cpu_code.c
 * \author Exdev
 * \version 0.5
 */

#include "cpu_t.h"
#include "time.h"
#include <assert.h>

const size_t MEMORY_SIZE = 4096;
const size_t CPU_MEMORY_SIZE = 1024 * 4096;

int main(int argc, char* argv[])
{
	char *text = calloc(MEMORY_SIZE, sizeof(char));
	uint8_t *code = calloc(MEMORY_SIZE, sizeof(uint8_t));
	FILE *fin = NULL;
	if (argc < 1)
		assert (0);
	fin = fopen(argv[1], "r");
	assert(fin);
	fseek(fin, 0, SEEK_END);
	size_t size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	fread(text, size, 1, fin);
	code_asm(text, &code);
	struct cpu_t cpu = {};
	cpu_init(&cpu, &code, CPU_MEMORY_SIZE);
	fprintf(stderr, "----Running----\n");
	clock_t exec_time = clock();
	uint64_t tmp = cpu_run(&cpu);
	exec_time = clock() - exec_time;
	printf("done:\n ncmds: %llu\n time: %lg\n freq: %lg MHz\n",
		   tmp, (double) exec_time / CLOCKS_PER_SEC, ((double) tmp )
		   / exec_time * CLOCKS_PER_SEC / 1000000);
	return 0;
}