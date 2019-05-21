/*!
 * \brief cpu_t.c
 * \author Exdev
 * \version 0.6.0
 */

#include "cpu_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

//#define NDEBUG

/// Debug macro
#ifdef NDEBUG
#define aqqq ;
#else
#define __DEBUG_EXEC(code) code
#endif

/// Definitions of cpu-cmds
#define DEF_CMD(name, type)	\
static inline void cpu_cmd_##name(struct cpu_t*);
#include "cmd_list.h"
#undef DEF_CMD

#define MOVQ(reg, val) 									\
do {													\
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);	\
	cpu_newrip_write_byte(cpu, X86_64_MOVQ_REG + reg);	\
	cpu_newrip_write_qword(cpu, val);					\
} while (0)

#define JMP_PREPARE 																					\
do {																									\
	cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);															\
	cpu_newrip_write_byte(cpu, X86_64_POPR_EXT_REG + REG_r10);											\
	cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);															\
	cpu_newrip_write_byte(cpu, X86_64_POPR_EXT_REG + REG_r11);											\
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE_EXT);												\
	cpu_newrip_write_byte(cpu, X86_64_CMPR);															\
	cpu_newrip_write_byte(cpu, X86_64_ASCM_REG + (REG_r11 - REG_r8) + ((REG_r10 - REG_r8) << 3));		\
} while (0)

#define ARITH_PREPARE 																					\
do {																									\
	cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);															\
	cpu_newrip_write_byte(cpu, X86_64_POPR_EXT_REG + REG_r11);											\
	cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);															\
	cpu_newrip_write_byte(cpu, X86_64_POPR_EXT_REG + REG_r10);											\
} while (0)

static const int LABEL_MAX = 256;
static struct label_t lbl[LABEL_MAX] = {0};

int cpu_init(struct cpu_t *cpu, uint8_t **code_p, size_t capacity)
{
	//cpu->reg[REG_rax] = SYSCALL_EXIT;
	cpu->trap = TRAP_NO_TRAP;
	auto memory = static_cast<uint8_t*>(calloc(capacity, sizeof(uint8_t)));
	auto newmemory = static_cast<uint8_t*>(calloc(2 * capacity, sizeof(uint8_t)));
	memcpy(memory, *code_p, 512);
	cpu_set_rsp(cpu, memory + capacity / 2);
	cpu_set_rip(cpu, memory);
	cpu_set_newrip(cpu, newmemory);
	cpu_set_mem(cpu, memory, memory + capacity);
	cpu_set_newmem(cpu, newmemory, newmemory + 2 * capacity);
	return 0;
}
int cpu_dtor(struct cpu_t *cpu)
{
	free(cpu->mem_min);
	free(cpu->newmem_min);
	return 0;
}
/// Set instr pointer
int cpu_set_rip(struct cpu_t *cpu, void *code)
{
	cpu->rip = code;
	return 0;
}
int cpu_set_newrip(struct cpu_t *cpu, void *code)
{
	cpu->newrip = code;
	return 0;
}
/// Set stack pointer
int cpu_set_rsp(struct cpu_t *cpu, void *rsp)
{
	cpu->reg[REG_rsp] = (uint64_t) rsp;
	return 0;
}

/// Set memory bounds
int cpu_set_mem(struct cpu_t *cpu, void *mem_min, void *mem_max)
{
	cpu->mem_min = mem_min;
	cpu->mem_max = mem_max;
	return 0;
}
int cpu_set_newmem(struct cpu_t *cpu, void *mem_min, void *mem_max)
{
	cpu->newmem_min = mem_min;
	cpu->newmem_max = mem_max;
	return 0;
}
/// Get byte from rip
static inline uint8_t cpu_rip_byte(struct cpu_t *cpu)
{
	__DEBUG_EXEC(
	if (cpu->rip > cpu->mem_max || cpu->rip < cpu->mem_min) {
		cpu->trap = TRAP_ERROR_MEMORY;
		assert(0);
		return 0;
	})
	register uint8_t cmd = *(uint8_t*) cpu->rip;
	cpu->rip = (uint8_t*) cpu->rip + 1;
	return cmd;
}

