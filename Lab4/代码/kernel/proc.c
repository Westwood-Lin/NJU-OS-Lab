#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
						   sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks(){
	return ticks;
}


/* TODO Lab4 新增函数定义↓ */

/*======================================================================*
                              schedule 调度器
 *======================================================================*/
PUBLIC void schedule() {
	while (1) {
		if (p_proc_ready >= proc_table + NR_TASKS) {
			p_proc_ready = proc_table;
		}

		//如果不是休眠进程和被阻塞进程，就可以直接break，选中执行
		if (p_proc_ready->sleep_ticks <= 0 && p_proc_ready->blocked != 1) {
			break;
		}
		//进程按序轮换
		p_proc_ready++;
	}
}


/*======================================================================*
						   sys_sleep 休眠
 *======================================================================*/
PUBLIC void sys_sleep(int milli_seconds){
	// ticks * 1000 / HZ ≈ milli 毫秒; 通过milli，推出ticks
	int ticks = milli_seconds * HZ / 1000; 

	p_proc_ready->sleep_ticks = ticks;

	//直接开始调度,把时间片分配给其他进程
	schedule();
}


/* 清屏,但是没有设置游标,本次实验也没有要求游标跟随输出,可以偷懒 */
/*======================================================================*
						   clear()
 *======================================================================*/
PUBLIC void clear() {
	u8* pVmem = (u8*)V_MEM_BASE;//显存起始处
	for (; pVmem < (u8*)(V_MEM_BASE + V_MEM_SIZE);) {
		*pVmem++ = ' ';
		*pVmem++ = DEFAULT_CHAR_COLOR;
	}
	disp_pos = 0;
	disp_line = 0;
}

/*======================================================================*
						  sys_print 封装disp_str
 *======================================================================*/
PUBLIC void sys_print(char* str) {
	disp_str(str);
}


/*======================================================================*
						  sys_P sys_V 原语操作
 *======================================================================*/
PUBLIC void sys_P(SEMAPHORE* s) {
	int tmp;
	disable_int();

	s->value--;
	tmp = s->value;
	if (s->value < 0) {
		//block();
		s->queue[s->tail] = p_proc_ready;
		p_proc_ready->blocked = 1;
		s->tail = (s->tail + 1) % SEMAPHORE_QUEUE_SIZE;
	}

	enable_int();
	if (tmp < 0)
		schedule();
}

PUBLIC void sys_V(SEMAPHORE* s) {
	disable_int();

	s->value++;
	if (s->value <= 0) {
		//wakeup();
		PROCESS* p = s->queue[s->head];
		p->blocked = 0;
		s->head = (s->head + 1) % SEMAPHORE_QUEUE_SIZE;
	}

	enable_int();
}

/*======================================================================*
						  sleep_random 休眠一个伪随机时间
 *======================================================================*/
PUBLIC void sleep_random(void) {
	int sleep_time_array[] = { 20,25,30,35,40 };
	sleep(sleep_time_array[get_ticks() % 5]);
}

/*======================================================================*
						  reader_first_R  reader_first_W 读者优先
 *======================================================================*/
