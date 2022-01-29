;TODO 增加的系统调用，注意要导出符号global、设置中断向量号（索引号）NR、写好函数体

%include "sconst.inc"

;中断向量索引
_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_sleep			equ 1 ;
_NR_my_print		equ 2 ;
_NR_P				equ 3 ;
_NR_V				equ 4 ;
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global	sleep
global	my_print
global	P
global	V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

;注意esp+4
;====================================================================
;                              sleep
; ====================================================================
sleep:
	mov	eax, _NR_sleep
	mov ebx,[esp+4]
	int	INT_VECTOR_SYS_CALL
	ret

;====================================================================
;                              my_print
; ====================================================================
my_print:
	mov	eax, _NR_my_print
	mov ebx,[esp+4]
	int	INT_VECTOR_SYS_CALL
	ret
	
;====================================================================
;                              P
; ====================================================================
P:
	mov	eax, _NR_P
	mov ebx,[esp+4]
	int	INT_VECTOR_SYS_CALL
	ret
	
;====================================================================
;                              V
; ====================================================================
V:
	mov	eax, _NR_V
	mov ebx,[esp+4]
	int	INT_VECTOR_SYS_CALL
	ret