/// Get qword from rip
static inline uint64_t cpu_rip_qword(struct cpu_t *cpu)
{
	__DEBUG_EXEC(
	if (cpu->rip > cpu->mem_max || cpu->rip < cpu->mem_min) {
		cpu->trap = TRAP_ERROR_MEMORY;
		assert(0);
		return 0;
	})
	register uint64_t cmd = *(uint64_t*) cpu->rip;
	cpu->rip = (uint64_t*) cpu->rip + 1;
	return cmd;
}
static inline void cpu_newrip_write_byte(struct cpu_t *cpu, uint8_t val)
{
	*(uint8_t*)cpu->newrip = val;
	cpu->newrip = (uint8_t*) cpu->newrip + 1;
}
static inline void cpu_newrip_write_dword(struct cpu_t *cpu, uint32_t val)
{
	*(uint32_t*)cpu->newrip = val;
	cpu->newrip = (uint32_t*) cpu->newrip + 1;
}
static inline void cpu_newrip_write_qword(struct cpu_t *cpu, uint64_t val)
{
	*(uint64_t*)cpu->newrip = val;
	cpu->newrip = (uint64_t*) cpu->newrip + 1;
}
/// Protected write for cpu
static inline void cpu_write(struct cpu_t *cpu, void *ptr, uint64_t val)
{
	__DEBUG_EXEC(
	if (ptr > cpu->mem_max || ptr < cpu->mem_min) {
		cpu->trap = TRAP_ERROR_MEMORY;
		assert(0);
	})
	*(uint64_t*) ptr = val;
}

/// Protected read for cpu
static inline uint64_t cpu_read(struct cpu_t *cpu, void *ptr)
{
	__DEBUG_EXEC(
	if (ptr > cpu->mem_max || ptr < cpu->mem_min) {
		cpu->trap = TRAP_ERROR_MEMORY;
		assert(0);
		return 0;
	})
	return *(uint64_t*) ptr;
}

/// Shell for cpu-push (not a cmd)
static inline void cpu_push(struct cpu_t *cpu, uint64_t val)
{
	cpu->reg[REG_rsp] -= sizeof(uint64_t);
	cpu_write(cpu, (void*) cpu->reg[REG_rsp], val);
}

/// Shell for cpu-pop (not a cmd)
static inline uint64_t cpu_pop(struct cpu_t *cpu)
{
	register uint64_t val = cpu_read(cpu, (void*) cpu->reg[REG_rsp]);
	cpu->reg[REG_rsp] += sizeof(uint64_t);
	return val;
}
/// Syscall handler
int cpu_syscall(struct cpu_t *cpu)
{
	switch (cpu->reg[REG_rax]) {
	case SYSCALL_EXIT:
	case SYSCALL_OUTQ:
	case SYSCALL_INPQ:
		cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);
		cpu_newrip_write_byte(cpu, 0xff);//syscall reg
		cpu_newrip_write_byte(cpu, X86_64_SYSCALL_REG + REG_r12);
		cpu->trap = TRAP_NO_TRAP;
		break;
	default:
		cpu->trap = TRAP_ERROR_SYSCALL;
		assert(0);
		return -1;
	}
	return 0;
}

