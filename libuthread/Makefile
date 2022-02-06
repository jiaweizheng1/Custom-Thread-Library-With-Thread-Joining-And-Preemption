#target library
lib := libuthread.a
#dependencies
objs := context.o queue.o uthread.o preempt.o

#compile flags
CC := gcc
CFLAGS := -Wall -Wextra -Werror

#more info compile flag
ifneq ($(V), 1)
Q = @
endif

#debug flag
ifneq ($(D), 1)
CFLAGS += -O2
else
CFLAGS += -g
endif

all: $(lib)

libuthread.a: $(objs)
	@echo "CC $@"
	$(Q)ar rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)