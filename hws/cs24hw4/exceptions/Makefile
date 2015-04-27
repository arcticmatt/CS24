all: test_setjmp divider

OBJS1 = c_except.o my_setjmp.o ptr_vector.o divider.o
OBJS2 = c_except.o my_setjmp.o ptr_vector.o test_setjmp.o

CFLAGS=-g
ASFLAGS=-g

test_setjmp: $(OBJS2)
	$(CC) $(LDFLAGS) $^ -o $@
divider : $(OBJS1)
	$(CC) $(LDFLAGS) $^ -o $@
check : test_setjmp
	./test_setjmp

my_setjmp.o : my_setjmp.s

test_setjmp.c : my_setjmp.h
ptr_vector.c : ptr_vector.h
c_except.c : c_except.h my_setjmp.h
divider.c : c_except.h my_setjmp.h

clean :
	rm -f *.o *~ divider divider.exe test_setjmp

.PHONY: all clean check
