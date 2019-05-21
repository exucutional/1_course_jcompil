/*!
 * \brief cpu_t.h
 * \author Exdev
 * \version 0.6.0
 */

#ifndef _CPU_T_H_
#define _CPU_T_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

enum CMD_CODES
{
	#define DEF_CMD(name, val)	CMD_##name,
	#include "cmd_list.h"
	CMD_MAX,
	#undef DEF_CMD
};

enum REG_CODES
{
	#define DEF_REG(name)	REG_##name,
	#include "reg_list.h"
	REG_MAX,
	#undef DEF_REG
};

enum TRAP_CODES
{
	TRAP_NO_TRAP = 0,
	TRAP_EXIT,
	TRAP_SYSCALL,
	TRAP_ERROR_MEMORY,
	TRAP_ERROR_INSTR,
	TRAP_ERROR_SYSCALL,
};

enum SYSCALL_CODES
{
	SYSCALL_EXIT = 0,
	SYSCALL_OUTQ = 1,
	SYSCALL_INPQ = 2,
};

struct label_t
{
	size_t offs;
	int isdefined;
};

struct cpu_t
{
	/// Memory includes rip, rsp
	void *mem_min;
	void *mem_max;
	void *rip;
	uint64_t reg[REG_MAX];
	uint64_t trap;
};

int cpu_init(struct cpu_t *cpu, uint8_t **code_p, size_t mem_cap);
int cpu_set_rip(struct cpu_t *cpu, void *code);
int cpu_set_rsp(struct cpu_t *cpu, void *rsp);
int cpu_set_rsmp(struct cpu_t *cpu, void *rsp);
int cpu_set_mem(struct cpu_t *cpu, void *mem_min, void *mem_max);
long cpu_run (struct cpu_t *cpu);

long code_asm (char  *text, uint8_t **code_p);
long code_dasm(uint8_t *code, char **text_p, long size);

int get_cmd(char **text_p, uint8_t **code_p);
int get_reg(char **text_p, uint8_t **code_p);
int get_num(char **text_p, uint8_t **code_p);
int get_label(char **text_p, uint8_t **code_p);

int put_cmd(char **text_p, uint8_t **code_p);
int put_reg(char **text_p, uint8_t **code_p);
int put_num(char **text_p, uint8_t **code_p);
int put_label(char **text_p, uint8_t **code_p);


#endif // _CPU_T_H_
