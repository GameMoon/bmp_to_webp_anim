PROG=converter
CC=g++
CPPFLAGS=-I.
LIBS=-lwebp -lwebpmux
OFILES = main.cpp

build: 
	$(CC) -o $(PROG) $(CPPFLAGS) $(LIBS) $(OFILES)

clean:
	@rm -f $(PROG)