SRC:=$(wildcard *.c)
TARGET:=$(patsubst %.c,%,$(SRC))
CUR_DIR=$(shell pwd)

CC=gcc
LD=ld
LIBDIR=../../../Lib
INCDIR=../../../Inc
CFLAGS=-O1 -g -Wall -I$(INCDIR) -L$(LIBDIR) -lpthread -lhash_mem_pool

all:
#CUR_DIR=$(shell pwd)	# Why I can't define variable here
	@echo $(CUR_DIR)
#TARGET:=$(patsubst %.c,%.o,$(SRC))
	@echo $(TARGET)
#	@echo "convert file to target" #Why convert failed
#	@for file in $(SRC); do\
#		echo "convert $$file to $(patsubst %.c,%.o,$$file)";\
#	done
	@echo "convert target to file"
	@for target in $(TARGET); do\
		echo "convert $$target to $(patsubst %,%.c,$$target)";\
		$(CC) $(CFLAGS) -o $$target $(patsubst %,%.c,$$target);\
	done

clean:
	@for target in $(TARGET); do\
		rm -f $(patsubst %,%.o,$$target);\
		rm -f $$target;\
	done
