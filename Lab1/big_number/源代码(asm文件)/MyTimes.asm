
;----------------------------我的大数乘法-----------------------------------;
;职责：  在处理过程中,设置好积的位数、积的结果(以字符数组的形式)、字符数组的首地址
;args:  rdi 第一个操作数的首地址; rsi 第二个操作数的首地址;
;       dh 第一个操作数的符号; dl 第一个操作数的长度
;       ch 第二个操作数的符号; cl 第二个操作数的长度
;       rax 预留空间的尾地址
;ret:   rax 数组的有效首地址
;       r8b 积的符号; r9b 积的位数
;       设置好prod的每一位
;note:  过程中用bl(rbx)存储carrier,还用r10、r11和r12分别存储预留空间的尾地址、第一个和第二个操作数的首地址
;       r13\r14\r15做tmp用
;       mul指令、div指令的使用需要留心
;       mul指令默认要用rax/eax和rdx/edx，只有一个参数做乘数，而且只能是 register; mul src
;       div指令也是只有一个register做除数，div src
;
;实现：
;       A x B  a1a2……an * b1b2……bm
;       1.首先初始化,并且设置符号位,负负得正,因为用0代表正,1代表负,设置两者乘积的符号,可以通过 xor 异或运算
;       2.将存储积的预留空间全部初始化为 真正的0值，是0,不是'0'（这一步很重要）
;       3.进行双重循环：外层我选的是b,内层选的是a
;           对bm和an,product(用carry暂存)=bm * an +carry+当前这一位的值;
;           product>=10? >=10,则当前这一位的新值=product%10,carry=product/10;否则,当前这一位的新值=product,carry=product/10
;           内层循环到下一步,判断 a有没有遍历完,没遍历完,就让a(n-1)替代a(n)；a遍历完,让a重新指向个位,让bm向前到b(m-1)
;           
;           (乘法的难度就在于设置这个双重循环，尤其循环的终止条件和每一步的递进要考虑清楚，还有在计算的时候一定要注意去除ascii码的偏移量)   
;       4.双重循环做完以后,判断进位器carry>0? 大于0就要多进1位
;       5.返回之前,给每一位加上ascii码的偏移量030H,方便数字能正确显示;设置好后返回
ret_from_times:
        inc rax
        ret
MyTimes:
       ;初始化积的位数、进位器bl(rbx),让rdi和rsi指向最后一位数字,r13\r14做tmp用
        mov rbx,0
        mov r9,0
        mov r13,0
        mov r14,0
        mov r15,0
        
        ;用r10、r11和r12分别存储预留空间的尾地址、第一个和第二个操作数的首地址
        mov r10,rax
        mov r11,rdi
        mov r12,rsi
        
        ;让rdi和rsi指向最后一位数字
        add dil,dl
        sub rdi,1        
        add sil,cl
        sub rsi,1
        
        jmp set_zero
        
;将存放积的空间全部置为0  
set_zero:
        cmp r9,42
        jnl set_sign
        mov BYTE[rax],0
        inc r9
        sub rax,1
        jmp set_zero
       
set_sign:
        ;恢复r9和rax
        mov r9,0
        mov rax,r10
        ; 用异或来设置符号
        xor dh,ch
        mov r8w,dx
        and r8w,0ff00H
        shr r8w,8
        ;
        jmp times_loop
        
times_loop:
        ;外层for循环的退出条件
        cmp rsi,r12
        jl  ret_from_times
        ;内层for循环的退出条件
        cmp rdi,r11
        jl  next_factor
        
        mov r13b,BYTE[rsi]
        mov r14b,BYTE[rdi]
        sub r13b,030H
        sub r14b,030H
        
        ;r13b=r13b*r14b
        mov r15,rax
        mov rax,r13
        mul r14b
        mov r13,rax
        mov rax,r15
        
        ;加上后面给的进位
        add r13b,bl
        ;
        add r13b,BYTE[rax]
        
        cmp r13b,10
        jl  carry0_times
        jnl carry_more

carry0_times:        
        mov BYTE[rax],r13b
        
        mov bl,0
        sub rax,1
        sub rdi,1
        jmp times_loop
carry_more: 
        
        ;mov BYTE[rax],?
        ;mov bl,?
        mov r15,rax
        ;----
        mov rax,r13
        mov r14b,10
        div r14b

        mov bl,al
        shr rax,8
        mov BYTE[r15],al              
        ;-----
        mov rax,r15
        sub rax,1
        sub rdi,1
        jmp times_loop  
save_bl:
        mov BYTE[rax],bl
        mov bl,0
        cmp rsi,r12
        jnz next_factor
        sub rax,1
        jmp save_times_res
next_factor:
        
        cmp bl,0
        jnz save_bl
        ;外层for循环的退出条件
        cmp rsi,r12
        jz  save_times_res
        
        mov rdi,r11
        add dil,dl
        sub rdi,1  
        
        inc r9b
        mov rax,r10
        sub rax,r9
        
        sub rsi,1
        jmp times_loop

save_times_res:
        ;设置长度
        mov r9,r10
        sub r9,rax
        add rax,r9
        ;添加ascii码偏移量
        mov r13,r9
        jmp add_030H
        
;添加ascii码偏移量
add_030H:
        cmp r13,0
        jz ret_from_times
        
        add BYTE[rax],030H
        sub r13,1
        sub rax,1
        jmp add_030H                 