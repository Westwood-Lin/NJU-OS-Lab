;----------------------------我的大数加法-----------------------------------;
;职责：  根据两个加数的符号位是否相同,决定执行绝对值相加还是绝对值相减
;       并在处理过程中,设置好和的位数、和的结果(以字符数组的形式)、字符数组的首地址
;args:  rdi 第一个操作数的首地址; rsi 第二个操作数的首地址;
;       dh 第一个操作数的符号; dl 第一个操作数的长度
;       ch 第二个操作数的符号; cl 第二个操作数的长度
;       rax 预留空间的尾地址
;ret:   rax 数组的有效首地址
;       r8b 和的符号; r9b 和的位数
;       设置好sum的每一位
;note:  过程中用bl(rbx)存储carrier,减法中还用r11和r12分别存储第一个和第二个操作数的首地址
;
;实现：  首先初始化,然后根据两个加数的符号位是否相同(异或后是否为0),决定执行绝对值相加还是绝对值相减
;
;绝对值相加：
;       如果是绝对值相加,先确定符号,然后做绝对值相加的循环： 1.第一个加数有没有遍历完？ 2.第二个加数有没有遍历完？ 
;       3.用进位器暂存 Sum(i)=A(i)+B(i)+Carry(i);  4.暂存的Sum(i)>10? 大于10就将进位器设置为1；否则将进位器设置为0；个位存入这一位
;       5.让指针指向高一位数 6.进入下一次循环 7.假如两个数都遍历完,判断进位器是否为1,如果是1,还需要向前多设置1个1；最后返回
;绝对值相减：
;       绝对值相减的问题是无法确定|A|-|B|的符号,可以直接做|A|-|B|,如果|A|>|B|,那么做完减法后最终的进位器会是0;
;       如果|A|<|B|,那么做完后进位器值会是-1,可以设置符号为B,然后交换A\B,重新调用一次减法的循环,让|B|去做被减数
;
;       1.被减数有没有遍历完？ 2.减数有没有遍历完？ 
;       3.用进位器暂存 Sum(i)=A(i)-B(i)+Carry(i);  
;       4.暂存的Sum(i)>0? 大于0就将进位器设置为0,并保存值；否则将进位器设置为-1,向前借1位,然后这一位的数字是10+sum
;       5.让指针指向高一位数 6.进入下一次循环 
;       7.假如两个数都遍历完,判断进位器是否为0,如果是0,可以设置好符号等后直接返回；如果是-1,设置符号,然后交换减数和被减数,重新做一次减法
ret_main:
        ret

MyAdd:
        ;初始化
        mov rbx,0
        mov r9b,0
        ;让第一个数和第二个数的指针指向个位,从末位加起来
        add dil,dl
        sub rdi,1
        add sil,cl
        sub rsi,1
        ;比较两者符号
        cmp dh,ch
        jz abs_add
        jnz abs_minus
        
;绝对值相加
;同号,dh 不用改
; Si=Ai+Bi+Carryi;Carryi代表第i位的进位;rbx存Carryi
abs_add:
        ; 已经知道两个是同号的,直接设置符号
        mov r8w,dx
        and r8w,0ff00H
        shr r8w,8
       
        jmp abs_add_loop
;绝对值相加的循环    
abs_add_loop:
        ;开始按位做加法循环
        ;第1个加数是否遍历完
        cmp r9b,dl
        jnl first_done
        ;第2个加数是否遍历完
        cmp r9b,cl
        jnl second_done
        ;Si=Ai+Bi+Carryi;Carryi
        add bl,BYTE[rdi]
        add bl,BYTE[rsi]
        sub bl,030H
        ;判断
        cmp bl,03AH
        jnl carry1_add
        jl  carry0_add
;循环相加后,进位器需要进位        
carry1_add:
        sub bl,10
        mov BYTE[rax],bl
        mov bl,1        
        jmp next_step_add
;进位器不需要进位        
carry0_add:
        mov BYTE[rax],bl
        mov bl,0
        jmp next_step_add

next_step_add:
        ;不太严谨,但所幸没有影响
        sub rdi,1                
        sub rsi,1
        sub rax,1
        add r9b,1
        jmp abs_add_loop
;第一个数遍历完了
first_done:
        ;两个数组都遍历完,检查是否有进位1
        cmp r9b,cl
        jnl save_OF
        ;第二个数没有遍历完
        add bl,BYTE[rsi]
        cmp bl,03AH
        jnl carry1_add
        jl  carry0_add
                
;第二个数遍历完了
second_done:
        ;两个数组都遍历完,检查是否有进位1
        cmp r9b,dl
        jnl save_OF
        ;第一个数没有遍历完
        add bl,BYTE[rdi]
        cmp bl,03AH
        jnl carry1_add
        jl  carry0_add
        
save_OF:
        ;没有进位
        add rax,1
        cmp bl,0
        jz ret_main
        ;有进位1
        sub rax,1
        add bl,030H
        mov BYTE[rax],bl
        add r9b,1
        ret
        
;绝对值相减
;不知道符号
; Si=Ai-Bi+Carryi;Carryi代表第i位的进位;rbx存Carryi        
abs_minus:
        mov r11,rdi
        mov r12,rsi
        jmp minus_loop
        
minus_loop:
        ;开始循环
        ;被减数遍历完了
        cmp r9b,dl
        jnl first_done_minus
        ;减数遍历完了
        cmp r9b,cl
        jnl second_done_minus
        ;
        add bl,BYTE[rdi]
        sub bl,BYTE[rsi]
        add bl,030H
        
        cmp bl,030H
        jnl carry0_minus
        jl  carryn1_minus
        
;被减数遍历完了
first_done_minus:
        cmp r9b,cl
        jnl save_OF_minus
        ;
        sub bl,BYTE[rsi]
        cmp bl,030H
        jnl carry0_minus
        jl  carryn1_minus
   
;减数遍历完了
second_done_minus:
        cmp r9b,dl
        jnl save_OF_minus
        ;
        add bl,BYTE[rdi]
        cmp bl,030H
        jnl carry0_minus
        jl  carryn1_minus
;进位器携带0
carry0_minus:
        mov BYTE[rax],bl
        mov bl,0
        
        jmp next_step_minux
        
;进位器携带-1
carryn1_minus:
        add bl,10
        mov BYTE[rax],bl
        mov bl,0
        sub bl,1
        
        jmp next_step_minux
       
next_step_minux:
        sub rdi,1        
        sub rsi,1
        sub rax,1
        add r9b,1
        jmp minus_loop
        
save_OF_minus:
        ;|A|-|B|>=0 符号与A相同
        cmp bl,0
        jz save_res
        
        ;|A|-|B|<0 符号与B相同
        ;交换减数和被减数 |B|-|A|
        mov rbx,0
        add al,r9b
        mov r9b,0       
        ;交换dh和ch
        xor dh,ch
        xor ch,dh
        xor dh,ch
        ;交换dl和cl
        xor dl,cl
        xor cl,dl
        xor dl,cl
        ;交换rdi和rsi
        mov rdi,r12
        mov rsi,r11
        
        jmp minus_loop
        
;设置符号并返回,|A|-|B|>0 符号与A相同
save_res:
        add rax,1
        mov r8w,dx
        and r8w,0ff00H
        shr r8w,8        
        ret