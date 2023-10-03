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

    Disponível em https://github.com/gabrielemirando/image-segmentation.git

    Mais informações no README.md
    
================================================================================================*/
/*================================================================================================

    Resultados:

    Informações do computador pessoal em que foram gerados os resultados apresentados:
    - CPU: 
    - Memory:  
    - Compiler: 
    - OS: 

    Resultados em máquina pessoal:

    1. Sequencial ----------------------------------------------- 

    2. Paralelo (1 thread) -------------------------------------- 

    3. Paralelo (2 threads) ------------------------------------- 

    2. Paralelo (4 threads) ------------------------------------- 

    3. Paralelo (8 threads) ------------------------------------- 

    ts = 1,899s
    tp = 0,560s

    Ganho = ts / tp = 3,391


    Resultados em servidor PARCODE:

    1. Sequencial ----------------------------------------------- 

    2. Paralelo (1 thread) -------------------------------------- 

    3. Paralelo (2 threads) ------------------------------------- 

    2. Paralelo (4 threads) ------------------------------------- 

    3. Paralelo (8 threads) ------------------------------------- 

    ts = 
    tp = 

    Ganho = ts / tp = 

================================================================================================*/
/*================================================================================================

    Análise de Resultados:



================================================================================================*/

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <omp.h>

#include "image_io.h"
#include "segmentation.h"

void init_centers(byte_t *data, double *centers, int n_px, int n_ch, int n_clus);
void assign_pixels(byte_t *data, double *centers, int *labels, double *dists, int *changes, int n_px, int n_ch, int n_clus);
void update_centers(byte_t *data, double *centers, int *labels, double *dists, int n_px, int n_ch, int n_clus);
void update_data(byte_t *data, double *centers, int *labels, int n_px, int n_ch);
void compute_sse(double *sse, double *dists, int n_px);

void kmeans_segm_omp(byte_t *data, int width, int height, int n_ch, int n_clus, int *n_iters, double *sse, int n_threads)
{
    int n_px;
    int iter, max_iters;
    int changes;
    int *labels;
    double *centers;
    double *dists;

    max_iters = *n_iters;

    n_px = width * height;

    labels = malloc(n_px * sizeof(int));
    centers = malloc(n_clus * n_ch * sizeof(double));
    dists = malloc(n_px * sizeof(double));

    omp_set_num_threads(n_threads);

    init_centers(data, centers, n_px, n_ch, n_clus);

    for (iter = 0; iter < max_iters; iter++)
    {
        assign_pixels(data, centers, labels, dists, &changes, n_px, n_ch, n_clus);

        if (!changes)
        {
            break; // impede a vetorização/paralelização do laço
        }

        update_centers(data, centers, labels, dists, n_px, n_ch, n_clus);
    }

    update_data(data, centers, labels, n_px, n_ch);

    compute_sse(sse, dists, n_px);

    *n_iters = iter;

    free(centers);
    free(labels);
    free(dists);
}

void init_centers(byte_t *data, double *centers, int n_px, int n_ch, int n_clus)
{
    int k, ch, rnd;

    for (k = 0; k < n_clus; k++)
    {
        rnd = rand() % n_px; // impede a vetorização/paralelização do laço

        for (ch = 0; ch < n_ch; ch++)
        {
            centers[k * n_ch + ch] = data[rnd * n_ch + ch];
        }
    }
}

void assign_pixels(byte_t *data, double *centers, int *labels, double *dists, int *changes, int n_px, int n_ch, int n_clus)
{
    int px, ch, k;
    int min_k, tmp_changes = 0;
    double dist, min_dist, tmp;

    // As variáveis da função declaradas acima são acessadas frequentemente em cada iteração
    // do laço de repetição, portanto, foram privatizadas de modo a evitar condições de corrida 
    // devido as threads compartilharem o mesmo espaço de memória para essas variáveis 
    //
    // Como não existe dependência direta entre as computações feitas em cada thread, não é 
    // necessário o uso da diretiva reduction
    #pragma omp parallel for schedule(static) private(px, ch, k, min_k, dist, min_dist, tmp)
    for (px = 0; px < n_px; px++)
    {
        min_dist = DBL_MAX;

        for (k = 0; k < n_clus; k++)
        {
            dist = 0;

            for (ch = 0; ch < n_ch; ch++)
            {
                tmp = (double)(data[px * n_ch + ch] - centers[k * n_ch + ch]);
                dist += tmp * tmp;
            }

            if (dist < min_dist)
            {
                min_dist = dist;
                min_k = k;
            }
        }

        dists[px] = min_dist;

        if (labels[px] != min_k)
        {
            labels[px] = min_k;
            tmp_changes = 1;
        }
    }

    *changes = tmp_changes;
}

