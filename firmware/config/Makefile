include ../flashvars.mk
CFLAGS += -DFLASHVARS_ADDR=$(FLASHVARS_ADDR)

TARGET=patcher

CFLAGS += -Wall -I../include -DPOSIX_ENABLED

all:
	gcc -o $(TARGET) $(CFLAGS) config.c patch.c

clean:
	rm -f *.o $(TARGET) $(TARGET).exe
