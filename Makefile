BRPATH=/home/domser/studia/semestr6/linsw/BR
OBJS := main.o
main: $(OBJS)
	$(CC) -o main $(CFLAGS) $(LDFLAGS) $(OBJS) -lperiphery
$(OBJS) : %.o : %.c
	$(CC) -I $(BRPATH)/output/build/c-periphery-2.4.2/src/ -c $(CFLAGS) $< -o $@