void update_centers(byte_t *data, double *centers, int *labels, double *dists, int n_px, int n_ch, int n_clus)
{
    int px, ch, k;
    int *counts;
    int min_k, far_px;
    double max_dist;

    counts = malloc(n_clus * sizeof(int));

    // Resetting centers and initializing clusters counters

    // Computação relativamente simples (poucas iterações), não justificaria a paralelização 
    // para ganho significativo de desempenho em uma análise de escalabilidade forte
    #pragma omp parallel for private(k, ch)
    for (k = 0; k < n_clus; k++)
    {
        for (ch = 0; ch < n_ch; ch++)
        {
            centers[k * n_ch + ch] = 0;
        }

        counts[k] = 0;
    }

    // Computing partial sums of the centers and updating clusters counters

    // A diretiva private garante que cada thread tenha sua própria cópia das variáveis 
    // px, ch e min_k, evitando assim possíveis condições de disputa por acesso a um
    // mesmo espaço de memória
    //
    // Já a diretiva reduction realiza o mesmo para os arrays centers e counts, a única 
    // diferença sendo que existe dependência entre as computações feitas em cada thread 
    // e que, por isso, precisam ser somadas ao fim da execução do loop)
    #pragma omp parallel for private(px, ch, min_k) // reduction(+:centers[:n_clus * n_ch],counts[:n_clus])
    for (px = 0; px < n_px; px++)
    {
        min_k = labels[px];

        for (ch = 0; ch < n_ch; ch++)
        {
            #pragma omp atomic update
            centers[min_k * n_ch + ch] += data[px * n_ch + ch];
        }

        #pragma omp atomic update
        counts[min_k]++;
    }

    // Dividing to obtain the centers mean

    for (k = 0; k < n_clus; k++)
    {
        if (counts[k])
        {
            for (ch = 0; ch < n_ch; ch++)
            {
                centers[k * n_ch + ch] /= counts[k]; // impede a vetorização/paralelização do laço
            }
        }
        else
        {
            // If the cluster is empty we find the farthest pixel from its cluster center

            max_dist = 0;

            for (px = 0; px < n_px; px++)
            {
                if (dists[px] > max_dist)
                {
                    max_dist = dists[px];
                    far_px = px;
                }
            }

            for (ch = 0; ch < n_ch; ch++)
            {
                centers[k * n_ch + ch] = data[far_px * n_ch + ch];
            }

            dists[far_px] = 0;
        }
    }

    free(counts);
}

void update_data(byte_t *data, double *centers, int *labels, int n_px, int n_ch)
{
    int px, ch, min_k;

    // As variáveis da função declaradas acima são acessadas frequentemente em cada iteração
    // do laço de repetição, portanto, foram privatizadas de modo a evitar condições de corrida 
    // devido as threads compartilharem o mesmo espaço de memória para essas variáveis 
    //
    // Como não existe dependência direta entre as computações feitas em cada thread, não é 
    // necessário o uso da diretiva reduction
    #pragma omp parallel for schedule(static) private(px, ch, min_k)
    for (px = 0; px < n_px; px++)
    {
        min_k = labels[px];

        for (ch = 0; ch < n_ch; ch++)
        {
            data[px * n_ch + ch] = (byte_t)round(centers[min_k * n_ch + ch]);
        }
    }
}

void compute_sse(double *sse, double *dists, int n_px)
{
    int px;
    double res = 0;

    // A variável px (identificador do pixel) é constantemente acessada em cada iteração para 
    // servir de índice ao array distâncias euclidianas (dists), portanto, foi privatizada
    // de modo a evitar condições de disputa
    //
    // Já a variável res também necessita ser privatizada, só que via a diretiva reduction, 
    // visto que cada thread modifica o seu valor somando a distância euclidiana do pixel de
    // determinada iteração e que ao final da execução, é preciso reagrupar as somas parciais
    // feitas na variável res original
    #pragma omp parallel for private(px) reduction(+:res)
    for (px = 0; px < n_px; px++)
    {
        res += dists[px];
    }

    *sse = res;
}
