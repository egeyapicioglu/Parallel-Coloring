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


int THREAD_NUMBER = 16;

int coloring(etype * row_ptr, vtype * col_ind, int nov, vector <int> & color, vector <int> & vertices)
{
  cout << "Function called..." << endl;
  cout << nov << endl;
  bool moveon = true;
	double start, end, diff;
	int i, j, x, y;
	int vertices_size = nov;
  //vector < vector <int> > forbidden_array(nov));
  cout << "Forbidden array is created..." << endl;
	while (moveon)
	{
    cout << "Iteration started..." << endl;
		start = omp_get_wtime();
    #pragma omp parallel for shared(color) num_threads(THREAD_NUMBER) schedule(guided) // Coloring part
		for (i = 0; i < vertices_size; i++)
		{
			int current_vertice = vertices[i];
      vector <int> forbidden_array(nov, 0);
			for (j = row_ptr[current_vertice]; j < row_ptr[current_vertice + 1]; j++)
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
		cout << "Iteration ended..." << endl;
    cout << "Took " << diff << endl;
		vector <int> temp_array(vertices_size, 0);
		int current_index = 0;
		printf("Speculation and analysis part goes off\n");
		start = omp_get_wtime();
#pragma omp parallel for reduction(+:current_index) num_threads(THREAD_NUMBER) schedule(guided) // ERROR CATCH PART
		for (y = 0; y < vertices_size; y++)
		{
			int current_vertice = vertices[y];
			for (j = row_ptr[current_vertice]; j < row_ptr[current_vertice + 1]; j++)
			{
				int neighbour = col_ind[j];
				if (color[current_vertice] == color[neighbour])
				{
					if (current_vertice > neighbour)
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

		int k;
		vertices.clear(); // Creating a new vertices array of smaller size here

		vertices_size = current_index + 1;
    vertices.resize(vertices_size);
#pragma omp parallel for shared(vertices, temp_array) schedule(guided) num_threads(THREAD_NUMBER)
		for (k = 0; k < vertices_size; k++)
		{
			vertices[k] = temp_array[k];
		}
		if (k == 0)
		{
			moveon = false;
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
