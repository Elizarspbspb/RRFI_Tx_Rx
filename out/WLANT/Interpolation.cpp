#include "Interpolation.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include <memory.h>
#include <gsl/gsl_complex_math.h>
#include <stdio.h>

float filter1[20] ={  -0.008180084f,  0.007237721f, -0.010439511f,  0.014670448f, -0.020335861f,
		   0.028250491f, -0.040098953f,  0.060358241f, -0.105497768f,  0.324035277f,
		   0.324035277f, -0.105497768f,  0.060358241f, -0.040098953f,  0.028250491f,
		  -0.020335861f,  0.014670448f, -0.010439511f,  0.007237721f, -0.008180084f }; 
		 
float filter2[30] = {  0.001532562f, -0.001674060f,  0.002554935f, -0.003713823f,  0.005213502f,
		  -0.007128707f,  0.009565063f, -0.012676025f,  0.016697756f, -0.022044785f,
		   0.029492361f, -0.040713548f,  0.060082826f, -0.103688115f,  0.316500058f,
		   0.316500058f, -0.103688115f,  0.060082826f, -0.040713548f,  0.029492361f,
		  -0.022044785f,  0.016697756f, -0.012676025f,  0.009565063f, -0.007128707f,
		   0.005213502f, -0.003713823f,  0.002554935f, -0.001674060f,  0.001532562f };

gsl_complex filter2_inv[59] = {
        {0.001532562,0},
        {0,0},
        {-0.00167406,0},
        {0,0},
        {0.002554935,0},
        {0,0},
        {-0.003713823,0},
        {0,0},
        {0.005213502,0},
        {0,0},
        {-0.007128707,0},
        {0,0},
        {0.009565063,0},
        {0,0},
        {-0.012676025,0},
        {0,0},
        {0.016697756,0},
        {0,0},
        {0.022044785,0},
        {0,0},
        {0.029492361,0},
        {0,0},
        {-0.040713548,0},
        {0,0},
        {0.060082826,0},
        {0,0},
        {-0.103688115,0},
        {0,0},
        {0.316500058,0},
        {0.5,0},
        {0.316500058,0},
        {0,0},
        {-0.103688115,0},
        {0,0},
        {0.060082826,0},
        {0,0},
        {-0.040713548,0},
        {0,0},
        {0.029492361,0},
        {0,0},
        {-0.022044785,0},
        {0,0},
        {0.016697756,0},
        {0,0},
        {-0.012676025,0},
        {0,0},
        {0.009565063,0},
        {0,0},
        {-0.007128707,0},
        {0,0},
        {0.005213502,0},
        {0,0},
        {-0.003713823,0},
        {0,0},
        {0.002554935,0},
        {0,0},
        {-0.00167406,0},
        {0,0},
        {0.001532562,0}
};


#if INTERPOLATION_FILTER_TYPE==1
#  define FILTER filter1
#  define FILTER_LEN 20
#  error No invFilter defined for this filer length
#elif INTERPOLATION_FILTER_TYPE==2
#  define FILTER filter2
#  define FILTER_LEN 30
#  define FILTER_INV filter2_inv
#  define FILTER_INV_LEN 59
#else
#  error Bad INTERPOLATION_FILTER_TYPE value
#endif
#define FILTER_HALF_LEN (FILTER_LEN>>1)

static gsl_complex interpolation_buffer[(1000+PreambleLength + MaxNumOfSymbols * 
                   ( CyclicPrefixLength + NumOfSubcarriers )) << MaxSamplingPower ];
static gsl_complex interpolation_buffer2[(1000+PreambleLength + MaxNumOfSymbols * 
                   ( CyclicPrefixLength + NumOfSubcarriers )) << MaxSamplingPower ];



