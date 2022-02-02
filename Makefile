.PHONY: all
all: contexts_fixed_schedule

contexts_fixed_schedule: contexts_fixed_schedule.c
	gcc contexts_fixed_schedule.c -o contexts_fixed_schedule
	#-Wall -Wextra -Werror 

.PHONY: clean
clean:
	rm -f *.txt untracked contexts_fixed_schedule all