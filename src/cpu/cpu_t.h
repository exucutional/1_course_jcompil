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
enum X86_64_OPCODES
{
	X86_64_NOP = 0x90,
	X86_64_OPERAND_SIZE = 0x48,
	X86_64_OPERAND_SIZE_EXT = 0x4d, 
	X86_64_MOVRR = 0x89,
	X86_64_MOVRMR = 0x8b,
	X86_64_MOVMEM_REG = 0x40,
	X86_64_MOVQ_REG = 0xb8,
	X86_64_ADD_SUB = 0x81,
	X86_64_ADDR = 0x01,
	X86_64_SUBR = 0x29,
	X86_64_SUB_REG = 0xe8,
	X86_64_MUL_DIV = 0xf7,
	X86_64_MUL_REG = 0xe0,
	X86_64_DIV_REG = 0xf0,
	X86_64_PUSHR = 0x50,
	X86_64_PUSHQ = 0x68,
	X86_64_PUSHMR = 0xff,
	X86_64_PUSHMR_REG = 0x70,
	X86_64_POPR = 0x58,
	X86_64_POPR_EXT_REG = 0x50,
	X86_64_EXT_CMD = 0x41,
	X86_64_RET = 0xc3,
	X86_64_CALL = 0xe8,
	X86_64_JMP = 0xeb,
	X86_64_JE = 0x74,
	X86_64_JNE = 0x75,
	X86_64_JG = 0x7f,
	X86_64_JGE = 0x7d,
	X86_64_JL = 0x7c,
	X86_64_JLE = 0x7e,
	X86_64_SYSCALL1 = 0x0f,
	X86_64_SYSCALL2 = 0x05,
	X86_64_SYSCALL_REG = 0xc8,
	X86_64_CMPR = 0x39,
	X86_64_ASCM_REG = 0xc0,
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
	void* mem_min;
	void* newmem_min;
	void* mem_max;
	void* newmem_max;
	void* rip;
	void* newrip;
	uint64_t reg[REG_MAX];
	uint64_t trap;
};

int cpu_init(struct cpu_t *cpu, uint8_t **code_p, size_t capacity);
int cpu_dtor(struct cpu_t *cpu);
int cpu_set_rip(struct cpu_t *cpu, void *code);
int cpu_set_newrip(struct cpu_t *cpu, void *code);
int cpu_set_rsp(struct cpu_t *cpu, void *rsp);
int cpu_set_rsmp(struct cpu_t *cpu, void *rsp);
int cpu_set_mem(struct cpu_t *cpu, void *mem_min, void *mem_max);
int cpu_set_newmem(struct cpu_t *cpu, void *mem_min, void *mem_max);
long cpu_run (struct cpu_t *cpu);

long code_asm (char  *text, uint8_t **code_p);
long code_dasm(uint8_t *code, char **text_p, ssize_t size);

int get_cmd(char **text_p, uint8_t **code_p);
int get_reg(char **text_p, uint8_t **code_p);
int get_num(char **text_p, uint8_t **code_p);
int get_label(char **text_p, uint8_t **code_p);

int put_cmd(char **text_p, uint8_t **code_p);
int put_reg(char **text_p, uint8_t **code_p);
int put_num(char **text_p, uint8_t **code_p);
int put_label(char **text_p, uint8_t **code_p);


#endif // _CPU_T_H_
