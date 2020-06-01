PROG = qTracker
CC = g++
CPPFLAGS = -I /usr/include -L /usr/lib
LDFLAGS = -lmysqlcppconn
LINUXUSER = tarves
LINUXFOLDER = qtracker

OBJS = main.o Server.o Database.o Torrent.o Peer.o AnnounceInfo.o Logger.o

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
