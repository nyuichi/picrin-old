CXXFLAGS += -g -Wall -Wextra -MMD -MP
CC=g++

TARG = picrin.o bool.o core.o list.o port.o

all: picrin

picrin: $(TARG)

test:
	./picrin ../init.scm

clean:
	rm -f picrin *.o *.d

-include $(TARG:%.o=%.d)

