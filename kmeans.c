/*================================================================================================

    Projeto 1 - Implementação Paralela de uma Técnica de Inteligência Artificial

    Alunos: Caio Eduardo Ramos Arães
            Danielle Dias Vieira
            Guilherme Quadros Werner
            João Augusto dos Santos Silva
            Thiago de Campos Ribeiro Nolasco

================================================================================================*/
/*================================================================================================

    Aplicação:

    Segmentação de imagens baseada em cor e implementada via algoritmo de agrupamento k-means

    Disponível em https://github.com/archity/segmentation-kmeans-c

    Mais informações no README.md sobre a aplicação e o algoritmo k-means.
    
================================================================================================*/
/*================================================================================================

    Resultados:

    Informações do computador pessoal em que foram gerados os resultados apresentados:
    - CPU: Intel i7-9750H (12) @ 4.500GHz
    - Memory: 32GB 
    - Compiler: gcc 10.5.0 - Suporta OpenMP 4.5
    - OS: Pop!_OS 20.04 LTS x86_64 

    Resultados em máquina pessoal:

    1. Sequencial ----------------------------------------------- 38,0154s

    2. Paralelo (2 threads) ------------------------------------- 23,0949s

    3. Paralelo (4 threads) ------------------------------------- 15,6061s

    4. Paralelo (8 threads) ------------------------------------- 12,999s

    ts = 38,0154s
    tp = 12,999s

    Ganho = ts / tp = 2,9244s

================================================================================================*/
/*================================================================================================

    Análise de Resultados:

    Foi realizada a tentativa de alterar o escalonamento (dynamic ou static) das iterações nos loops, 
    porém não houve diferença em relação aos tempos de execução.

    Os resultados acima são resultantes da média de 10 execuções para cada número de threads.

    - Falar qual imagem utilizamos no teste.

================================================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Util.h"
#include <time.h>
#include <omp.h>

void PrintFunc(bit *clusters, short a, int b)
{
  for (int i = 0; i < a; i++)
  {
    for (int j = 0; j < b; j++)
    {
      printf("%d, ", clusters[i * b + j]);
    }
    printf("\n");
  }
  printf("\n");
}

int ClusterCompare(bit *prevCluster, bit *currCuster, short a, int b)
{
  for (int i = 0; i < a; i++)
  {
    for (int j = 0; j < b; j++)
    {
      if (prevCluster[i * b + j] != currCuster[i * b + j])
      {
        return 0;
      }
    }
  }
  return 1;
}

void AllocateRandomClusters(bit *clusters, short clustercount)
{
  printf("Initial randomized cluster centres: \n\n");
  // Initializing clusters
  srand(time(NULL));
  for (int i = 0; i < clustercount; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      clusters[i * 3 + j] = rand() % 256;
      printf("%d, ", clusters[i * 3 + j]);
    }
    printf("\n");
  }
}

void RecalculateClusters(bit *bitmap, bit *clusters, short clustercount, bit *labels, int rows, int cols)
{
  /*
      Como a aplicação envolve a segmentação de uma imagem, é necessário iterar por cada pixel da mesma.
      Conforme os laços de repetição abaixo, estas iterações pelos pixels são realizadas e é uma boa oportunidade
      para uma paralelização, pois atualmente a resolução e a qualidade das imagens vem aumentando, tornando
      estes laços de repetições com muitas iterações.
      A paralelização abaixo está sobre o loop externo que itera sobre os clusters. As variáveis count, pixelsum
      são declaradas dentro do loop, logo já são privadas para cada thread.
      A variável clusters não foi necessária a privativação, pois nenhuma iteração vai acessar duas posições
      ao mesmo tempo, pois ela utiliza a variável "i" do loop mais externo, e esta variável é diferente para
      cada thread.
  */
  #pragma omp parallel for
  for (int i = 0; i < clustercount; i++)
  {
    int count = 0;
    float pixelsum[3];
    for (int j = 0; j < 3; j++)
    {
      pixelsum[j] = 0;
    }
    for (int j = 0; j < rows; j++)
    {
      for (int k = 0; k < cols; k++)
      {
        if (labels[j * cols + k] == i)
        {
          for (int l = 0; l < 3; l++)
          {
            pixelsum[l] += bitmap[(j * cols + k) * 3 + l];
          }
          count++;
        }
      }
    }
    for (int l = 0; l < 3; l++)
    {
      clusters[i * 3 + l] = pixelsum[l] / count;
    }
  }
}

int GetClusterPixelIndex(bit *pixel, bit *clusters, short clustercount)
{
  // Assign pixel to cluster
  int min_dist = -1, dist, clusterIndex;
  for (int i = 0; i < clustercount; i++)
  {
    dist = 0;
    for (int j = 0; j < 3; j++)
    {
      dist = dist + pow(pixel[j] - clusters[(i * 3 + j)], 2);
    }
    dist = sqrt(dist);
    if (dist < min_dist || min_dist == -1)
    {
      min_dist = dist;
      clusterIndex = i;
    }
  }

  return clusterIndex;
}

