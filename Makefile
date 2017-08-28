vpath %.c = src/
vpath %.h = inc/

CPPFLAGS += -I ./inc

LDFLAGS += -L ./lib
LDFLAGS += -lcommon
LDFLAGS += -lpthread
LDFLAGS += -Wl,-rpath=./lib

SRCPATH = ./src
INCPATH = ./inc
LIBPATH = ./lib


#CROSS = arm-none-linux-gnueabi-
CC = $(CROSS)gcc


teddy.elf:teddy.c libcommon.so
	$(CC) $< -o $@ $(CPPFLAGS) $(LDFLAGS) -DNDEBUG

debug.elf:teddy.c libcommon.so
	$(CC) $< -o $@ $(CPPFLAGS) $(LDFLAGS) -DDEBUG

	
libcommon.so:
	$(MAKE) -C $(SRCPATH) install # 调用./src里面的子Makefile
	
clean:
	$(RM) teddy.elf debug.elf
	
distclean:clean
	$(MAKE) -C $(SRCPATH) clean
	$(RM) $(LIBPATH)/*
