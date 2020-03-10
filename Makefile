PROG = qTracker
CC = g++
CPPFLAGS = 
LDFLAGS = 
LINUXUSER=qTracker
LINUXFOLDER=cpp

OBJS = main.o 

all: $(PROG) .depend

$(PROG): $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)

.depend:
	-@$(CC) $(CPPFLAGS) -MM $(OBJS:.o=.cpp) > $@ 2> /dev/null

-include .depend

clean:
	rm -f *stackdump $(PROG) *.o .depend

upload:
	echo 'put $(PROG)' | sftp $(LINUXUSER):$(LINUXFOLDER)
