OBJS = task.o functional.o spinlock.o libglobal.o thread.o mem/uvirtmem.o

DEPS= $(filter %.d, $(subst .o,.d, $(OBJS)))

.PHONY: clean

default: ../rlib.o

-include $(DEPS)

../rlib.o: $(OBJS)
	ld -r $^ -o $@

clean:
	-rm -f $(OBJS) $(TEST_OBJS) $(DEPS) *.s *.ii
