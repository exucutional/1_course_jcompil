mkdir compile
gcc -c -o ./compile/cpu_asm.o 	cpu_asm.c
gcc -c -o ./compile/cpu_dasm.o 	cpu_dasm.c
gcc -c -o ./compile/cpu_t.o	cpu_t.c
gcc -c -o ./compile/unit_test.o	unit_test.c

gcc -o ./compile/cpu ./compile/cpu_asm.o ./compile/cpu_dasm.o ./compile/cpu_t.o ./compile/unit_test.o