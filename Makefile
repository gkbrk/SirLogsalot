all: sirlogsalot

sirlogsalot: sirlogsalot.c
	$(CC) -o sirlogsalot sirlogsalot.c

run: sirlogsalot
	./sirlogsalot

clean:
	rm sirlogsalot

test: sirlogsalot
	valgrind ./sirlogsalot
