/*!
 * \brief cpu_asm.c
 * \author Exdev
 * \version 0.5
 */

#include "cpu_t.h"
#include <ctype.h>
#include <string.h>

#define LABEL_MAX 16
struct label_t lbl[LABEL_MAX];

#define SKIP_SPACE	\
{	\
	while(isspace(*text))	\
	{	\
		text += sizeof(char);	\
	}	\
}	\

int get_cmd(char **text_p, uint8_t **code_p)
{
#define DEF_CMD(name, val) if (strncmp(*text_p, #name, sizeof(#name) - 1) == 0)	\
	{	\
		**(uint8_t**)code_p = CMD_##name;	\
		*code_p += sizeof(uint8_t);	\
		*text_p += sizeof(#name) - 1;	\
		return 0;	\
	}
#include "cmd_list.h"
#undef DEF_CMD
	return -1;
}

int get_reg(char **text_p, uint8_t **code_p)
{
#define DEF_REG(name) if (strncmp(*text_p, #name, sizeof(#name) - 1) == 0)	\
	{	\
		**(uint8_t**)code_p = REG_##name;	\
		*code_p += sizeof(uint8_t);	\
		*text_p += sizeof(#name) - 1;	\
		return 0;	\
	}
#include "reg_list.h"
#undef DEF_REG
	return -1;
}

int get_num(char **text_p, uint8_t **code_p)
{
	if (!isdigit(**text_p) && **text_p != '-')
	{
		return -1;
	}
	int dol = 0;
	int num = 0;
	char *text_end = NULL;
	while (!isspace(**text_p))
	{
		if(**text_p == '.')
		{
			dol = 1;
		}
		num += sizeof(char);
		*text_p += sizeof(char);
	}
	*text_p -= num;
	if (dol == 0)
	{
		**(uint64_t**)code_p = strtoll(*text_p, &text_end, 0);
		*code_p += sizeof(uint64_t);
		*text_p = text_end;
		return 1;
	}
	if (dol == 1)
	{
		**(double**)code_p = strtod(*text_p, &text_end);
		*code_p += sizeof(double);
		*text_p = text_end;
		return 1;
	}
	return -1;
}

int get_label(char **text_p, uint8_t **code_p)
{
	if(**text_p != ':')
	{
		return -1;
	}
	*text_p += sizeof(char);
	char *text_end = NULL;
	int temp = 0;
	temp = strtoll(*text_p, &text_end, 0);
	*text_p = text_end;
	lbl[temp].offs = (ssize_t)*code_p;
	lbl[temp].isdefined = 1;
	return 0;
}

int put_label(char **text_p, uint8_t **code_p)
{
#define DEF_CMD(name, val) if (val == 4 && strncmp(*text_p, #name, sizeof(#name) - 1) == 0)	\
	{	\
		**(uint8_t**)code_p = CMD_##name;	\
		*code_p += sizeof(uint8_t);	\
		*text_p += sizeof(#name) - 1;	\
		goto SKIP_CMD;	\
	} else
#include "cmd_list.h"
	return -1;
#undef DEF_CMD
	char *text_end = NULL;
	SKIP_CMD: text_end = NULL;
	int temp = 0;
	temp = strtoll(*text_p, &text_end, 0);
	*text_p = text_end;
	if (!lbl[temp].isdefined)
	{
		*code_p += sizeof(int64_t);
		return 1;
	}
	**(int64_t**)code_p = lbl[temp].offs - (ssize_t)*code_p;
	*code_p += sizeof(int64_t);
	return 0;
}

long code_asm (char *text, uint8_t **code_p)
{
	uint8_t *ip_begin = NULL;

	char *text_begin = NULL;

	for (int i = 0; i < 2; i++)
	{
		text_begin = text;
		ip_begin = *code_p;
		while(*text != '\0')
		{
			//printf("%s\n", text);
			SKIP_SPACE;
			get_label(&text, code_p);
			SKIP_SPACE;
			put_label(&text, code_p);
			SKIP_SPACE;
			get_cmd(&text, code_p);
			SKIP_SPACE;
			get_reg(&text, code_p);
			SKIP_SPACE;
			get_num(&text, code_p);
			//printf("\n--------------------------------------\n");
		}
		text = text_begin;
		*code_p = ip_begin;
	}
	return 0;
}
