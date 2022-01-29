/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

/* —————————————————————————————— */
/*					TODO 全局变量声明							*/
/* —————————————————————————————— */

/* 最大读者数													*/
extern int MAX_READER;

/* 打印相关														*/

/* 记录输出行数，屏幕最多显示25行，输出26行清屏					*/
extern int	disp_line;
/* 打印颜色控制													*/
extern int	COLORS[];
/* 解决disp_str输出的问题										*/
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
   信号量相关声明
   rw			共享文件互斥量,实现对文件的互斥访问，用于读写、写写互斥
   sread		读者互斥量,实现对 reader_num 的互斥访问
   reader_num	读者数
   swrite		写者互斥量,实现对 writer_num 的互斥访问
   writer_num	写者数

   max_reader	文件最大读者数
   write_first	用于写优先,写优先互斥信号量，当至少有一个写进程准备访问数据区时，用于禁止所有的读进程
   equal_S		用于读写公平、无饿死的读写互斥锁
   sem_table	信号量表(便于初始化所有信号量)
   SEM_NUM		信号量数
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


/* —————————————————————————————— */
/*							无修改								*/
/* —————————————————————————————— */
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