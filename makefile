# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: client server 
# make rule per il client
client: client.c client/*.h client/implementation/*.c
	gcc -Wall -o dev client/implementation/*.c client.c
# make rule per il server
server: server.c server/implementation/*.c server/*.h
	gcc -Wall -o serv server.c server/implementation/*.c
# pulizia dei file della compilazione (eseguito con ‘make clean’ da terminale)
clean: clean1 clean2 clean3
clean1:
	rm *o dev serv
clean2:
	rm test/client[123]/dev test/server/serv
clean3:
	rm test/client[123]/*dat test/server/*dat
setup:
	cp dev test/client1
	cp dev test/client2
	cp dev test/client3
	cp serv test/server
