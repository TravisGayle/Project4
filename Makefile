all: site-tester

site-tester: site-tester.o
	gcc site-tester.o -lcurl -o site-tester -lpthread

site-tester.o: site-tester.c
	gcc -Wall -g -c site-tester.c -o site-tester.o

clean:
	rm -f site-tester.o site-tester