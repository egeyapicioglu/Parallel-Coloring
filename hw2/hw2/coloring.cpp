#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#ifdef __cplusplus
extern "C" {
#endif
#include "graphio.h"
#include "graph.h"
#ifdef __cplusplus
}
#endif

using namespace std;

char gfile[2048];

void usage() {
	printf("./coloring <filename>\n");
	exit(0);
}


/*
You can ignore the ewgths and vwghts. They are there as the read function expects those values
row_ptr and col_ind are the CRS entities. nov is the Number of Vertices
*/


int THREAD_NUMBER = 1;

int coloring(etype * row_ptr, vtype * col_ind, int nov, vector <int> & color, vector <int> & vertices)
{
  cout << "Function called..." << endl;
  cout << nov << endl;
  bool moveon = true;
	double start, end, diff;
	int i, j, x, y;
	int vertices_size = nov;
  int neighbour, neighbour_color, vector_size, current_vertice;
  vector < vector <int> > forbidden_array(nov, vector <int> (500, 0));
  cout << "Forbidden array is created..." << endl;
	while (moveon)
	{
    cout << "Coloring started... with vertices size : " << vertices_size <<  endl;
		start = omp_get_wtime();

    #pragma omp parallel for schedule(guided) private(neighbour, neighbour_color, vector_size, current_vertice) shared(color) num_threads(THREAD_NUMBER) // Coloring part
		for (i = 0; i < vertices_size; i++)
		{

			current_vertice = vertices[i];
      cout << current_vertice << endl;
			for (j = row_ptr[current_vertice]; j < row_ptr[current_vertice + 1]; j++)
			{
				neighbour = col_ind[j];
				neighbour_color = color[neighbour];

        if(neighbour_color != -1)
        {
          vector_size = forbidden_array[current_vertice].size();
          if(neighbour_color >= vector_size-1)
          {
            forbidden_array[current_vertice].resize(vector_size*2, 0);
          }
          forbidden_array[current_vertice][neighbour_color] = 1;
        }

			}


      int x = 0;
      while (forbidden_array[current_vertice][x] == 1)
      {
        x++;
      }
      color[current_vertice] = x;
    }
		end = omp_get_wtime();
		diff = end - start;
		cout << "Coloring ended..." << endl;
    cout << "Took " << diff << endl;
		vector <int> temp_array;
		int current_index = 0;
		printf("Conflict detection part goes off\n");
		start = omp_get_wtime();
    #pragma omp parallel for reduction(+:current_index) num_threads(THREAD_NUMBER) schedule(guided) // ERROR CATCH PART
		for (y = 0; y < vertices_size; y++)
		{
			int current_vertice = vertices[y];
			for (j = row_ptr[current_vertice]; j < row_ptr[current_vertice + 1]; j++)
			{
				int neighbour = col_ind[j];
				if (color[current_vertice] == color[neighbour] && color[current_vertice] != -1)
				{
					if (current_vertice > neighbour)
					{
            #pragma omp critical
						temp_array.push_back(current_vertice);
            color[current_vertice] = -1;
					}
					else
					{
            #pragma omp critical
						temp_array.push_back(neighbour);
            color[neighbour] = -1;
					}
				}
			}
		}
		end = omp_get_wtime();
		diff = end - start;
		printf("Conflict detection part took: %g\n", diff);

		int k;
		vertices.clear(); // Creating a new vertices array of smaller size here

		vertices_size = temp_array.size();
    vertices.resize(vertices_size);

    if (vertices_size == 0)
		{
			moveon = false;
		}
    else
    {
      		for (k = 0; k < vertices_size; k++)
      		{
      			vertices[k] = temp_array[k];
      		}
    }

    if(vertices_size < 500)
    {
      THREAD_NUMBER = 1;
    }
	}


  int max = 0;

	for (int z = 0; z < nov; z++)
	{
		if (color[z] > max)
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
	if (argc != 2)
		usage();

	const char* fname = argv[1];
	strcpy(gfile, fname);
	if (read_graph(gfile, &row_ptr, &col_ind, &ewghts, &vwghts, &nov, 0) == -1) {
		printf("error in graph read\n");
		exit(1);
	}



	vector <int> color(nov, 0);

	vector <int> vertices(nov);
  cout << "Starting the program..." << endl;
	for (int y = 0; y < nov; y++)
	{
		vertices[y] = y;
	}

	double start_time = omp_get_wtime();
  cout << "Calling function..."<<endl;
	int num_colors = coloring(row_ptr, col_ind, nov, color, vertices);
	double end_time = omp_get_wtime();
	double timetotal = end_time - start_time;
	cout << "Number of colors is: " << num_colors << endl;
	cout << "Total time spent is: " << timetotal << endl;


	return 1;
}
