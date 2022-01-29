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


/* TODO Lab4 ������������� */

/*======================================================================*
                              schedule ������
 *======================================================================*/
PUBLIC void schedule() {
	while (1) {
		if (p_proc_ready >= proc_table + NR_TASKS) {
			p_proc_ready = proc_table;
		}

		//����������߽��̺ͱ��������̣��Ϳ���ֱ��break��ѡ��ִ��
		if (p_proc_ready->sleep_ticks <= 0 && p_proc_ready->blocked != 1) {
			break;
		}
		//���̰����ֻ�
		p_proc_ready++;
	}
}


/*======================================================================*
						   sys_sleep ����
 *======================================================================*/
PUBLIC void sys_sleep(int milli_seconds){
	// ticks * 1000 / HZ �� milli ����; ͨ��milli���Ƴ�ticks
	int ticks = milli_seconds * HZ / 1000; 

	p_proc_ready->sleep_ticks = ticks;

	//ֱ�ӿ�ʼ����,��ʱ��Ƭ�������������
	schedule();
}


/* ����,����û�������α�,����ʵ��Ҳû��Ҫ���α�������,����͵�� */
/*======================================================================*
						   clear()
 *======================================================================*/
PUBLIC void clear() {
	u8* pVmem = (u8*)V_MEM_BASE;//�Դ���ʼ��
	for (; pVmem < (u8*)(V_MEM_BASE + V_MEM_SIZE);) {
		*pVmem++ = ' ';
		*pVmem++ = DEFAULT_CHAR_COLOR;
	}
	disp_pos = 0;
	disp_line = 0;
}

/*======================================================================*
						  sys_print ��װdisp_str
 *======================================================================*/
PUBLIC void sys_print(char* str) {
	disp_str(str);
}


/*======================================================================*
						  sys_P sys_V ԭ�����
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
						  sleep_random ����һ��α���ʱ��
 *======================================================================*/
PUBLIC void sleep_random(void) {
	int sleep_time_array[] = { 20,25,30,35,40 };
	sleep(sleep_time_array[get_ticks() % 5]);
}

/*======================================================================*
						  reader_first_R  reader_first_W ��������
 *======================================================================*/
PUBLIC void reader_first_R() {
	while (1) {
		print_reader_arrive(p_proc_ready);
		/*  1. read_num++,��һ�������̼� rw��							*/
		P(&sread);
		if (reader_num == 0)
			P(&rw);
		reader_num++;
		V(&sread);

		/*  2. ��������������											*/
		P(&max_reader);

		/*  3. ���ļ�(��ӡ)������ʱ��Ƭ											*/
		p_proc_ready->running = 1;
		print_reading(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  4. �ͷ�����������read_num--,���һ���������ͷ� rw��		*/
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
		/*  1. �� rw��													*/
		P(&rw);

		/*  2. ��ӡ������ʱ��Ƭ											*/
		p_proc_ready->running = 1;
		print_writing(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  3. �ͷ� rw��												*/
		V(&rw);

		p_proc_ready->running = 0;
		print_writer_end(p_proc_ready);

		/*  4. sleep													*/
		sleep_random();
	}
}

/*======================================================================*
						writer_first_R  writer_first_W д������
 *======================================================================*/
PUBLIC void writer_first_R() {
	while (1) {
		print_reader_arrive(p_proc_ready);

		/*  1. ������д��������											*/
		P(&write_first);

		/*  2. read_num++,��һ�������̼� rw��							*/
		P(&sread);
		if (reader_num == 0)
			P(&rw);
		reader_num++;
		V(&sread);

		V(&write_first);
		/*  3. ��������������											*/
		P(&max_reader);

		/*  4. ���ļ�(��ӡ)������ʱ��Ƭ									*/
		p_proc_ready->running = 1;
		print_reading(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  5. �ͷ�����������read_num--,���һ���������ͷ� rw��		*/
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
		/*  1. writer_num++����һ��д���̼�д������						*/
		P(&swrite);
		if (writer_num == 0)
			P(&write_first);
		writer_num++;
		V(&swrite);

		/*  2. д���ļ�(��ӡ)������ʱ��Ƭ								*/
		P(&rw);
		p_proc_ready->running = 1;
		print_writing(p_proc_ready);
		sleep(p_proc_ready->ticks);
		V(&rw);

		/*  3. writer_num--�����һ��д�����ͷ�д������					*/
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
						no_hungry_R  no_hungry_W û�ж���
 *======================================================================*/
PUBLIC void no_hungry_R() {
	while (1) {
		print_reader_arrive(p_proc_ready);

		/*  1. �ӻ�����S,read_num++,��һ�������̼� rw��					*/
		P(&equal_S);
		P(&sread);
		if (reader_num == 0)
			P(&rw);
		reader_num++;
		V(&sread);
		V(&equal_S);

		/*  2. ��������������											*/
		P(&max_reader);

		/*  3. ���ļ�(��ӡ)������ʱ��Ƭ									*/
		p_proc_ready->running = 1;
		print_reading(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  4. �ͷ�����������read_num--,���һ���������ͷ� rw��		*/
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

		/*  1. �� rw�� �͹�ƽ��S��										*/
		P(&equal_S);
		P(&rw);

		/*  2. ��ӡ������ʱ��Ƭ											*/
		p_proc_ready->running = 1;
		print_writing(p_proc_ready);
		sleep(p_proc_ready->ticks);

		/*  3. �ͷ� rw����S��											*/
		V(&rw);
		V(&equal_S);

		p_proc_ready->running = 0;
		print_writer_end(p_proc_ready);
		/*  4. sleep													*/
		sleep_random();
	}
}

/*======================================================================*
						��ӡ����
 *======================================================================*/
PUBLIC void print_reader_arrive(PROCESS* p) {
	//reader name begin '\n'
	disp_color_str(READER, p->color);	//reader
	disp_color_str(p->p_name, p->color);//����
	disp_color_str(WHITESPACE, p->color);
	disp_color_str(ARRIVE, p->color);	//arrive,����
	disp_color_str(CRLF, p->color);		//����
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