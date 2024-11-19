CC = gcc
CFLAGS = -I. -lpthread

new: tcpclient tcpserver udpclient udpserver 

tcpclient: tcpclient.c
	$(CC) -o tcpclient tcpclient.c $(CFLAGS)

tcpserver: tcpserver.c
	$(CC) -o tcpserver tcpserver.c $(CFLAGS)

udpclient: udpclient.c
	$(CC) -o udpclient udpclient.c $(CFLAGS)

udpserver: udpserver.c
	$(CC) -o udpserver udpserver.c $(CFLAGS)

clean:
	rm -f tcpclient tcpserver udpclient udpserver
