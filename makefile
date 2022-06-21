# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: dev serv
# make rule per il client
dev: client.c client/*.h client/implementation/*.c
	gcc -Wall -o dev client/implementation/*.c client.c
# make rule per il server
serv: server.c server/implementation/*.c server/*.h
	gcc -Wall -o serv server.c server/implementation/*.c
# pulizia dei file della compilazione (eseguito con ‘make clean’ da terminale)
clean:
	rm *o dev serv
