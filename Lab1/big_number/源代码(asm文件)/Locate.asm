;----------------------------封装函数:Locate-------------------------------;
;Locate:主要目的是实现 split ，将输入的字符串分割为2个整数
;职责:   1.确定1个数的符号 2.确定1个数的位数/长度 3.确定数组的首地址
;args:  rdi，该数组的首地址（可能包含首个符号位）
;ret:   rax存储该数去除符号位后的首地址；
;       dl存储该数的符号
;       rdi存储跳出循环的那个地址
;实现：  循环，一直做rdi++，遇到空格或者换行则返回，遇到+号、-号设置符号；遇到第一个数字就开始存储数字
Locate:
        cmp byte[rdi],20H          ;ascii 20H空格
        jz return                  ;return
        cmp byte[rdi],0AH          ;ascii 0AH换行
        jz return                  ;return
        
        cmp byte[rdi],2BH          ;ascii 2BH +号
        jz  setPos
        cmp byte[rdi],2DH          ;ascii 2DH -号
        jz  setNeg
        
        inc rdi
        jmp Locate                 ;loop

;setPos
;职责:   发现正号,设置dl为0(dl为存储符号位的部件)
;       因为发现了符号位,rax负责存储绝对值的首地址，故rax要加1
setPos:
        add rax,1
        inc rdi
        jmp Locate
                
;setNeg
;职责:   发现负号,设置dl为1(dl为存储符号位的部件)
;       因为发现了符号位,而rax负责存储绝对值的首地址，故rax要加1
setNeg:
        add rax,1
        mov dl,1
        inc rdi
        jmp Locate

return:
        ret