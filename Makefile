VERSION	= 1.0
CC	= gcc
CFLAGS	= -Wall -g -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/glib-2.0 -I. -fopenmp 
LDFLAGS	= -lm -lglib-2.0 -lpsi-util -lssl -lcrypto
NAME	= psi-cuckoo-hashing
OBJ	= main.o psi_cuckoo_hashing.o

default: psi_cuckoo_hashing

psi_cuckoo_hashing: $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm $(OBJ) $(NAME)

remove:
	rm /usr/bin/$(NAME)

install: psi_cuckoo_hashing
	cp $(NAME) /usr/bin/$(NAME) 
