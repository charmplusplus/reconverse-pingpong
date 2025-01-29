all: lib

queue:
	gcc -fPIC -c MpscQueue.C -o queue.o

scheduler:
	gcc -fPIC -c scheduler.C -o scheduler.o

reconverse:
	gcc -fPIC -c reconverse.C -o reconverse.o -lpthread

lib: queue scheduler reconverse
	gcc -shared -o libreconverse.so reconverse.o scheduler.o queue.o

clean:
	rm *.o *.so
