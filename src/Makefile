CFLAGS += -g -Wall -Wextra -MMD -MP

TARG = picrin.o bool.o env.o eval.o core.o gc.o list.o port.o lib.o

all: picrin test

picrin: $(TARG)

test:
	./picrin ../init.scm

clean:
	rm -f picrin *.o *.d

-include $(TARG:%.o=%.d)

