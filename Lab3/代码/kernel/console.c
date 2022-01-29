
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

// TODO New 新方法声明和全局变量声明
PUBLIC void clear_console(CONSOLE* p_con);
PUBLIC void search(CONSOLE* p_con,int);
PRIVATE void change_color(u8* vmm_start, int chars, int COLOR);
PRIVATE void delete_word(CONSOLE* p_con, int chars);
extern void init_undo();
extern char undo_char;
extern u8 undo_content[2];
// End New

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	/* TODO New 初始化搜索模式 */
	p_tty->p_console->search_mod = SEARCH_CLOSE;
	p_tty->p_console->search_len = 0;
	/* End New */

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
	out_char(p_tty->p_console, nr_tty + '0');
	out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	/* TODO New 修改的重点函数 */

	//正在查找时，代表ESC '\f'外的所有输入都没用
	if (p_con->search_mod == SEARCH_DO && ch != '\f') {
		return;
	}

	switch (ch) {

		/*
		*  \t输出4个0x00,0x00不能显示,看上去就是空格,注意书上P265页图7.11;
		*  这个不显示的字符是一个标识
		*/
	case '\t':
		if (p_con->cursor <
			p_con->original_addr + p_con->v_mem_limit - 4) {
			for (int i = 0; i < 4; ++i) {
				//0x00不能显示,看上去就是空格
				*p_vmem++ = 0x00;
				*p_vmem++ = DEFAULT_CHAR_COLOR;
				p_con->cursor++;
			}

			//查找模式下输入Tab,长度其实是加4
			if (p_con->search_mod != SEARCH_CLOSE) {
				p_con->search_len += 4;
			}
		}
		break;

		// 我用\f代表key ESC
	case '\f':
		if (p_con->search_mod == SEARCH_CLOSE) {
			p_con->search_mod = SEARCH_OPEN;
		}
		else {
			p_con->search_mod = SEARCH_CLOSE;
			out_search(p_con, ESC_COLOR);
		}
		break;

		//用\v代表 ctrl+z 操作
	case '\v':
		//撤销删除
		if (undo_char == '\b') {
			if (undo_content[0] == '\t') {
				out_char(p_con, '\t');
			}
			else if (undo_content[0] == '\n') {
				out_char(p_con, '\n');
			}
			else {
				*p_vmem++ = undo_content[0];
				*p_vmem++ = undo_content[1];
				p_con->cursor++;
			}
		}
		else if (undo_char != 0) {
			out_char(p_con, '\b');
		}
		init_undo();
		break;

	case '\n':
		if (p_con->search_mod == SEARCH_OPEN) {
			p_con->search_mod = SEARCH_DO;
			search(p_con, ESC_COLOR);
		}
		else if (p_con->cursor < p_con->original_addr +
			p_con->v_mem_limit - SCREEN_WIDTH) {

			//用一种不同的颜色区分换行式空格
			p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
			int cursor_pos = p_con->cursor;
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
				((p_con->cursor - p_con->original_addr) /
					SCREEN_WIDTH + 1);
			//i是游标偏移量
			for (int i = 0; i + cursor_pos < p_con->cursor; i++) {
				p_vmem[0 + 2 * i] = ' ';
				p_vmem[1 + 2 * i] = LINE_BS_COLOR;
			}
		}
		break;

	case '\b':
		//判断是不是遇上了Tab类的空格,是的话连续删除4次
		if ((*(p_vmem - 2) == 0x00) &&
			(p_con->cursor >= p_con->original_addr + 4)) {
			for (int i = 0; i < 4; i++) {
				p_con->cursor--;
				*(p_vmem - 2 - i * 2) = ' ';
				*(p_vmem - 1 - i * 2) = DEFAULT_CHAR_COLOR;
			}
			//记录删除掉了Tab
			undo_content[0] = '\t';
		}
		//遇到\n
		else if ((*(p_vmem - 1) == LINE_BS_COLOR)) {
			while (*(p_vmem - 1) == LINE_BS_COLOR){
				*(p_vmem - 1) = DEFAULT_CHAR_COLOR;
				p_vmem -= 2;
				p_con->cursor--; 
			}
			//记录删除掉了换行
			undo_content[0] = '\n';
		}
		//下面的else-if，用以应对所有其他情况
		else if (p_con->cursor > p_con->original_addr) {
			//记录删除的内容
			undo_content[0] = *(p_vmem - 2);
			undo_content[1] = *(p_vmem - 1);

			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;
		}
		//End New
		break;

	default:
		if (p_con->cursor <
			p_con->original_addr + p_con->v_mem_limit - 1) {

			*p_vmem++ = ch;
			// 查找模式
			if (p_con->search_mod == SEARCH_OPEN) {
				*p_vmem++ = ESC_COLOR;
				p_con->search_len++;
			}

			// 兼容正常模式
			else {
				*p_vmem++ = DEFAULT_CHAR_COLOR;
			}
			p_con->cursor++;
		}
		break;
	}

	//End New

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

