
;----------------------------封装函数区-------------------------------------;
;职责:打印 print
;注意要输出的字符串地址，需要提前mov到rsi里
;要输出的字符串长度，需要提前mov到rdx里;
;有call就有ret
;ret: rax中存储输出的字节数
print:
	pusha			  ; 保存现场
	mov rax,1             ; sys_write的系统调用编号为1
	mov rdi,1             ; 文件句柄1对应输出流stdout
    	syscall               ; 系统调用(64bit下可用,对比32位下是int 80h)
	popa			  ; 恢复现场
    	ret			  ; 返回

;职责:读取键盘输入 read
;arg: (rax,rdi), rsi, rdx;前两个参数在函数内设置
;要保存输入的缓冲区起始地址，需要存到rsi里
;将最大读取的字节大小提前mov到rdx里
;ret：rax中存储总共输入位数
read:	
	pusha			  ; 保存现场
	mov rax,0		  ; 64bit系统下sys_read的系统调用编号为0
	mov rdi,0		  ; 文件句柄0对应输入流stdin
	syscall               ; 系统调用(64bit下可用)
	popa			  ; 恢复现场
	ret
        