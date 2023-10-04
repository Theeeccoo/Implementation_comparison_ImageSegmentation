#######################################
# Makefile PBM                        #
#                                     #
# E.B.                                #
#######################################

PROG = kmeans
SRC = kmeans.c Util.c
IMG_PATH = ./pix
HEADERS = Util.h

all: run

# Variables for file compilation
CC        =  gcc
CFLAGS    =  -g -Wall -lm -fopenmp
CPPFLAGS  =  -DDEBUG

kmeans: $(SRC) $(HEADERS)
	$(CC) $(SRC) -o $(PROG) -I. $(CFLAGS)

run: kmeans
#	./kmeans ./pix/frog.ppm 3 7 1
# 	./kmeans ./pix/frog.ppm 3 7 2
#	./kmeans ./pix/frog.ppm 3 7 4
# 	./kmeans ./pix/frog.ppm 3 7 8
	./kmeans ./pix/uma-pintura-de-um-lago-de-montanha-com-uma-montanha-ao-fundo.ppm 3 7 8

clean :
	@rm -f *.o

cleanall : clean
	@rm -f $(PROG)
