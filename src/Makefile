CXXFLAGS += -g -Wall -Wextra -MMD -MP
CC=g++

TARG = picrin.o bool.o core.o list.o port.o eval.o env.o lib.o

all: picrin run

picrin: $(TARG)

run:
	./picrin ../init.scm

clean:
	rm -f picrin *.o *.d

-include $(TARG:%.o=%.d)

