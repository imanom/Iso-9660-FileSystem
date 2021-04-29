OBJECTS = ./lib/src/helper.o ./lib/src/isoMethods.o

CC = gcc
CFLAGS  += -Wall -g -DDEBUG -pedantic -std=gnu99 -Wundef -Wcast-align -W -Wpointer-arith -Wwrite-strings -Wno-unused-parameter

./default: main
isoSpec.a: $(OBJECTS)
	@echo 'Creating isoSpec.a'
	@$(AR) -cr isoSpec.a $(OBJECTS)

$(OBJECTS): %.o: %.c %.h Makefile ./lib/src/isoSpec.h
	@echo 'Compiling' $<
	@$(CC) $< $(CFLAGS) -c -o $@

main: main.o
	$(CC) $(CFLAGS) main.c isoSpec.a -o main

clean:
	$(RM) *.o ./lib/src/*.o isoSpec.a ./main

