CC      := gcc
CFLAGS  := -g -Wall
LDFLAGS := -lcurl -lncurses -pthread

TARGET  := final_project
SRCS    := main.c bbb_dht_read.c bbb_mmio.c common_dht_read.c
OBJS    := $(SRCS:.c=.o)
DEPS    := bbb_dht_read.h bbb_mmio.h common_dht_read.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
