#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/* —————————————————————————————— */
/*						TODO 全局变量定义						*/
/* —————————————————————————————— */

/* 最大读者数													*/
PUBLIC int MAX_READER = 3;

/*	用户自定义的系统调用表										*/
PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = { sys_get_ticks,sys_sleep,sys_print,sys_P,sys_V };

PUBLIC	TASK	task_table[NR_TASKS] = {
					{reader, STACK_SIZE_A, "A"},
					{reader, STACK_SIZE_B, "B"},
					{reader, STACK_SIZE_C, "C"},
					{writer, STACK_SIZE_D, "D"},
					{writer, STACK_SIZE_E, "E"},
					{normal, STACK_SIZE_F, "F"}, };


/* 
——————————————打印相关——————————————

COLORS		输出颜色控制;红色,黄色,淡紫色,绿色,青色？,白色
disp_line	输出行数,超过25行在clear()你清屏
用char[]解决disp_str和disp_color输出乱码的问题
*/

PUBLIC int COLORS[] = { 0x0C,0x0E,0x0D,0x0A,0x0B,0x07 };
PUBLIC int disp_line = 0;
PUBLIC char READER[] = "reader ";
PUBLIC char READ[] = "read ";
extern char READING[] = " reading ";
PUBLIC char WRITER[] = "writer ";
PUBLIC char WRITE[] = "write ";
PUBLIC char WRITING[] = " writing ";
PUBLIC char ARRIVE[] = " arrive ";
PUBLIC char ING[] = " ing  ";
PUBLIC char END[] = " end ";
PUBLIC char CRLF[] = { '\n'};
PUBLIC char WHITESPACE[] = " ";
PUBLIC char* NUM[] = { " 0", " 1", " 2", " 3" };



/*  信号量相关的全局变量定义,具体说明在global.h					*/
PUBLIC SEMAPHORE rw;
PUBLIC SEMAPHORE sread;
PUBLIC SEMAPHORE swrite;
PUBLIC SEMAPHORE write_first;
PUBLIC SEMAPHORE equal_S;
PUBLIC SEMAPHORE max_reader;

PUBLIC u32	reader_num;
PUBLIC u32	writer_num;

PUBLIC SEMAPHORE* sem_table[] = {&rw,&sread,&swrite,&write_first,&equal_S,&max_reader};
PUBLIC u32	SEM_NUM = 6;

/* —————————————————————————————— */
/*								无修改							*/
/* —————————————————————————————— */

PUBLIC	PROCESS		proc_table[NR_TASKS];
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];
PUBLIC	TTY			tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];
PUBLIC	irq_handler	irq_table[NR_IRQ];