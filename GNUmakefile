CC=gcc -std=gnu99 -Wall

all:
	$(CC) src/*.c -o bin/fuzztp_server -DSPLIT_BUILD -DBUILD_SERVER
	$(CC) src/*.c -o bin/fuzztp_client -DSPLIT_BUILD

union:
	$(CC) src/*.c -o bin/fuzztp

clean:
	rm -vf bin/*
	touch bin/_

