#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "cpu_t.h"


int unit_test(struct cpu_t *cpu, uint8_t **code_t)
{
	int res = 0;
	for (int i = 0; i < 100; i++)
	{
		cpu_init(cpu, code_t);
		res += cpu_run(cpu);
	}
	printf("\n---------------------------------\nFrequency cmd exec : %lf\n"
			"---------------------------------\n", (double)clock() / res);
	return 0;
}
