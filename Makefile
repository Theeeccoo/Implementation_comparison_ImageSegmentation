CC = gcc
CC_FLAGS = -Wall
CC_OMP = -fopenmp
CC_VECT = -fopt-info-vec-optimized

all: serial.out omp.out omp_alt.out

clean:
	rm serial.out omp.out omp_alt.out result_serial.jpg result_omp.jpg result_omp_alt.jpg

serial.out: src/main_serial.c src/image_io.h src/image_io.c src/segmentation.h src/segmentation_serial.c
	$(CC) $(CC_FLAGS) -o serial.out src/main_serial.c src/image_io.c src/segmentation_serial.c -lm

omp.out: src/main_omp.c src/image_io.h src/image_io.c src/segmentation.h src/segmentation_omp.c
	$(CC) $(CC_FLAGS) $(CC_OMP) -o omp.out src/main_omp.c src/image_io.c src/segmentation_omp.c -lm

omp_alt.out: src/main_omp.c src/image_io.h src/image_io.c src/segmentation.h src/segmentation_omp_alt.c
	$(CC) $(CC_FLAGS) $(CC_OMP) $(CC_VECT) -O3 -o omp_alt.out src/main_omp.c src/image_io.c src/segmentation_omp_alt.c -lm
