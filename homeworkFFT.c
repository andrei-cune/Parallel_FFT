#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

typedef double complex cplx;
int N;
cplx *in_arr;
cplx *out_arr;
int num_threads;


typedef struct args_struct{       //structura in caare vom pastra vectorii si numarul de pasi
	cplx *buf;                    // va reprezenta argumentul functiei de thread
	cplx *out;
	int step;
}Args_struct;

void _fft(cplx buf[], cplx out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * M_PI * i / n) * out[i + step];   //implementarea rosseta 
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}


void* threadFunction_2_fft(void *var)
{

	Args_struct args = *(Args_struct*)var;     //vom aplica fft ca functie de thread
											   //deci ma folosesc de structura de argumente

	if ( args.step < N) {
		_fft(args.out, args.buf, N, args.step * 2);
		_fft(args.out + args.step, args.buf + args.step, N, args.step * 2);  //
 
		for (int i = 0; i < N; i += 2 * args.step) {
			cplx t = cexp(-I * M_PI * i / N) * args.out[i + args.step];
			args.buf[i / 2]     = args.out[i] + t;
			args.buf[(i + N)/2] = args.out[i] - t;
		}
	}
	return NULL;
}

void* threadFunction_4_fft(void *var)	//functie pentru aplicarea transformarii pe 4 thread uri
{
									
	Args_struct args = *(Args_struct*)var;

	if ( args.step < N) {

		Args_struct new;
		new.buf = args.out;
		new.out = args.buf;
		new.step = args.step * 2;
		pthread_t x;
		pthread_create(&(x), NULL, threadFunction_2_fft, &new);  //vom crea inca 2 thread uri in interiorul functiei 
                   												 // pe langa cele 2 create in main
		_fft(args.out + args.step, args.buf + args.step, N, args.step * 2);

		pthread_join(x, NULL );
		for (int i = 0; i < N; i += 2 * args.step) {
			cplx t = cexp(-I * M_PI * i / N) * args.out[i + args.step];
			args.buf[i / 2]     = args.out[i] + t;
			args.buf[(i + N)/2] = args.out[i] - t;
		}

		

	}
	return NULL;
}

int main(int argc, char * argv[]) {

	pthread_t tid[num_threads];

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
	
	in_arr = malloc( N * sizeof( cplx ));
	out_arr = malloc( N * sizeof( cplx));

	int i;
	for( i = 0 ; i < N ; i++ )
	{	double a, b = 0;
		read_res = fscanf(fin, "%lf" , &a );
		if (read_res == 0 )
		{
	    	fprintf(stdout, "Not read");   //citirea
	  	}
	  	in_arr[i] = a + I * b;
	  	out_arr[i] = in_arr[i];
	}

	if( num_threads == 1 )  //1 thread
	{
		
		_fft(in_arr,out_arr,N,1);

	}
	else if( num_threads == 2 )
	{	
		

		int initial_step = 1;
		Args_struct args1,args2;

		args1.buf = out_arr;    //argumentele fucntiei de thread
		args1.out = in_arr;
		args1.step = initial_step * 2;
		

		args2.buf = out_arr + initial_step;
		args2.out = in_arr + initial_step;
		args2.step = initial_step * 2;
		

		pthread_create(&(tid[0]), NULL, threadFunction_2_fft, &args1);  //primul thread
		pthread_create(&(tid[1]), NULL, threadFunction_2_fft, &args2);  // al doilea

		pthread_join(tid[0], NULL );
		pthread_join(tid[1], NULL );

		for (int i = 0; i < N; i += 2 * initial_step) {
			cplx t = cexp(-I * M_PI * i / N) * out_arr[i + initial_step];
			in_arr[i / 2]     = out_arr[i] + t;
			in_arr[(i + N)/2] = out_arr[i] - t;
		}

	}
	else if( num_threads == 4 )
	{
		pthread_t tid[2];

		int initial_step = 1;
		Args_struct args1,args2;

		args1.buf = out_arr;  //argumentele
		args1.out = in_arr;
		args1.step = initial_step * 2;
		

		args2.buf = out_arr + initial_step;
		args2.out = in_arr + initial_step;
		args2.step = initial_step * 2;
		

		pthread_create(&(tid[0]), NULL, threadFunction_4_fft, &args1); //primele 2 threaduri
		pthread_create(&(tid[1]), NULL, threadFunction_4_fft, &args2);

		pthread_join(tid[0], NULL );
		pthread_join(tid[1], NULL );

		for (int i = 0; i < N; i += 2 * initial_step) {
			cplx t = cexp(-I * M_PI * i / N) * out_arr[i + initial_step];
			in_arr[i / 2]     = out_arr[i] + t;
			in_arr[(i + N)/2] = out_arr[i] - t;
		}

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
  		fprintf(fout, "%lf %lf\n",creal(in_arr[i]),cimag(in_arr[i]) );  //vom scrie in final din vectorul de input
  	}

  	free(in_arr);
  	free(out_arr);
	fclose(fin);
	fclose(fout);

	return 0;
}