/**
///////////////////////////////////////////////////////////////////////////////////////////
//Name:		Interpolation
//Purpose:	Increasing sample rate
//Author:	Dmitry Gusev, Vladislav Chernyshev
//Parameters:
//				input_signal			The pointer to input signal vector
//				interpolated_signal		The pointer to output signal vector
//				input_length			Length of the input signal (number of samles)
//        times             Oversampling rate (must be power of two)
//
// Returning value: number of samples in interpolated signal
/////////////////////////////////////////////////////////////////////////////////////////// */
int Interpolation( gsl_complex* input_signal,	        // The pointer to input vector
					          gsl_complex* interpolated_signal,   // The pointer to output vector
					          int input_length,            			  // Length of the input sequence
                    int times     
                  )
{
	int i,j,k;

  memcpy(interpolated_signal, input_signal, input_length*sizeof(gsl_complex));

  for(k=1;k<times;k<<=1){
    memcpy(interpolation_buffer, interpolated_signal, input_length*sizeof(gsl_complex));

	  for( i = 0; i < input_length; i++ )		{
		  interpolated_signal[ 2*i+1 ].dat[0] = 0.0f;
		  interpolated_signal[ 2*i ].dat[0] = interpolation_buffer[ i ].dat[0];

      interpolated_signal[ 2*i+1 ].dat[1] = 0.0f;
		  interpolated_signal[ 2*i ].dat[1] = interpolation_buffer[ i ].dat[1];
	  }

	  for( i = 0; i < FILTER_HALF_LEN-1; i++ )	{
		  for( j = 0; j < FILTER_HALF_LEN+1 + i; j++ )	{
			  interpolated_signal[ 2*i+1 ].dat[0] += 2 * interpolation_buffer[j].dat[0] * FILTER [FILTER_HALF_LEN+i-j];
        interpolated_signal[ 2*i+1 ].dat[1] += 2 * interpolation_buffer[j].dat[1] * FILTER [FILTER_HALF_LEN+i-j];
		  }
	  }

	  for( i = FILTER_HALF_LEN-1; i < input_length; i++ )		{
		  for( j = 0; j < FILTER_LEN; j++ )	{
			  if( i - (FILTER_HALF_LEN-1) + j < input_length )	{
				  interpolated_signal[ 2*i+1 ].dat[0] += 2 * interpolation_buffer[ i-(FILTER_HALF_LEN-1)+j ].dat[0]
                                                   * FILTER [ FILTER_LEN-1 - j ];
				  interpolated_signal[ 2*i+1 ].dat[1] += 2 * interpolation_buffer[ i-(FILTER_HALF_LEN-1)+j ].dat[1]
                                                   * FILTER [ FILTER_LEN-1 - j ];
			  }
		  }
	  }

    input_length*=2;
  }

  return input_length;
}