void GetClusteredImage(bit *bitmap, bit *clustermap, bit *clusters, bit *labels, short clustercount, int rows, int cols)
{
  bit pixel[3];
  int clusterIndex;
  //printf("rows: %d\n", rows);
  ///printf("cols: %d\n", cols);
  
  /*
      Como a aplicação envolve a segmentação de uma imagem, é necessário iterar por cada pixel da mesma.
      Conforme os laços de repetição abaixo, estas iterações pelos pixels são realizadas e é uma boa oportunidade
      para uma paralelização, pois atualmente a resolução e a qualidade das imagens vem aumentando, tornando
      estes laços de repetições com muitas iterações.
      A paralelização abaixo está sobre o loop externo que itera sobre as linhas. As variáveis clusterIndex e pixel 
      são marcadas como privadas para garantir que cada thread tenha suas próprias cópias dessas variáveis, para
      não gerar conflito ao acessar o mesmo espaço de memória ao mesmo tempo. 
      As variáveis clustermap e labels não foram privatizadas, pois nenhuma iteração vai acessar a mesma posição
      destas variáveis, pois as iterações serão divididas entre as threads e o i e o j para acessar cada posição
      serão diferentes.
  */
  #pragma omp parallel for private(clusterIndex, pixel)
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      for (int k = 0; k < 3; k++)
      {
        pixel[k] = bitmap[(i * cols + j) * 3 + k];
      }
      clusterIndex = GetClusterPixelIndex(pixel, clusters, clustercount);
      for (int k = 0; k < 3; k++)
      {
        pixel[k] = clusters[clusterIndex * 3 + k];
      }
      for (int l = 0; l < 3; l++)
      {
        clustermap[(i * cols + j) * 3 + l] = pixel[l];
      }
      // Assign Label
      labels[i * cols + j] = clusterIndex;
    }
  }
}

void updateOldCluster(bit *prevCluster, bit *currCuster, short a, int b)
{
  for (int i = 0; i < a; i++)
  {
    for (int j = 0; j < b; j++)
    {
      prevCluster[i * b + j] = currCuster[i * b + j];
    }
  }
}

void ApplyKmeans(bit *bitmap, bit *clustermap, bit *clusters, bit *labels, short clustercount, int rows, int cols, int iterations)
{
  bit *prevClusters = (bit *)malloc(clustercount * 3 * sizeof(bit));

  AllocateRandomClusters(clusters, clustercount);

  for (int i = 0; i < iterations; i++)
  {
    // PrintFunc(clusters, clustercount, 3);

    if (ClusterCompare(prevClusters, clusters, clustercount, 3) && i != 0)
    {
      printf("No. of iterations: %d\n\n", i + 1);
      break;
    }

    updateOldCluster(prevClusters, clusters, clustercount, 3);

    GetClusteredImage(bitmap, clustermap, clusters, labels, clustercount, rows, cols);

    RecalculateClusters(bitmap, clusters, clustercount, labels, rows, cols);
  }
}

int main(int argc, char *argv[])
{
  FILE *ifp;
  bit *bitmap, *clustermap, *clusters, *labels;
  int ich1, ich2, rows, cols, maxval = 255, ppmraw;
  int i, j, k, iterations, num_threads;
  short cluster_count;

  /* Arguments */
  if (argc != 5)
  {
    printf("\nUsage: %s file k iterations num_threads\n\n", argv[0]);
    exit(0);
  }

  /* Opening */
  ifp = fopen(argv[1], "r");
  if (ifp == NULL)
  {
    printf("error in opening file %s\n", argv[1]);
    exit(1);
  }

  cluster_count = atoi(argv[2]);
  iterations = atoi(argv[3]);
  num_threads = atoi(argv[4]);

  omp_set_num_threads(num_threads);
  printf("\nRunning with %d threads\n", num_threads);

  if (cluster_count < 1 || iterations < 1)
  {
    printf("Invalid size of k or iterations\n");
    exit(1);
  }

  /*  Magic number reading */
  ich1 = getc(ifp);
  if (ich1 == EOF)
    pm_erreur("EOF / read error / magic number");
  ich2 = getc(ifp);
  if (ich2 == EOF)
    pm_erreur("EOF /read error / magic number");
  if (ich2 != '3' && ich2 != '6')
    pm_erreur(" wrong file type ");
  else if (ich2 == '3')
    ppmraw = 0;
  else
    ppmraw = 1;

  /* Reading image dimensions */
  cols = pm_getint(ifp);
  rows = pm_getint(ifp);
  maxval = pm_getint(ifp);

  /* Memory allocation  */
  bitmap = (bit *)malloc(cols * rows * 3 * sizeof(bit));
  clustermap = (bit *)malloc(cols * rows * 3 * sizeof(bit));

  // Cluster Centroid Matrix
  clusters = (bit *)malloc(cluster_count * 3 * sizeof(bit));

  // Label Matrix
  labels = (bit *)malloc(cols * rows * sizeof(bit));

  /* Reading */
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
      for (k = 0; k < 3; k++)
        if (ppmraw)
          bitmap[(i * cols + j) * 3 + k] = pm_getrawbyte(ifp);
        else
          bitmap[(i * cols + j) * 3 + k] = pm_getint(ifp);

  ApplyKmeans(bitmap, clustermap, clusters, labels, cluster_count, rows, cols, iterations);

  /* Writing */

  // char *ipFilNam;
  // ipFilNam = remove_ext(argv[1], '.', '/');
  // strncat(ipFilNam, "-k%d-i$d", cluster_count, iterations, 32);
  FILE *ifpWrite = fopen("pix/fileWritten.ppm", "w");

  if (ppmraw)
    fprintf(ifpWrite, "P3\n");
  else
    printf("P6\n");

  fprintf(ifpWrite, "%d %d \n", cols, rows);
  fprintf(ifpWrite, "%d\n", maxval);

  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
      for (k = 0; k < 3; k++)
        if (ppmraw)
          fprintf(ifpWrite, "%d ", clustermap[(i * cols + j) * 3 + k]);
        else
          fprintf(ifpWrite, "%c", clustermap[(i * cols + j) * 3 + k]);

  free(bitmap);
  free(clustermap);
  free(clusters);
  free(labels);

  /* Closing */
  fclose(ifp);
  fclose(ifpWrite);
  return 0;
}
