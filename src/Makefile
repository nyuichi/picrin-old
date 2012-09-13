CXXFLAGS += -g -Wall -Wextra -MMD -MP -I../include
CC=g++

TARG = picrin.o bool.o core.o list.o port.o eval.o env.o

all: picrin run

picrin: $(TARG)

run:
	./picrin ../init.scm

clean:
	rm -f picrin *.o *.d

PHONY: check-syntax
check-syntax:
	$(CC) -Wall -Wextra -fsyntax-only $(CXXFLAGS) $(CHK_SOURCES)

-include $(TARG:%.o=%.d)

