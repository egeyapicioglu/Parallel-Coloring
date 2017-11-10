#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "graphio.h"
#include "graph.h"

char gfile[2048];

void usage(){
  printf("./coloring <filename>\n");
  exit(0);
}


/*
  You can ignore the ewgths and vwghts. They are there as the read function expects those values
  row_ptr and col_ind are the CRS entities. nov is the Number of Vertices
*/
int tail = 1;
int head = 0;
int current_range = 1;
int * color;
int * vertices;
int boolean = 1;
int THREAD_NUMBER = 16;
int coloring(etype * row_ptr, vtype * col_ind, int nov)
{
  double start, end, diff;
  int i, j, x, y;
  int vertices_size = nov;
  while (boolean)
  {
    printf("vertices_size %d \n", vertices_size);
    printf("Iterative algo goes off\n");
    start = omp_get_wtime();
    #pragma omp parallel for shared(color) num_threads(THREAD_NUMBER) schedule(guided) // Coloring part
    for (i = 0; i < vertices_size; i++)
    {
      int forbidden_array[nov];
      memset(forbidden_array, 0, nov*sizeof(int));
      int current_vertice = vertices[i];
      for(j = row_ptr[current_vertice]; j < row_ptr[current_vertice+1]; j++)
      {
        int neighbour = col_ind[j];
        int neighbour_color = color[neighbour];
        forbidden_array[neighbour_color] = 1;
      }

      int x = 0;
      while (forbidden_array[x] == 1)
      {
        x++;
      }
      color[current_vertice] = x;
    }
    end = omp_get_wtime();
    diff = end - start;
    printf("Iterative part took: %g\n", diff);

    int temp_array[vertices_size];
    memset(temp_array, 0, vertices_size*sizeof(int));
    int current_index = 0;
    printf("Speculation and analysis part goes off\n");
    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:current_index) shared(color) num_threads(THREAD_NUMBER) schedule(guided) // ERROR CATCH PART
    for (y = 0; y < vertices_size ; y++)
    {
      int current_vertice = vertices[y];
      for(j = row_ptr[current_vertice]; j < row_ptr[current_vertice+1]; j++)
      {
        int neighbour = col_ind[j];
        if (color[current_vertice] == color[neighbour])
        {
          if(current_vertice > neighbour)
          {
              #pragma omp atomic write
              temp_array[current_index] = current_vertice;
              current_index += 1;
          }
          else
          {
            #pragma omp atomic write
            temp_array[current_index] = neighbour;
            current_index += 1;
          }
        }
      }
    }
    end = omp_get_wtime();
    diff = end - start;
    printf("Speculation part took: %g\n", diff);
    free(vertices);
    int k;
    vertices = malloc((current_index+1)*sizeof(int)); // Creating a new vertices array of smaller size here
    vertices_size = current_index + 1;
    #pragma omp parallel for shared(vertices, temp_array) schedule(guided) num_threads(THREAD_NUMBER)
    for(k = 0; k < current_index+1; k++)
    {
      vertices[k] = temp_array[k];
    }
    if(k == 0)
    {
      boolean = 0;
    }
  }
    int max = 0;
    for (int z = 0; z < nov; z++)
    {
      if(color[z] > max)
      {
        max = color[z];
      }
    }
    return max+1;
}

int main(int argc, char *argv[]) {
  etype *row_ptr;
  vtype *col_ind;
  ewtype *ewghts;
  vwtype *vwghts;
  vtype nov;
  if(argc != 2)
    usage();

  const char* fname = argv[1];
  strcpy(gfile, fname);
  if(read_graph(gfile, &row_ptr, &col_ind, &ewghts, &vwghts, &nov, 0) == -1) {
    printf("error in graph read\n");
    exit(1);
  }



  color = malloc(nov * sizeof(int));

  for(int i = 0; i < nov; i++)
    {
      color[i] = 0;
    }

  vertices = malloc(nov * sizeof(int));

  for (int y = 0; y < nov; y++)
  {
    vertices[y] = y;
  }
  double start_time = omp_get_wtime();
  int num_colors = coloring(row_ptr, col_ind, nov);
  double end_time = omp_get_wtime();
  double timetotal = end_time - start_time;
  printf("Number of colors is: %d\n", num_colors);
  printf("Total time spent is: %g\n", timetotal);
  free(row_ptr);
  free(col_ind);

  return 1;
}
