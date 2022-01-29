;author: Yalin Feng 冯亚林,191850036
;filename: BigNum.asm
;compile:  nasm -f elf64 BigNum.asm -o BigNum.o
;link:     gcc BigNum.o -o BigNum -no-pie
;execute:  ./BigNum

;-------------------------------分割线-------------------------------------;
;---------------------------------宏--------------------------------------;
;macro 宏,64位linux上没有popa和pusha, 需要自定义
;rax和rdx要保存系统调用返回值的，如果在read和print的popa和pusha里不注释它俩，相当于覆盖了返回值
%macro popa 0
	pop rcx
	pop rbx
%endmacro

%macro pusha 0
	push rcx
	push rbx
%endmacro

%include "./MyAdd.asm"
%include "./MyTimes.asm"
%include "./UserIO.asm"
%include "./Locate.asm"

;-------------------------------分割线-------------------------------------;
;-------------------------------数据区-------------------------------------;
section .data
	tips: db "please input 2 numbers seperated by space",0Ah
	tipsLen: equ $ - tips
        ;第一个数和第二个数是否是负数,值为0代表正数,1代表负数,默认为正
        isNeg1:db 0
        isNeg2:db 0
        ;换行符
        nextLine:db 0AH       

;-------------------------------分割线-------------------------------------;
;-------------------------------声明区-------------------------------------;
section .bss
        ;第一个数和第二个数的位数
        len1:resb 1
        len2:resb 1

        ;10的20次方也就是21位数，1位数1个字节，50个字节的缓冲区够2个大数输入了
        inputBuff: resb 50
        inputLen:resb 1
        
        ;第一个数和第二个数的起始地址
        ;when use:QWORD[ptr]
        firstPtr:resq 1
        secondPtr:resq 1
        
        ;2个21位的数相加，绝对值顶多22位数;值得注意的是sumPtr才是指向和数组首元素的指针
        sumSpace:resb 22
        sumPtr:resq 1
        sumLen:resb 1               ;长度
        sumSign:resb 1              ;符号
        
        ;21位数乘以21位数，顶多42位;值得注意的是prodPtr才是指向乘积数组首元素的指针
        prodSpace:resb 42
        prodPtr:resq 1
        prodLen:resb 1              ;长度
        prodSign:resb 1              ;符号
;-------------------------------分割线-------------------------------------;
;-------------------------------代码区-------------------------------------;
section .text	
	;程序入口
global main
main:
    mov rbp, rsp; for sscorrect debugging
	;提示
	mov rsi,tips                ; 字符串地址 
    	mov rdx,tipsLen             ; 字符串长度
	call print

;-------------------小分割线--------------------------;
;-------------------读取输入--------------------------;
        ;ret：rax中存储总共输入位数
	mov rsi,inputBuff           ; 保存输入的变量,缓冲区
	mov rdx,50                  ; 最大读取长度,最多读取50个字节 
	call read
        mov BYTE[inputLen],al       ; 输入的总长度
        
;-------------------小分割线--------------------------;
;-------------------处理输入--------------------------;
        ;封装函数Locate，主要目的是实现 split ，将输入的字符串分割为2个整数;
        ;具体职责是确定1个数的符号、长度、数字首地址
        ;Locate,详见Locate.asm        
        ;args: rdi，该数组的首地址(可能包含首个符号位)
        ;ret:  rax存储该数去除符号位后的首地址；
        ;      dl存储该数的符号
        ;      rdi存储跳出循环的那个地址(第一次是空格，这样设置有利于第二个数的Locate
        
        ;用Locate确定第一个数的info
        mov rdi,rsi
        mov rax,rdi
        mov dl,0
        call Locate
        
        mov BYTE[isNeg1],dl
        mov QWORD[firstPtr],rax
        ;确定数的位数/长度
        mov rcx,rdi
        sub rcx,QWORD[firstPtr]
        mov BYTE[len1],cl
   
        ;用Locate确定第二个数的info
        ;注意执行完第一次Locate，rdi指向的是两个数中间的空格，要执行一次inc
        inc rdi
        mov rax,rdi
        mov dl,0
        call Locate
        
        mov BYTE[isNeg2],dl
        mov QWORD[secondPtr],rax
        ;确定数的位数/长度
        mov rcx,rdi
        sub rcx,QWORD[secondPtr]
        mov BYTE[len2],cl

;-------------------小分割线--------------------------;    
;-------------------求和并输出------------------------;    
        ;调用我自己的加法函数MyAdd,具体args和ret见文件MyAdd.asm
        mov rdi,QWORD[firstPtr]
        mov rsi,QWORD[secondPtr]
        mov dh,BYTE[isNeg1]
        mov ch,BYTE[isNeg2]
        mov dl,BYTE[len1]
        mov cl,BYTE[len2]
        mov rax,sumSpace
        add rax,21
        call MyAdd

        mov QWORD[sumPtr],rax
        mov BYTE[sumSign],r8b
        mov BYTE[sumLen],r9b
                      
        ;输出和 show sum
        mov rcx,QWORD[sumPtr]        ;
        mov r8b,BYTE[sumLen]         ;无符号扩展
        mov r9b,BYTE[sumSign]
        call showRes
;-------------------小分割线--------------------------;        
        ;换行
        mov rsi,nextLine             ;
        mov rdx,1                    ;无符号扩展
        call print
;-------------------小分割线--------------------------;
;-------------------求积并输出--------------------------;        
        ;调用我自己的乘法函数MyTimes,具体args和ret见文件MyTimes.asm
        mov rdi,QWORD[firstPtr]
        mov rsi,QWORD[secondPtr]
        mov dh,BYTE[isNeg1]
        mov ch,BYTE[isNeg2]
        mov dl,BYTE[len1]
        mov cl,BYTE[len2]
        mov rax,prodSpace
        add rax,41
        call MyTimes        
        
        mov QWORD[prodPtr],rax
        mov BYTE[prodSign],r8b
        mov BYTE[prodLen],r9b
        
        ;输出积 show product
        mov rcx,QWORD[prodPtr]      ;
        mov r8b,BYTE[prodLen]       ;无符号扩展
        mov r9b,BYTE[prodSign]
        call showRes
;-------------------小分割线--------------------------;        
        ;换行
        mov rsi,nextLine             ;
        mov rdx,1                    ;无符号扩展
        call print
;exit
	mov rax, 60
	syscall
	ret	

;-----------------------------分割线---------------------------------------;
;----------------------------封装函数:showRes------------------------------;

;
;args:  rcx sum字符数组首地址
;       r8b 输出位数
;       r9b sum符号大小（0正1负
;ret:   
showRes:
        jmp format
              
format: 
        cmp r8b,0
        jz  showZero
        cmp BYTE[rcx],030H
        jnz showSignal
        inc rcx
        sub r8b,1
        jmp format
        
showSignal:
        cmp r9b,0                 ;+正号不用输出
        jz  showBits
        inc r8b
        sub rcx,1
        mov BYTE[rcx],02DH
        jmp showBits

showZero:
        add r8b,1
        sub rcx,1
        jmp showBits
        
showBits:
        mov rsi,rcx
        movzx rdx,r8b
        call print
