#
#
PROG := schema_test
#SRC:= sp_switch_env.c #json_test.c util.c generate_station_stats.c generate_ipset_mac.c
SRC:= schema_validate.c #util.c generate_station_stats.c generate_ipset_mac.c
OBJS:=$(SRC:%.c=%.o)
OBJS_D:=$(SRC:%.c=%.d)
CFLAGS = -Wall -I/home/alan/tools/libs_install_dir/libjansson/include/ -L/home/alan/tools/libs_install_dir/libjansson/lib \
		 -I/home/alan/tools/libs_install_dir/wjelement/include -L/home/alan/tools/libs_install_dir/wjelement/lib/ \
		-lwjelement -lxpl -lwjreader -lwjwriter -ljansson -ldl -L/home/alan/tools/libs_install_dir/wjelement/lib/

#	 -Wl,-static -lwjelement -lxpl -lwjreader -lwjwriter -ljansson -ldl -L/home/alan/tools/libs_install_dir/wjelement/lib/ -Wl,-call_shared
#-Wl,-call_shared -lwjelement -lxpl -lwjreader -lwjwriter -ljansson -ldl -L/home/alan/tools/libs_install_dir/wjelement/lib/

all: $(PROG)

$(PROG):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)
    
%.d: %.c 
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) -MT $*.o $< > $@.$$$$; \
	rm -f $@.$$$$
-include $(OBJS_D)

clean:
	rm -rf *.o
	rm -rf *.d
	rm -rf $(PROG)
