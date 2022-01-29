test:main.cpp structure.cpp structure.h my_print.o constants.h 
	@echo "===== 开始生成可执行文件 test ====="
	g++ -m64 main.cpp structure.cpp my_print.o -o test -no-pie
	@echo "===== test 文件已生成 ====="
my_print.o:my_print.asm
	@echo "===== 开始编译 my_print.asm ====="
	nasm -f elf64 my_print.asm -o my_print.o
	@echo "=====     编译结束    ====="
