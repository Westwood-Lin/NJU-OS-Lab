
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_


/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */

	/* TODO New 支持查找模式增加的新成员 */
	unsigned int search_mod;
	unsigned int search_len;
	/* End New */
}CONSOLE;

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */

//TODO New 查找模式字串颜色、查找模式状态、回车空格的颜色
#define ESC_COLOR 0x0C	/*  0000 1100 黑底红字 */
#define LINE_BS_COLOR 0x06	/*  0000 0110 黑底棕字 */
#define SEARCH_CLOSE 0
#define SEARCH_OPEN 1
#define SEARCH_DO 2

//End New

#endif /* _ORANGES_CONSOLE_H_ */
