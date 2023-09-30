CC = gcc
CC_FLAGS = -Wall
CC_OMP = -fopenmp

all: serial.out omp.out omp_alt.out

clean:
	rm serial.out omp.out omp_alt.out result.jpg

serial.out: src/main_serial.c src/image_io.h src/image_io.c src/segmentation.h src/segmentation_serial.c
	$(CC) $(CC_FLAGS) -o serial.out src/main_serial.c src/image_io.c src/segmentation_serial.c -lm

omp.out: src/main_omp.c src/image_io.h src/image_io.c src/segmentation.h src/segmentation_omp.c
	$(CC) $(CC_FLAGS) $(CC_OMP) -o omp.out src/main_omp.c src/image_io.c src/segmentation_omp.c -lm

omp_alt.out: src/main_omp.c src/image_io.h src/image_io.c src/segmentation.h src/segmentation_omp_alt.c
	$(CC) $(CC_FLAGS) $(CC_OMP) -o omp_alt.out src/main_omp.c src/image_io.c src/segmentation_omp_alt.c -lm

