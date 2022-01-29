#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
			TODO main.c 全局入口、进程体、读写优先的模式切换
 *======================================================================*/
/* 宏 0读者优先 1写者优先 2防止饿死										*/
#define READER_FIRST 0
#define WRITER_FIRST 1
#define NO_HUNGRY 2
#define WHICH_FIRST NO_HUNGRY 


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK* 	 p_task = task_table;
	PROCESS* p_proc = proc_table;
	char* 	 p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16		 selector_ldt = SELECTOR_LDT_FIRST;
	int i;

	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;						// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
			sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
			sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs = ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		/* 初始化睡眠tick 和 是否阻塞的值 */
		p_proc->sleep_ticks = 0;
		p_proc->blocked = 0;
		p_proc->color = COLORS[i];
		p_proc->running = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].type = proc_table[1].type = proc_table[2].type = READER_FIRST;//A\B\C reader
	proc_table[3].type = proc_table[4].type = WRITER_FIRST; //D\E writer
	proc_table[5].type = 2;//normal process F

	proc_table[0].ticks = 20;
	proc_table[1].ticks = proc_table[2].ticks = proc_table[3].ticks = 30;
	proc_table[4].ticks = 40;
	proc_table[5].ticks = 10;

	//init_all_semaphore()		//以下进行信号量的初始化
	for (int i = 0; i < SEM_NUM; i++) {
		SEMAPHORE* s = sem_table[i];
		s->value = 1;/* 互斥信号量,初始值为1									*/
		s->head = 0;
		s->tail = 0;
	}
	sem_table[SEM_NUM - 1]->value = MAX_READER; /* 最大读者数的value不一定是1	*/

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;
	clear();
	init_clock();
	init_keyboard();

	restart();

	while(1){}
}

/*======================================================================*
							   Reader A\B\C 读者
 *======================================================================*/
PUBLIC void reader() {
	switch (WHICH_FIRST) {
	case READER_FIRST:
		reader_first_R();
		break;

	case WRITER_FIRST:
		writer_first_R();
		break;

	case NO_HUNGRY:
		no_hungry_R();
		break;
	default:
		break;
	}
}


/*======================================================================*
							   Writer D\E   写者
 *======================================================================*/
PUBLIC void writer() {
	switch (WHICH_FIRST) {
		//0读者优先 1写者优先 2防止饿死
	case READER_FIRST:
		reader_first_W();
		break;

	case WRITER_FIRST:
		writer_first_W();
		break;

	case NO_HUNGRY:
		no_hungry_W();
		break;
	default:
		break;
	}
}

/*======================================================================*
							   Normal F
 *======================================================================*/
PUBLIC void normal() {
	while (1) {
		int has_running=0;
		PROCESS* p = proc_table;
		for (; p < proc_table + NR_TASKS; p++) {
			if (p->running == 1) {
				has_running = 1;
				break;
			}
		}
		if (has_running) {//有读或写进程在
			switch (p->type) {
				int reader_cnt;
				char F_output[] = { 'F',':',' ','\0'};
				char reader_cnt_output[] = { '\0','\0' };

			case READER_FIRST://reader在运行
				P(&sread);
				reader_cnt = reader_num;
				V(&sread);
				reader_cnt_output[0] = '0' + reader_cnt;

				//3 reader reading \n
				disp_color_str(reader_cnt_output, p_proc_ready->color);
				disp_color_str(WHITESPACE, p_proc_ready->color);
				disp_color_str(READER, p_proc_ready->color);
				disp_color_str(READING, p_proc_ready->color);
				disp_color_str(CRLF, p_proc_ready->color);
				disp_line++;
				break;

			case WRITER_FIRST://writer在运行
				//writer writing \n
				disp_color_str(WRITER, p_proc_ready->color);
				disp_color_str(p->p_name, p_proc_ready->color);
				disp_color_str(WHITESPACE, p_proc_ready->color);
				disp_color_str(WRITING, p_proc_ready->color);
				disp_color_str(CRLF, p_proc_ready->color);
				disp_line++;
				break;

			default://啥都不干，直接跳过
				break;
			}
		}

		sleep(p_proc_ready->ticks);
	}
}