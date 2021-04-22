#
#
PROG := test
SRC:=json_test.c util.c
OBJS:=$(SRC:%.c=%.o)
OBJS_D:=$(SRC:%.c=%.d)
CFLAGS = -Wall -I/home/alan/tools/libs_install_dir/libjansson/include/ -L/home/alan/tools/libs_install_dir/libjansson/lib -Wl,-static -ljansson -Wl,-call_shared

all: $(PROG)

$(PROG):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.d: %.c 
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) -MT $*.o $< > $@.$$$$; \
	rm -f $@.$$$$
-include $(OBJS_D)

clean:
	rm -rf *.o
	rm -rf *.d
	rm -rf $(PROG)
