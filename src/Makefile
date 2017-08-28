SRC := $(wildcard *.c) # SRC: http.c teddy.c
OBJ := $(patsubst %.c, %.o, $(SRC)) # OBJ: http.o teddy.o
OBJ := $(filter-out teddy.o, $(OBJ))# OBJ: http.o

CPPFLAGS += -I ../inc

LIBPATH = ../lib

all:$(OBJ)


$(OBJ):%.o:%.c
	$(CC) $^ -o $@ -c $(CPPFLAGS) $(LDFLAGS)

	
install:$(OBJ)
	$(CC) -shared -fPIC -o $(LIBPATH)/libcommon.so $(OBJ)
	
clean:
	$(RM) $(OBJ) .*.sw? core

	