/*TODO New function : 清屏、搜索、改变颜色(从前往后)、删除一个字符(从后往前)
*======================================================================*
							clear_console
*======================================================================*
* 清除当前console的屏幕,然后重新显示所有输入
*/
PUBLIC void clear_console(CONSOLE* p_con)
{
	/* 如果在查找模式里，那就不清屏 */
	if (p_con->search_mod != SEARCH_CLOSE)
		return;

	//cursor_pos: 未清空前的游标地址
	int cursor_pos = p_con->cursor;

	/* current_start_addr 只清除当前屏幕; original_addr 清空显存所有内容;  */
	
	p_con->cursor = p_con->original_addr; /* p_con->cursor = p_con->current_start_addr; */
	for (u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
		p_con->cursor < cursor_pos;
		p_con->cursor++, flush(p_con) ) {
		*p_vmem++ = ' ';
		*p_vmem++ = DEFAULT_CHAR_COLOR;
	}
	
	p_con->cursor = p_con->original_addr; /* p_con->cursor = p_con->current_start_addr; */

	flush(p_con);
}

PUBLIC void out_search(CONSOLE* p_con) {
	//文本恢复白色，删除之前输入的关键字，光标回退。
	search(p_con, DEFAULT_CHAR_COLOR);
	delete_word(p_con, p_con->search_len);
}

PUBLIC void search(CONSOLE* p_con,int color) {
	//从 original_addr 开始匹配
	//i是显存偏移量,success是bool型
	int i = 0, success = 1;
	int cursor_pos = p_con->cursor;
	p_con->cursor = p_con->original_addr;
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->original_addr * 2);
	u8* p_str = (u8*)(V_MEM_BASE + cursor_pos * 2 - p_con->search_len * 2);

	for (; p_vmem < p_str;) {

		if (p_vmem[i] == p_str[i]) {
			//if-else 判断是否匹配成功
			if (p_con->search_len == 1) {
				success = 1;
			}
			else {
				success = 1;
				for (i = 2; i < (p_con->search_len) * 2; i += 2) {
					if (p_vmem[i] != p_str[i]) {
						success = 0;
						break;
					}
				}
			}

			//匹配成功改颜色\正确定位,不成功下一个
			if (success) {
				change_color(p_vmem, p_con->search_len, color);
				p_vmem += i;
				p_con->cursor += (p_con->search_len - 1);
			}
		}
		//下一轮匹配
		p_con->cursor++;
		flush(p_con);
		i = 0;
		p_vmem += 2;
	}

	p_con->cursor = cursor_pos;
	flush(p_con);
}

PRIVATE void change_color(u8* vmm_start, int chars, int color) {
	for (int i = 0; i < chars; i++) {
		*(vmm_start + 1 + 2 * i) = color;
	}
}

PRIVATE void delete_word(CONSOLE* p_con, int chars) {
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	
	for (int i = 0; i<chars;i++) {
		p_con->cursor--;
		*(--p_vmem ) = DEFAULT_CHAR_COLOR;
		*(--p_vmem) = ' ';
	}
	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	p_con->search_len = 0;
	flush(p_con);
}
//End New