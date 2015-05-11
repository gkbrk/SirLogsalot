compile:
	clang -o sirlogsalot sirlogsalot.c

run:
	make compile
	./sirlogsalot

clean:
	rm sirlogsalot

test:
	make compile
	valgrind ./sirlogsalot
