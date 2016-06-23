OBJS = psocket.o

DEPS= $(filter %.d, $(subst .o,.d, $(OBJS)))

.PHONY: clean

default: ../net.o

-include $(DEPS)

../net.o: $(OBJS)
	ld -r $^ -o $@

clean:
	-rm -f $(OBJS) $(TEST_OBJS) $(DEPS) *.s *.ii