long cpu_run(struct cpu_t *cpu)
{
	long cpu_cmd_count = 0;
	while (1) {
		// Handle cpu-traps
		switch (cpu->trap) {
		case TRAP_NO_TRAP:
			break;
		case TRAP_EXIT:
			return cpu_cmd_count;
		case TRAP_SYSCALL:
			if (cpu_syscall(cpu) == -1) {
				assert(0);
				return -1;
			}
			continue;
		default:
			fprintf(stderr, "Unknown trap: %lu\n", (unsigned long) cpu->trap);
			assert(0);
			return -1;
		}

		#ifdef NDEBUG
		#define DEF_CMD(name, type)			\
		case CMD_##name:					\
			cpu_cmd_##name(cpu);			\
			cpu_cmd_count++;				\
			break;
		#else
		#define DEF_CMD(name, type)			\
		case CMD_##name:					\
			fprintf(stderr, "Exec: " #name "\n");	\
			cpu_cmd_##name(cpu);			\
			break;
		#endif

		uint8_t cmd = cpu_rip_byte(cpu);
		if (cpu->trap == TRAP_ERROR_MEMORY) {
			assert(0);
			return -1;
		}

		switch (cmd) {
		#include "cmd_list.h"
		#undef DEF_CMD
		default:
			cpu->trap = TRAP_ERROR_INSTR;
			assert(0);
			return -1;
		}
	}
	return -1;
}

static inline void cpu_cmd_nop(struct cpu_t *cpu)
{
	// nop
}

static inline void cpu_cmd_syscall(struct cpu_t *cpu)
{
	//cpu_newrip_write_byte(cpu, X86_64_SYSCALL1);
	//cpu_newrip_write_byte(cpu, X86_64_SYSCALL2);
	cpu->trap = TRAP_SYSCALL;
}

static inline void cpu_cmd_pushq(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_PUSHQ);
	cpu_newrip_write_dword(cpu, cpu_rip_qword(cpu));
}

static inline void cpu_cmd_pushr(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_PUSHR + cpu_rip_byte(cpu));
}

static inline void cpu_cmd_pushm(struct cpu_t *cpu)
{
	cpu_push(cpu, *(uint64_t*)cpu_rip_qword(cpu));
}

static inline void cpu_cmd_pushmr(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_PUSHMR);
	cpu_newrip_write_byte(cpu, cpu_rip_byte(cpu) + X86_64_PUSHMR_REG);
	cpu_newrip_write_byte(cpu, cpu_rip_qword(cpu));
}

static inline void cpu_cmd_popr(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_POPR + cpu_rip_byte(cpu));
}

static inline void cpu_cmd_popm(struct cpu_t *cpu)
{
	*(uint64_t*)cpu_rip_qword(cpu) = cpu_pop(cpu);
}

static inline void cpu_cmd_popmr(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_POPMR);
	cpu_newrip_write_byte(cpu, X86_64_POPMR_REG + cpu_rip_byte(cpu));
	cpu_newrip_write_byte(cpu, cpu_rip_qword(cpu));
}
/*
static inline void cpu_cmd_add(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_ADD_SUB);
	cpu_newrip_write_byte(cpu, cpu_rip_byte(cpu) + X86_64_ASCM_REG);
	cpu_newrip_write_dword(cpu, cpu_rip_qword(cpu));
}*/
static inline void cpu_cmd_add(struct cpu_t *cpu)
{
	ARITH_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE_EXT);
	cpu_newrip_write_byte(cpu, X86_64_ADDR);
	cpu_newrip_write_byte(cpu, (REG_r10 - REG_r8) + ((REG_r11 - REG_r8) << 3) + X86_64_ASCM_REG);
	cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);
	cpu_newrip_write_byte(cpu, X86_64_PUSHR + (REG_r10 - REG_r8));
}
static inline void cpu_cmd_addr(struct cpu_t *cpu)
{
	uint8_t reg1 = cpu_rip_byte(cpu);
	uint8_t reg2 = cpu_rip_byte(cpu);	 
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_ADDR);
	cpu_newrip_write_byte(cpu, reg1 + (reg2 << 3) + X86_64_ASCM_REG);
}
static inline void cpu_cmd_sub(struct cpu_t *cpu)
{
	ARITH_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE_EXT);
	cpu_newrip_write_byte(cpu, X86_64_SUBR);
	cpu_newrip_write_byte(cpu, (REG_r10 - REG_r8) + ((REG_r11 - REG_r8) << 3) + X86_64_ASCM_REG);
	cpu_newrip_write_byte(cpu, X86_64_EXT_CMD);
	cpu_newrip_write_byte(cpu, X86_64_PUSHR + (REG_r10 - REG_r8));
}
static inline void cpu_cmd_subr(struct cpu_t *cpu)
{
	uint8_t reg1 = cpu_rip_byte(cpu);
	uint8_t reg2 = cpu_rip_byte(cpu);	 
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_SUBR);
	cpu_newrip_write_byte(cpu, reg1 + (reg2 << 3) + X86_64_ASCM_REG);
}
static inline void cpu_cmd_mul(struct cpu_t *cpu) {}
static inline void cpu_cmd_div(struct cpu_t *cpu) {}
//not working
/*
static inline void cpu_cmd_mul(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_MUL_DIV);
	cpu_newrip_write_byte(cpu, cpu_rip_byte(cpu) + X86_64_MUL_REG);
}

static inline void cpu_cmd_div(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_MUL_DIV);
	cpu_newrip_write_byte(cpu, cpu_rip_byte(cpu) + X86_64_DIV_REG);
}
*/
static inline void cpu_cmd_jmp(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JMP);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JMP ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}

