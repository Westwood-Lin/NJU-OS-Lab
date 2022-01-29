/*  
*   TODO 新函数声明
*   main.c        进程函数体
*   proc.c        系统调用
*   syscall.asm   系统调用
*/
//通过宏控制是reader_first_R、writer_first_R、no_hungry_R
PUBLIC void reader();
PUBLIC void writer();
PUBLIC void normal();

PUBLIC void clear();

PUBLIC void reader_first_R();
PUBLIC void reader_first_W();
PUBLIC void writer_first_R();
PUBLIC void writer_first_W();
PUBLIC void no_hungry_R();
PUBLIC void no_hungry_W();

PUBLIC void print_reader_arrive(PROCESS* p);
PUBLIC void print_writer_arrive(PROCESS* p);
PUBLIC void print_reader_end(PROCESS* p);
PUBLIC void print_writer_end(PROCESS* p);
PUBLIC void print_reading(PROCESS* p);
PUBLIC void print_writing(PROCESS* p);

PUBLIC void sleep_random();
PUBLIC void milli_delay(int);

/* 系统调用声明 */

/* 内核态 */
PUBLIC  int     sys_get_ticks();                /* 获得ticks        */
PUBLIC  void    sys_sleep(int milli_seconds);   /* 进程休眠(内核)   */
PUBLIC  void    sys_print(char*);               /* 包装disp_str     */
PUBLIC  void    sys_P(SEMAPHORE*);              /* P操作            */
PUBLIC  void    sys_V(SEMAPHORE*);              /* V操作            */

/* 用户态 */
PUBLIC  void    sys_call();                     /* int_handler      */
PUBLIC  int     get_ticks();
PUBLIC  void    sleep(int milli_seconds);       /* 进程休眠(用户级) */
PUBLIC  void    my_print(char*);                /* 封装disp_str     */
PUBLIC  void    P(SEMAPHORE*);                  /* P操作            */
PUBLIC  void    V(SEMAPHORE*);                  /* V操作            */

/* ———————————————————————————————— */
/*                           无修改                                 */
/* ———————————————————————————————— */

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char* info);
PUBLIC void	disp_color_str(char* info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);