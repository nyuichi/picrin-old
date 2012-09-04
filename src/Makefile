CFLAGS += -g -Wall -Wextra -MMD -MP

TARG = picrin.o bool.o env.o eval.o core.o gc.o list.o port.o lib.o

all: picrin

picrin: $(TARG)

clean:
	rm picrin *.o *.d

-include $(TARG:%.o=%.d)

