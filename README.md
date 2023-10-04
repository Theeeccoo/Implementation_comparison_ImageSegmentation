# Parallel implementation of the K-means algorithm for image segmentation

Image segmentation is a fundamental process in the field of image processing and computer vision. It refers to the technique of dividing an image into several regions or segments, where each segment represents a distinct area or object in the image. The main purpose of image segmentation is to isolate and identify specific objects or features in an image for analysis, interpretation, or recognition.

The K-Means algorithm is often used for image segmentation, where its objective is to group similar pixels into distinct regions or clusters. The basic idea behind K-Means is to find groups of points (in this case, pixels) that are similar to each other, assigning each pixel to a cluster represented by its centroid.

K-Means is sensitive to the initialization of centroids and can converge to local solutions. Therefore, it is common to run the algorithm several times with different initializations and choose the best solution.

Coloured PPM images have been used as inputs and image segmentation has been implemented using K-means technique. A detailed report of the original code can be found in [report](https://github.com/archity/segmentation-kmeans-c/blob/main/report.pdf).

As the application involves segmenting an image, it is necessary to iterate through each pixel of the image. According to the repetition loops below, these iterations across pixels are carried out and are a good opportunity for parallelization, as currently the resolution and quality of images are increasing, making these repetition loops with many iterations.

Changing the original code, which can be found on [github](https://github.com/archity/segmentation-kmeans-c), to a version that contains parallelism to obtain better performance in time, was a work proposal from the Parallel Computing discipline. Guided by professor [Henrique Cota de Freitas](https://github.com/hcfreitas) in the 6th period of the Computer Science course at the Pontifical Catholic University of Minas Gerais. This work was carried out by the students:

- [Caio Eduardo Ramos Arães](https://github.com/Shadod123)
- [Danielle Dias Vieira](https://github.com/DanielleDVieira)
- [Guilherme Quadros Werner](https://github.com/GuilhermeWerner)
- [João Augusto dos Santos Silva](https://github.com/joaoaugustoss)
- [Thiago de Campos Ribeiro Nolasco](https://github.com/Theeeccoo)


## Input images used:

<table style="background-color:black;">
  <tr>
    <center>
      <td align="center">Frog</td>
      <td align="center">The Night King</td>
      <td align="center">Lakes with mountains</td>
    </center>
  </tr>
  <tr>
    <td><img src="https://github.com/Theeeccoo/Implementation_comparison_ImageSegmentation/blob/ompKmeans/pix/frog.png" height=200></td>
    <td><img src="https://github.com/Theeeccoo/Implementation_comparison_ImageSegmentation/blob/ompKmeans/pix/tnk.png" height=200></td>
     <td><img src="https://github.com/Theeeccoo/Implementation_comparison_ImageSegmentation/blob/ompKmeans/pix/uma-pintura-de-um-lago-de-montanha-com-uma-montanha-ao-fundo.jpg" height=200></td>
  </tr>
</table>


## Few Outputs:

<table>
  <tr>
    <center>
      <td align="center">Frog (3 clusters)</td>
      <td align="center">The Night King (3 clusters)</td>
      <td align="center">Lakes with mountains (3 clusters)</td>
    </center>
  </tr>
  <tr>
    <td><img src="https://github.com/archity/segmentation-kmeans-c/blob/main/pix/frog-k3-i11.png" height=200></td>
    <td><img src="https://github.com/archity/segmentation-kmeans-c/blob/main/pix/tnk-k3-i12-rand_INIT-v1.png" height=200></td>
    <td><img src="https://github.com/Theeeccoo/Implementation_comparison_ImageSegmentation/blob/ompKmeans/pix/mountains.jpg" height=200></td>
  </tr>
  <tr>
    <td><img src="https://github.com/archity/segmentation-kmeans-c/blob/main/pix/frog-k3-i7-rand_INIT-v1.png" height=200></td>
    <td><img src="https://github.com/archity/segmentation-kmeans-c/blob/main/pix/tnk-k3-i12-rand_INIT-v2.png" height=200></td>
    <td><img src="https://github.com/Theeeccoo/Implementation_comparison_ImageSegmentation/blob/ompKmeans/pix/mountains2.png" height=200></td>
  </tr>
</table>

### Languages supported

C (Implementation)

## Requirements

The following tools are required in order to start the installation.
- OpenMP
        
## Compilation and execution

- To compile and run the code:

  1. Clone this repository: `git clone https://github.com/Theeeccoo/Implementation_comparison_ImageSegmentation.git`
  2. To compile and execute the code, in the folder where the makefile is located, do: `make run`

## Usage
5 parameters are required to execute the code, which are:

  1. The executable file: `./kmeans`
  2. Image that will be segmented in the ppm extension: `./pix/frog.ppm`
  3. Number of clusters in the K-mens algorithm, which means the number of regions in the final segmented image: `3`
  4. Number of iterations in K-means algorithm: `7`
  5. Number of threads: `4`
     
Example: `./kmeans ./pix/frog.ppm 3 7 4`

**Note:** To change the image that will be segmented, the number of clusters, the number of iterations and the number of threads can be changed in the makefile execution rule, which is where the command line for execution is. Or it can be placed directly in the terminal to run as per usage.

