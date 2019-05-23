CC = g++
ASM = nasm
LD = g++
SFML_DIR= C:/SFML
LIBDIR= $(SFML_DIR)/lib
CFLAGS =-c -g -O3 -Wall -I$(SFML_DIR)/include
AFLAGS = -f elf64	
LDFLAGS = -g -no-pie
SFML = -lsfml-graphics -lsfml-window -lsfml-system 
VPATH = ./src
OBJPATH = ./compile
DIRPATH = ./compile ./compile/cpu
SRCC = main.cpp jcompil.cpp cpu/cpu_asm.cpp cpu/cpu_t.cpp
HEAD = jcompil.hpp cpu/cpu_t.h
SRCASM = 
OBJC = $(SRCC:%.cpp=$(OBJPATH)/%.o)
OBJASM = $(SRCASM:%.S=$(OBJPATH)/%.o)
EXECUTABLE_LINUX = jitrun.out 
EXECUTABLE_WINDOWS = run.exe
SUBDIRS = proglang cpu-old
MAKE = make
MAKEFLAG = linux
windows: $(OBJPATH) $(SRCC) $(SRCASM) $(EXECUTABLE_WINDOWS)

linux: $(DIRPATH) $(SRCC) $(SRCASM) $(EXECUTABLE_LINUX) $(SUBDIRS)
.PHONY: all $(SUBDIRS)

$(DIRPATH):
	@mkdir -p $@

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKEFLAG) 

$(EXECUTABLE_LINUX): $(OBJC) $(OBJASM)
	@echo "LINKING:"
	$(LD) $(LDFLAGS) $(OBJC) -o  $@ $(SFML)

$(EXECUTABLE_WINDOWS): $(OBJC) $(OBJASM)
	@echo "LINKING:"
	$(LD) $(LDFLAGS) $(OBJC) -o  $@ -L$(LIBDIR) $(SFML)

$(OBJPATH)/%.o: %.cpp $(HEAD)
	@echo "COMPILING:"
	$(CC) $(CFLAGS) $< -o $@

$(OBJPATH)/%.o: %.S
	@echo "COMPILING:"
	$(ASM) $(AFLAGS) $< -o $@