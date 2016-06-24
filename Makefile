OBJS = rlib.o libc.o

DEPS= $(filter %.d, $(subst .o,.d, $(OBJS)))

.PHONY: clean rlib.o libc.o

default: lib.o

-include $(DEPS)

lib.o: $(OBJS)
	ld -r $^ -o $@

rlib.o:
	make -C rlib CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" ASFLAGS="$(ASFLAGS)"

libc.o:
	make -C libc CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" ASFLAGS="$(ASFLAGS)"

clean:
	make -C rlib clean
	-rm -f $(OBJS) $(TEST_OBJS) $(DEPS) *.s *.ii
