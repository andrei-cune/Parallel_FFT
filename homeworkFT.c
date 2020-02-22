#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

int N;
double *in_arr;
complex *out_arr;
int num_threads;

void* threadFunction(void *var)
{
	int thread_id = *(int*)var;
	int start = ( thread_id * N ) / num_threads;       // impartirea pe thread-uri
	int stop = ( thread_id + 1 ) * N / num_threads;    //fiecare thread va incepe de la start-ul propriu si se va opri
	                                                   // la stop -ul propriu
	int k;
	int x;
	 for( k = start ; k < stop ; k++ )
	 {
	 	complex double sum = 0.0;
	 	for( x = 0 ; x < N ; x++ )
	 	{
	 		double exp = -2 * M_PI * x * k  / N;
	 		sum += in_arr[x] * cexp(exp * I);
	 	}
	 	out_arr[k] = sum;
	 }
	
	return NULL;
}

int main(int argc, char * argv[]) {

	char file_in[500];
	char file_out[500];

	strcpy(file_in,argv[1]);
	strcpy(file_out,argv[2]);
	num_threads = atoi(argv[3]);

	FILE *fin = fopen(file_in,"rt");
	if (fin == NULL )
	{
    	fprintf(stdout, "Failed to open file.\n");
    	exit(1);
  	}

	int read_res;
	read_res = fscanf(fin,"%d" , &N);
	if (read_res == 0 )
	{
    	fprintf(stdout, "Not read");
  	}
	
	in_arr = malloc( N * sizeof( double ));
	out_arr = malloc( N * sizeof( complex double));

	int i;
	for( i = 0 ; i < N ; i++ )
	{
		read_res = fscanf(fin, "%lf" , &in_arr[i] );    //citirea datelor
		if (read_res == 0 )
		{
	    	fprintf(stdout, "Not read");
	  	}
	}

	pthread_t tid[num_threads];
	int thread_id[num_threads];

	for(i = 0;i < num_threads; i++)
		thread_id[i] = i;

	for(i = 0; i < num_threads; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

	for(i = 0; i < num_threads; i++) {
		pthread_join(tid[i], NULL);
	}

	FILE *fout = fopen(file_out,"wt");
	if (fout == NULL )
	{
    	fprintf(stdout, "Failed to open file.\n");
    	exit(1);
  	}

  	fprintf(fout,"%d\n",N);
  	for( i = 0 ; i < N ; i ++ )
  	{
  		fprintf(fout, "%lf %lf\n",creal(out_arr[i]),cimag(out_arr[i]) );    //scrierea datelor
  	}

	fclose(fin);
	fclose(fout);

	return 0;
}
