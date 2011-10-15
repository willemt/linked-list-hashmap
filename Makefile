CC     = gcc


all: tests

main.c:
	sh make-tests.sh > main.c

tests: main.c linked_list_hashmap.o test_linked_list_hashmap.c CuTest.c main.c
	$(CC) -o $@ $^
	./tests

linked_list_hashmap.o: linked_list_hashmap.c
	$(CC) -c -o $@ $^

clean:
	rm -f main.c linked_list_hashmap.o tests
