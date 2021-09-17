myls: myls.c
	gcc -Wall -g -pedantic -o myls myls.c

.PHONY: clean
clean:
	rm -f myls
