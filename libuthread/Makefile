# Target library; Add comments for the rest later
lib := libuthread.a
objs := queue.o

CC := gcc
CFLAGS := -Wall -Wextra -Werror
#CFLAGS += -g -MMD

ifneq ($(V), 1)
Q = @
endif

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

libuthread.a: $(objs)
	@echo "CC $@"
	$(Q)ar -rcs $@ $<

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)