static inline void cpu_cmd_call(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_CALL);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_dword(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 4);
		__DEBUG_EXEC(printf("CALL ADRESS: %llu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_dword(cpu, 15 * X86_64_NOP);
	}
}

static inline void cpu_cmd_ret(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_RET);
}

static inline void cpu_cmd_movrr(struct cpu_t *cpu)
{
	uint8_t reg1 = cpu_rip_byte(cpu);
	uint8_t reg2 = cpu_rip_byte(cpu);	 
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_MOVRR);
	cpu_newrip_write_byte(cpu, reg1 + (reg2 << 3) + X86_64_ASCM_REG);
}

static inline void cpu_cmd_movq(struct cpu_t *cpu)
{
	uint64_t temp1 = cpu_rip_byte(cpu);
	uint64_t temp2 = cpu_rip_qword(cpu);
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, temp1 + X86_64_MOVQ_REG);
	cpu_newrip_write_qword(cpu, temp2);
	cpu->reg[temp1] = temp2;
}
static inline void cpu_cmd_movmrr(struct cpu_t *cpu)
{
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_MOVRR);
	uint8_t reg1 = cpu_rip_byte(cpu);
	uint64_t val = cpu_rip_qword(cpu);
	uint8_t reg2 = cpu_rip_byte(cpu);
	cpu_newrip_write_byte(cpu, X86_64_MOVMEM_REG + reg1 + (reg2 << 3));
	cpu_newrip_write_byte(cpu, val);
}
static inline void cpu_cmd_movrmr(struct cpu_t *cpu)
{
	uint8_t reg1 = cpu_rip_byte(cpu);
	uint8_t reg2 = cpu_rip_byte(cpu);
	cpu_newrip_write_byte(cpu, X86_64_OPERAND_SIZE);
	cpu_newrip_write_byte(cpu, X86_64_MOVRMR);
	cpu_newrip_write_byte(cpu, X86_64_MOVMEM_REG + (reg1 << 3) + reg2);
	cpu_newrip_write_byte(cpu, cpu_rip_qword(cpu));
}
static inline void cpu_cmd_jl(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JL);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JL ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}

static inline void cpu_cmd_jg(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JG);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JG ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}

static inline void cpu_cmd_jle(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JLE);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JLE ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}

static inline void cpu_cmd_jge(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JGE);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JGE ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}

static inline void cpu_cmd_jeq(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JE);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JEQ ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}

static inline void cpu_cmd_jne(struct cpu_t *cpu)
{
	JMP_PREPARE;
	cpu_newrip_write_byte(cpu, X86_64_JNE);
	uint64_t tmp = cpu_rip_qword(cpu);
	if (lbl[tmp].isdefined) {
		cpu_newrip_write_byte(cpu, lbl[tmp].offs - reinterpret_cast<size_t>(cpu->newrip) - 1);
		__DEBUG_EXEC(printf("JNE ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	} else {
		cpu_newrip_write_byte(cpu, X86_64_NOP);
	}
}
static inline void cpu_cmd_procstop(struct cpu_t *cpu)
{
	cpu->trap = TRAP_EXIT;
}
static inline void cpu_cmd__LABEL(struct cpu_t *cpu)
{
	uint64_t tmp = cpu_rip_qword(cpu);
	lbl[tmp].offs = reinterpret_cast<size_t>(cpu->newrip);
	__DEBUG_EXEC(printf("LABEL ADRESS: %lu\n", reinterpret_cast<size_t>(cpu->newrip)));
	lbl[tmp].isdefined = 1;
}