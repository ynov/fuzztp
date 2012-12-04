CC=gcc -std=gnu99 -Wall

all:
	$(CC) *.c -o build/fuzztp_server -DSPLIT_BUILD -DBUILD_SERVER
	$(CC) *.c -o build/fuzztp_client -DSPLIT_BUILD

union:
	$(CC) *.c -o build/fuzztp

clean:
	rm -vf build/*
	touch build/empty

