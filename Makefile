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
SRCC = main.cpp game.cpp gameobjects.cpp animation.cpp
HEAD = game.hpp gameobjects.hpp animation.hpp
SRCSASM = 
OBJC = $(SRCC:%.cpp=$(OBJPATH)/%.o)
OBJASM = $(SRCASM:.S=.o)
EXECUTABLE_LINUX = run.out
EXECUTABLE_WINDOWS = run.exe

windows: $(OBJPATH) $(SRCC) $(SRCASM) $(EXECUTABLE_WINDOWS)

linux: $(OBJPATH) $(SRCC) $(SRCASM) $(EXECUTABLE_LINUX)

$(OBJPATH):
	@mkdir $@

$(EXECUTABLE_LINUX): $(OBJC)
	@echo "LINKING:"
	$(LD) $(LDFLAGS) $(OBJC) -o  $@ $(SFML)

$(EXECUTABLE_WINDOWS): $(OBJC)
	@echo "LINKING:"
	$(LD) $(LDFLAGS) $(OBJC) -o  $@ -L$(LIBDIR) $(SFML)

$(OBJPATH)/%.o: %.cpp $(HEAD)
	@echo "COMPILING:"
	$(CC) $(CFLAGS) $< -o $@

%.o: %.S
	$(ASM) $(AFLAGS) $< -o $@