/*!
 * \brief cpu_dasm.c
 * \author Exdev
 * \version 0.5
 */

#include "cpu_t.h"
#include <ctype.h>
#include <string.h>

int put_cmd(char **text_p, uint8_t **code)
{
	uint8_t cmd = 0;
	cmd = **code;
	*code += sizeof(uint8_t);
#define DEF_CMD(name, val) if(cmd == CMD_##name)	\
	{	\
		strcat(*text_p, #name" ");	\
		return val;	\
	}
#include "cmd_list.h"
#undef DEF_CMD
	return -1;
}

int put_reg(char **text_p, uint8_t **code_p)
{
	uint8_t reg = 0;
	reg = **code_p;
	*code_p += sizeof(uint8_t);
#define DEF_REG(name) if (reg == REG_##name)	\
	{	\
		strcat(*text_p, #name" ");	\
		return 3;	\
	}
#include "reg_list.h"
#undef DEF_REG
	return -1;
}

int put_num(char **text_p, uint8_t **code_p)
{
	double num = 0;
	num = **code_p;
	char *numstr = NULL;
	numstr = (char*)calloc(64, sizeof(char));
	*code_p += sizeof(double);
	if(num - (long long int)num != 0)
	{
		sprintf(numstr, " %lf", num);
		strcat(*text_p, numstr);
		return 1;
	}
	if (num - (long long int)num == 0)
	{
		sprintf(numstr, " %.0lf", num);
		strcat(*text_p, numstr);
		return 1;
	}
	return -1;
}

//doesn't work with labels
long code_dasm(uint8_t *code, char **text_p, ssize_t size)
{
	uint8_t exp = 1;
	for (ssize_t i = 0; i < size; i++)
	{
		switch (exp)
		{
			case 1:
				exp = put_cmd(text_p, &code);
				break;
			case 2:
				exp = put_reg(text_p, &code);
				break;
			case 3:
				exp = put_num(text_p, &code);
				break;
			default:
				return -1;
				break;
		}
	}
	return 0;
}