PUBLIC void reader_first_R() {
	while (1) {
		print_reader_arrive(p_proc_ready);
		/*  1. read_num++,第一个读进程加 rw锁							*/
		P(&sread);
		if (reader_num == 0)
			P(&rw);
		reader_num++;
		V(&sread);

		/*  2. 阻塞在最大读者锁											*/
		P(&max_reader);

		/*  3. 读文件(打印)、消耗时间片											*/
		p_proc_ready->running = 1;
		print_reading(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  4. 释放最大读者锁，read_num--,最后一个读进程释放 rw锁		*/
		V(&max_reader);
		P(&sread);
		reader_num--;
		if (reader_num == 0)
			V(&rw);
		V(&sread);

		p_proc_ready->running = 0;
		print_reader_end(p_proc_ready);

		/*  5. sleep													*/
		sleep_random();
	}
}

PUBLIC void reader_first_W() {
	while (1) {
		print_writer_arrive(p_proc_ready);
		/*  1. 加 rw锁													*/
		P(&rw);

		/*  2. 打印、消耗时间片											*/
		p_proc_ready->running = 1;
		print_writing(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  3. 释放 rw锁												*/
		V(&rw);

		p_proc_ready->running = 0;
		print_writer_end(p_proc_ready);

		/*  4. sleep													*/
		sleep_random();
	}
}

/*======================================================================*
						writer_first_R  writer_first_W 写者优先
 *======================================================================*/
PUBLIC void writer_first_R() {
	while (1) {
		print_reader_arrive(p_proc_ready);

		/*  1. 阻塞在写优先锁上											*/
		P(&write_first);

		/*  2. read_num++,第一个读进程加 rw锁							*/
		P(&sread);
		if (reader_num == 0)
			P(&rw);
		reader_num++;
		V(&sread);

		V(&write_first);
		/*  3. 阻塞在最大读者锁											*/
		P(&max_reader);

		/*  4. 读文件(打印)、消耗时间片									*/
		p_proc_ready->running = 1;
		print_reading(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  5. 释放最大读者锁，read_num--,最后一个读进程释放 rw锁		*/
		V(&max_reader);
		P(&sread);
		reader_num--;
		if (reader_num == 0)
			V(&rw);
		V(&sread);

		p_proc_ready->running = 0;
		print_reader_end(p_proc_ready);
		/*  6. sleep													*/
		sleep_random();
	}
}

PUBLIC void writer_first_W() {
	while (1) {
		print_writer_arrive(p_proc_ready);
		/*  1. writer_num++、第一个写进程加写优先锁						*/
		P(&swrite);
		if (writer_num == 0)
			P(&write_first);
		writer_num++;
		V(&swrite);

		/*  2. 写入文件(打印)、消耗时间片								*/
		P(&rw);
		p_proc_ready->running = 1;
		print_writing(p_proc_ready);
		sleep(p_proc_ready->ticks);
		V(&rw);

		/*  3. writer_num--、最后一个写进程释放写优先锁					*/
		P(&swrite);
		writer_num--;
		if (writer_num == 0)
			V(&write_first);
		V(&swrite);

		p_proc_ready->running = 0;
		print_writer_end(p_proc_ready);
		/*  4. sleep													*/
		sleep_random();
	}
}

/*======================================================================*
						no_hungry_R  no_hungry_W 没有饿死
 *======================================================================*/
PUBLIC void no_hungry_R() {
	while (1) {
		print_reader_arrive(p_proc_ready);

		/*  1. 加互斥锁S,read_num++,第一个读进程加 rw锁					*/
		P(&equal_S);
		P(&sread);
		if (reader_num == 0)
			P(&rw);
		reader_num++;
		V(&sread);
		V(&equal_S);

		/*  2. 阻塞在最大读者锁											*/
		P(&max_reader);

		/*  3. 读文件(打印)、消耗时间片									*/
		p_proc_ready->running = 1;
		print_reading(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  4. 释放最大读者锁，read_num--,最后一个读进程释放 rw锁		*/
		V(&max_reader);
		P(&sread);
		reader_num--;
		if (reader_num == 0)
			V(&rw);
		V(&sread);

		p_proc_ready->running = 0;
		print_reader_end(p_proc_ready);
		/*  5. sleep													*/
		sleep_random();
	}
}

PUBLIC void no_hungry_W() {
	while (1) {
		print_writer_arrive(p_proc_ready);

		/*  1. 加 rw锁 和公平的S锁										*/
		P(&equal_S);
		P(&rw);

		/*  2. 打印、消耗时间片											*/
		p_proc_ready->running = 1;
		print_writing(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  3. 释放 rw锁和S锁											*/
		V(&rw);
		V(&equal_S);

		p_proc_ready->running = 0;
		print_writer_end(p_proc_ready);
		/*  4. sleep													*/
		sleep_random();
	}
}

/*======================================================================*
						打印方法
 *======================================================================*/
PUBLIC void print_reader_arrive(PROCESS* p) {
	//reader name begin '\n'
	disp_color_str(READER, p->color);	//reader
	disp_color_str(p->p_name, p->color);//名字
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(ARRIVE, p->color);	//arrive,到达
	disp_color_str(CRLF, p->color);		//换行
	disp_line++;
}

PUBLIC void print_reader_end(PROCESS* p) {
	//reader name end '\n'
	disp_color_str(READER, p->color);
	disp_color_str(p->p_name, p->color);
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(END, p->color);
	disp_color_str(CRLF, p->color);
	disp_line++;
	
}

PUBLIC void print_reading(PROCESS* p) {
	//reader name reading '\n'
	disp_color_str(READER, p->color);
	disp_color_str(p->p_name, p->color);
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(READING, p->color);
	disp_color_str(CRLF, p->color);
	disp_line++;
}

PUBLIC void print_writer_arrive(PROCESS* p) {
	//writer name begin '\n'
	disp_color_str(WRITER, p->color);
	disp_color_str(p->p_name, p->color);
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(ARRIVE, p->color);
	disp_color_str(CRLF, p->color);
	disp_line++;
}

PUBLIC void print_writer_end(PROCESS* p) {
	//writer name begin '\n'
	disp_color_str(WRITER, p->color);
	disp_color_str(p->p_name, p->color);
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(END, p->color);
	disp_color_str(CRLF, p->color);
	disp_line++;
}

PUBLIC void print_writing(PROCESS* p) {
	//writer name begin '\n'
	disp_color_str(WRITER, p->color);
	disp_color_str(p->p_name, p->color);
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(WRITING, p->color);
	disp_color_str(CRLF, p->color);
	disp_line++;
}