/*!
 * \brief cmd_list.h
 * \author Exdev
 * \version 1.0.0
 * \brief Format : DEF_CMD(name, value)
 * value = 1 -> next act = get cmd
 * value = 2 -> next act = get reg
 * value = 3 -> next act = get number
 * value = 4 -> next act = get label
 */


DEF_CMD(nop, 1) 	//0
DEF_CMD(syscall, 1) //1
DEF_CMD(pushmr, 2) 	//2
DEF_CMD(pushq, 3) 	//3
DEF_CMD(pushr, 2) 	//4
DEF_CMD(pushm, 3) 	//5
DEF_CMD(popmr, 2) 	//6
DEF_CMD(popr, 2) 	//7
DEF_CMD(popm, 3) 	//8
DEF_CMD(addr, 1) 	//9
DEF_CMD(add, 1) 	//10
DEF_CMD(subr, 1) 	//11
DEF_CMD(sub, 1) 	//12
DEF_CMD(mul, 1) 	//13
DEF_CMD(call, 4) 	//14
DEF_CMD(ret, 1) 	//15 //0f
DEF_CMD(movmrr, 2)	//16 //10
DEF_CMD(movrmr, 2)	//17 //11
DEF_CMD(movrr, 2) 	//18 //12
DEF_CMD(movq, 2) 	//19 //13
DEF_CMD(jge, 4) 	//20
DEF_CMD(jg, 4) 		//
DEF_CMD(jle, 4) 	//
DEF_CMD(jl, 4) 		//
DEF_CMD(jeq, 4) 	//
DEF_CMD(jne, 4) 	//
DEF_CMD(exit, 1) 	//
DEF_CMD(div, 1) 	//
DEF_CMD(jmp, 4) 	//
