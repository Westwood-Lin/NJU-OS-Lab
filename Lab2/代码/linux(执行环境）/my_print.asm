;----------------------------封装函数区-------------------------------------;
;第一次实验中已经写好了 read 和 print 方法，可以直接复用
;这次考察的要点不是 my_print.asm的编写，而是怎样在 C/Cpp文件里调用 asm里的函数，怎样将二者正确地编译和链接
global my_print;
	section .text
;职责:	向屏幕输出 print
;arg:	(rax,rdi), rsi, rdx;前两个参数在函数内设置
;		rsi:要输出的字符串起始地址
;		rdx: 字符串长度(单位:字节)
;有call就有ret
;ret:	rax中存储输出的字节数s
my_print:
	mov rax,1                  ; sys_write的系统调s用编号为1
	mov rdi,1                  ; 文件句柄1对应输出流stdout
        syscall                    ; 系统调用(64bit下可用,对比32位下是int 80h)
        ret		          ; 返回


;职责:读取键盘输入 read
;arg:   (rax,rdi), rsi, rdx;前两个参数在函数内设置
;		rsi:输入缓冲区起始地址
;		rdx:最大读取的字节数
;ret:	rax中存储总共输入位数
read:	
	mov rax,0		  ; 64bit系统下sys_read的系统调用编号为0
	mov rdi,0		  ; 文件句柄0对应输入流stdin
	syscall                    ; 系统调用(64bit下可用)
	ret
        