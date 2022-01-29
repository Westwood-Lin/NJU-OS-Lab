
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
#include <stdio.h>

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

//TODO New
PUBLIC void tty_clear();
extern void init_undo();
extern char undo_char;
extern u8 undo_content[2];
//End New

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
	/* TODO New */
	p_tty->search_mod = SEARCH_CLOSE;
	/* End New */

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};

		//TODO New:撤回 第一个if语句
		
		//ctrl+z/Z
		if (
			(key == (FLAG_CTRL_L | 'Z')) ||
			(key == (FLAG_CTRL_L | 'z')) ||
			(key == (FLAG_CTRL_R | 'Z')) ||
			(key == (FLAG_CTRL_R | 'z'))
			) {
			//记录上一步操作put_key的key,便于撤回,当然ctrl z不能撤回自己
			put_key(p_tty, '\v');
		}

		//可打印字符
        else if (!(key & FLAG_EXT)) {

			//记录put_key的key,便于撤回
			undo_char = (char)(key % 256);

			put_key(p_tty, key);
        }

		//特殊字符
        else {
			int raw_code = key & MASK_RAW;
			switch(raw_code) {

			//TODO New:新增对 TAB的特殊处理\ 对查找模式的支持
			case TAB:
				//记录put_key的key,便于撤回
				undo_char = '\t';
				put_key(p_tty, '\t');
				break;

			case ESC:
				if (p_tty->search_mod == SEARCH_CLOSE) {
					p_tty->search_mod = SEARCH_OPEN;
				}
				else{
					p_tty->search_mod = SEARCH_CLOSE;
				}
				//记录put_key的key,便于撤回,ESC不用管撤销操作
				put_key(p_tty, '\f');
				init_undo();
				break;

			case ENTER:
				if (p_tty->search_mod == SEARCH_OPEN)
					p_tty->search_mod = SEARCH_DO;

				//记录put_key的key,便于撤回
				undo_char = '\n';
				put_key(p_tty, '\n');
				break;

			case BACKSPACE:
				//记录put_key的key,便于撤回
				undo_char = '\b';
				put_key(p_tty, '\b');
				break;

			case UP:
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_DN);
				}
				break;
			case DOWN:
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_UP);
				}
				break;
			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
			case F8:
			case F9:
			case F10:
			case F11:
			case F12:
			/* Alt + F1~F12 */
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
				select_console(raw_code - F1);
			}
			break;
			default:
				break;
			}
		}
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

/* TODO New function:
 *======================================================================*
				  tty_clear()
 *======================================================================*
 */
PUBLIC void tty_clear()
{
	TTY* p_tty;

	/* 通过for循环,找到当前选中的控制台 */
	for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
		if (is_current_console(p_tty->p_console)) {
			break;
		}
	}

	clear_console(p_tty->p_console);
}

//End New