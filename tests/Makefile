LIBS=$(shell pkg-config --libs gtk+-3.0) -lm ../build/libgoatplot.so
CFLAGS=-Wall -g -export-dynamic $(shell pkg-config --cflags gtk+-3.0) -I ../src

gladeTest: gladeTest.c test.glade
	gcc -o gladeTest gladeTest.c $(LIBS) $(CFLAGS)

