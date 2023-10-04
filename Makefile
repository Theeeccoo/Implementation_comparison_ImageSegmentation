#######################################
# Makefile PBM                        #
#                                     #
# E.B.                                #
#######################################

PROG = kmeans
SRC = kmeans.c Util.c
IMG_PATH = ./pix
HEADERS = Util.h

all : kmeans

# Variables for file compilation
CC        =  gcc
CFLAGS    =  -g -Wall -lm -fopenmp
CPPFLAGS  =  -DDEBUG

kmeans: $(SRC) $(HEADERS)
	$(CC) $(SRC) -o $(PROG) -I. $(CFLAGS)
# time ./kmeans ./pix/uma-pintura-de-um-lago-de-montanha-com-uma-montanha-ao-fundo.ppm 3 7
	time ./kmeans ./pix/frog.ppm 3 7

clean :
	@rm -f *.o

cleanall : clean
	@rm -f $(PROG)
