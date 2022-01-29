/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

/* ������������������������������������������������������������ */
/*					TODO ȫ�ֱ�������							*/
/* ������������������������������������������������������������ */

/* ��������													*/
extern int MAX_READER;

/* ��ӡ���														*/

/* ��¼�����������Ļ�����ʾ25�У����26������					*/
extern int	disp_line;
/* ��ӡ��ɫ����													*/
extern int	COLORS[];
/* ���disp_str���������										*/
extern char READER[];
extern char READ[];
extern char READING[];
extern char WRITER[];
extern char WRITE[];
extern char WRITING[];
extern char ARRIVE[];
extern char END[];
extern char CRLF[];
extern char WHITESPACE[];
extern char* NUM[];

/*
   �ź����������
   rw			�����ļ�������,ʵ�ֶ��ļ��Ļ�����ʣ����ڶ�д��дд����
   sread		���߻�����,ʵ�ֶ� reader_num �Ļ������
   reader_num	������
   swrite		д�߻�����,ʵ�ֶ� writer_num �Ļ������
   writer_num	д����

   max_reader	�ļ���������
   write_first	����д����,д���Ȼ����ź�������������һ��д����׼������������ʱ�����ڽ�ֹ���еĶ�����
   equal_S		���ڶ�д��ƽ���޶����Ķ�д������
   sem_table	�ź�����(���ڳ�ʼ�������ź���)
   SEM_NUM		�ź�����
																		*/

extern SEMAPHORE rw;
extern SEMAPHORE sread;
extern SEMAPHORE swrite;
extern SEMAPHORE max_reader;
extern SEMAPHORE write_first;
extern SEMAPHORE equal_S;

extern SEMAPHORE* sem_table[];

extern u32 reader_num;
extern u32 writer_num;
extern u32 SEM_NUM;


/* ������������������������������������������������������������ */
/*							���޸�								*/
/* ������������������������������������������������������������ */
EXTERN	int			ticks;
EXTERN	int			disp_pos;
EXTERN	u8			gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8			idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];

EXTERN	u32			k_reenter;
EXTERN	TSS			tss;
EXTERN	PROCESS* p_proc_ready;
EXTERN	int			nr_current_console;
extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK        task_table[];
extern	irq_handler	irq_table[];
extern	TTY			tty_table[];
extern  CONSOLE     console_table[];