/**
///////////////////////////////////////////////////////////////////////////////////////////
//Name:		Decimation
//Purpose:	Reducing the sample rate
//Author:	Dmitry Gusev, Vladislav Chernyshev
//Parameters:
//				input_signal			 The pointer to input signal vector (must have space for
//                                1 additional sample at tail)
//				decimated_signal		 The pointer to output signal vector
//				input_length			Length of the input signal (number of samles)
//        times             Oversampling rate (must be power of two)
//
// Returning value: number of samples in decimated signal
/////////////////////////////////////////////////////////////////////////////////////////// */
int Decimation(  gsl_complex* input_signal,	      // The pointer to input signal vector
			            gsl_complex* decimated_signal,   // The poinetr to output signal vector
				          int input_length,            			  // Length of the input sequence
                  int times     
                  )
{
	int i,j,k;
  for(k=1;k<times;k<<=1){
    if(input_length & 1){
      input_signal[input_length]=input_signal[input_length-1];
      input_length++;
    }
    for( i = 0; i < input_length/2; i++ )	
    {
	    interpolation_buffer[i].dat[0] = 0.5f * input_signal[2*i].dat[0];
      interpolation_buffer[i].dat[1] = 0.5f * input_signal[2*i].dat[1];
    }
    for( i = 0; i < FILTER_HALF_LEN; i++ )	{
	    for( j = 0; j < FILTER_HALF_LEN + i; j++ )	{
		    interpolation_buffer[i].dat[0] += input_signal[1+2*j].dat[0] * FILTER [FILTER_HALF_LEN-1 +i-j];
		    interpolation_buffer[i].dat[1] += input_signal[1+2*j].dat[1] * FILTER [FILTER_HALF_LEN-1 +i-j];
	    }
    }
    for( i = FILTER_HALF_LEN; i < input_length/2; i++ )	{
	    for( j = 0; j < FILTER_LEN; j++ )	{
		    if( 2*i - (FILTER_LEN-1) + 2*j < input_length )		{
			    interpolation_buffer[i].dat[0] +=
              input_signal[2*(i+j)-(FILTER_LEN-1)].dat[0] * FILTER [FILTER_LEN-1 -j];
			    interpolation_buffer[i].dat[1] +=
              input_signal[2*(i+j)-(FILTER_LEN-1)].dat[1] * FILTER [FILTER_LEN-1 -j];
		    }
	    }
    }
    input_length/=2;
    memcpy(interpolation_buffer + input_length, interpolation_buffer,
            input_length*sizeof(gsl_complex) );
    input_signal = interpolation_buffer + input_length;
  }
  memcpy(decimated_signal, input_signal, input_length*sizeof(gsl_complex));
  return input_length;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
///////////////////////////////////////////////////////////////////////////////////////////
//Name:		apply_decimation_filter
//Purpose:	applies decimation filter to signal and makes number of samples even. 
//Author:	Alexandr Kozlov, Vladislav Chernyshev
//Parameters:
//				input_signal			(input, output)  The pointer to input signal vector (must have space for
//                                1 additional sample at tail)
//				input_length			(input) Length of the input signal (number of samles)
//
// Returning value: number of samples in decimated signal
/////////////////////////////////////////////////////////////////////////////////////////// */
int apply_decimation_filter(
                  gsl_complex* input_signal,	      // The pointer to input signal vector
				          int input_length            			  // Length of the input sequence
                  )
{
	int i;

  if(input_length & 1){
    input_signal[input_length]=input_signal[input_length-1];
    input_length++;
  }

  memset( interpolation_buffer2,0,(input_length+FILTER_INV_LEN-1)*sizeof(gsl_complex));

  signal_complex_convolution( input_signal, 
                            input_length, 
                            FILTER_INV,
                            FILTER_INV_LEN,
                            interpolation_buffer2
                           );

  for(i=0;i < input_length; i++){
    input_signal[i].dat[0]=interpolation_buffer2[FILTER_INV_LEN/2 + i ].dat[0];
    input_signal[i].dat[1]=interpolation_buffer2[FILTER_INV_LEN/2 + i ].dat[1];
  }

  return input_length;
}

/**
///////////////////////////////////////////////////////////////////////////////////////////
//Name:		do_decimate
//Purpose:	decimates input signal
//Author:	Alexandr Kozlov, Vladislav Chernyshev
//Parameters:
//				input_signal			(input,output) The pointer to input signal vector
//				input_length			(input) Length of the input signal (number of samles)
//
// Returning value: number of samples in decimated signal
/////////////////////////////////////////////////////////////////////////////////////////// */
int do_decimate(
                  gsl_complex* input_signal,	      // The pointer to input signal vector
				          int input_length            			  // Length of the input sequence
                  )
{

  int i;
  for(i=0;i<input_length >> 1;i++){
    input_signal[i].dat[0]=input_signal[i<<1].dat[0];
    input_signal[i].dat[1]=input_signal[i<<1].dat[1];
  }

  return input_length >> 1;

}

/**
///////////////////////////////////////////////////////////////////////////////////////////
//Name:		Decimation2
//Purpose:	Reducing the sample rate (second version)
//Author:	Dmitry Gusev, Vladislav Chernyshev
//Parameters:
//				input_signal			 The pointer to input signal vector (must have space for
//                                1 additional sample at tail)
//				decimated_signal		 The pointer to output signal vector
//				input_length			Length of the input signal (number of samles)
//        times             Oversampling rate (must be power of two)
//
// Returning value: number of samples in decimated signal
/////////////////////////////////////////////////////////////////////////////////////////// */
int Decimation2(  gsl_complex* input_signal,	      // The pointer to input signal vector
			            gsl_complex* decimated_signal,   // The poinetr to output signal vector
				          int input_length,            			  // Length of the input sequence
                  int times     
                  )
{
	int k;


  memcpy(interpolation_buffer,input_signal,input_length*sizeof(gsl_complex));

  for(k=1;k<times;k<<=1){
    input_length = apply_decimation_filter(interpolation_buffer,input_length);
    input_length = do_decimate(interpolation_buffer,input_length);
  }

  memcpy(decimated_signal, interpolation_buffer, input_length*sizeof(gsl_complex));

  return input_length;
}




#ifdef TESTING_ROUTINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
///////////////////////////////////////////////////////////////////////////////////////////
//Name:		test_interpolation
//Purpose:	Tests interpolation and decimation routines
//Author:	Vladislav Chernyshev
//Parameters: none
/////////////////////////////////////////////////////////////////////////////////////////// */
void test_interpolation(){
  int i;
  
  FILE* fff=fopen("_ip.log","w");
  FILE* fff2=fopen("_ip2.log","w");

  gsl_complex a[800];
  gsl_complex b[800];
  gsl_complex c[800];
  gsl_complex c2[800];

  memset(a,0,800*sizeof(gsl_complex));
  memset(b,0,800*sizeof(gsl_complex));
  memset(c,0,800*sizeof(gsl_complex));
  memset(c2,0,800*sizeof(gsl_complex));

  for(i=0;i<100;i++){
    a[i].dat[0]=10.0*sin(2.0*3.14159265*(float)i/50.0);
    a[i].dat[1]=10.0*cos(2.0*3.14159265*(float)i/50.0);
  }

  Interpolation(a,b,100,4);
  Decimation(b,c,400,4);
  Decimation2(b,c2,400,4);

  for(i=0;i<800;i++){
    fprintf(fff,"%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\n",
      a[i].dat[0], a[i].dat[1], b[i].dat[0], b[i].dat[1], c[i].dat[0], c[i].dat[1] );
    fprintf(fff2,"%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\n",
      a[i].dat[0], a[i].dat[1], b[i].dat[0], b[i].dat[1], c2[i].dat[0], c2[i].dat[1] );
  }

  fclose(fff);
  fclose(fff2);

  if(1){
    exit(1);
  }

}

#endif // TESTING_ROUTINES


