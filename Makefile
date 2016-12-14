VERSION	= 1.0
CC	= gcc
CFLAGS	= -Wall -g -fopenmp -I. -I/usr/include/psi_util -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/glib-2.0 
LDFLAGS	= -fopenmp -lpsi-util -lm -lssl -lcrypto -lglib-2.0
NAME	= psi-simple-hashing
OBJ	= main.o psi_simple_hashing.o

default: psi_simple_hashing

psi_simple_hashing: $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f $(OBJ) $(NAME)

remove:
	rm -f /usr/bin/$(NAME)

install: psi_simple_hashing
	cp $(NAME) /usr/bin/$(NAME) 
