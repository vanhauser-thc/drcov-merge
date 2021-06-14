all:	drcov-merge

drcov-merge:	drcov-merge.c
	gcc -g -o drcov-merge drcov-merge.c

clean:
	rm -f drcov-merge

install:
	cp -vf drcov-merge /usr/local/bin/
