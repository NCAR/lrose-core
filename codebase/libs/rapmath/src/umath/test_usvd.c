/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2014 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** See LICENCE.TXT if applicable for licence details */
/* ** 2014/05/26 09:53:32 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */

/************************************************************************

Module:	test_usvd

Author:	C S Morse

Date:	Fri Sep 26 16:58:06 2003

Description: test driver for svd routines

************************************************************************/



/* System include files / Local include files */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>

# include <rapmath/usvd.h>
# include <rapmath/RMmalloc.h>

/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */

static char *Application = "test_usvd";

/* External functions / Internal global functions / Internal static functions */

extern int usvd( double **a, int nrow, int ncol, double **u, double **v, double *w );

static void read_command_arguments( int argc, char **argv );
static void usage( char **argv );
     

/************************************************************************

Function Name: 	usage

Description:	describes calling sequence

************************************************************************/

static void usage( char **argv )
{
  fprintf(stdout,"usage: %s [-h]\n", argv[0] );
  fprintf(stdout,"\nREQUIRED ARGUMENTS\n");
  fprintf(stdout,"\nOPTIONAL ARGUMENTS\n");
  fprintf(stdout,"\t-h outputs this usage message to stdout and exits\n");
}

/************************************************************************

Function Name: 	test_svd

Description:	test specific routines 

Returns:	

Globals:	

Notes:

************************************************************************/

static int test_svd( void )
{
  static double epsilon = 0.00001;
  int nerr = 0, save_nerr;
  
  int m = 3, n = 2;
  double **a,**u,**v,*w, **cvm, *x;
  int rc;
  int i,j,k;
  
  double matrx_data[] = { 1.,  0.,  0.54030231,  1., -0.41614684,   8. };
  double b_exact[] = { 2., 0.08060461, -8.83229367 }; 
  double x_exact[] = { 2., -1. }; 

  double xx[] = { -1., 0., 0.5, 1., 2., 3. }; 
  double yy[] = { 10., 5.,  4., 4., 7., 14. }; 
  double sg[] = {  1., 2., 0.5, 0.25, 1., 1. }; 
  double cf_soln[] = { 5., -3., 2. }; 
  double cf[3], cfsg[3], chisq;
  int ma=3;
  
  w = (double *) RMcalloc( n, sizeof(double) );
  a = (double **) RMcalloc2( m, n, sizeof(double) );
  u = (double **) RMcalloc2( m, n, sizeof(double) );
  v = (double **) RMcalloc2( n, n, sizeof(double) );
  cvm = (double **) RMcalloc2( n, n, sizeof(double) );
  x = (double *) RMcalloc( n, sizeof(double) );
  
  
  /* first test usvd - the decomposition itself */
  for ( i=0; i<m; i++ )
    for ( j=0; j<n; j++ )
      a[i][j] = u[i][j] = matrx_data[i*n+j];

  rc = usvd( a, m, n, u, v, w );
  usvd_print_dbg( 0, m, n, a, w, v );
  usvd_print_dbg( 1, m, n, u, w, v );
  
  /* compare Product u*w*(v-transpose) with original matrix */
  for ( i=0; i<m; i++ )
    {
      for ( j=0; j<n; j++ )
	{
	  for ( k=0; k<n; k++ )
	    a[i][j] -= u[i][k]*w[k]*v[j][k];
	  if ( fabs(a[i][j]) > epsilon )
	    {
	      fprintf(stderr,"usvd error: delta[%d][%d] = %8.4f\n",i,j,a[i][j]);
	      nerr++;
	    }
	}
    }

  /* Now test usvd_apply with an exact solution */
  fprintf( stderr, "Testing usvd_apply: \n");
  usvd_apply( u, w, v, m, n, b_exact, x );
  save_nerr = nerr;
  for ( i=0; i<n; i++ )
    if ( fabs(x[i] - x_exact[i]) > epsilon )
      {
	fprintf(stderr,"usvd_apply error: x[%d] error %9.5f\n", 
		i, x[i] - x_exact[i] );
	nerr++;
      }
  fprintf( stderr, "\t %d errors found in solution\n", nerr - save_nerr );
  
      
  /* Now test usvd_solve */
  fprintf( stderr, "Testing usvd_solve: \n");
  rc = usvd_solve( xx, yy, 6, ma, cf, sg, &chisq, cfsg, upolynomial );
  fprintf( stderr, "\t chisquare = %f\n", chisq );
  save_nerr = nerr;
  for ( i=0; i<ma; i++ )
    if ( fabs( cf[i] - cf_soln[i] ) > epsilon )
      {
	fprintf(stderr,"\tusvd_solve error: a[%d]  error %9.6f\n", 
		i, cf[i] - cf_soln[i] );
	nerr++;
      }
    else
      {
	/*
	  fprintf( stderr, "\t%d fit parameter, sigma = %f\t%f\n", 
	  i, cf[i], cfsg[i] );
	 */
      }
  fprintf( stderr, "\t %d errors found in solution\n", nerr - save_nerr );
  
  RMfree2((void**) a );
  RMfree2((void**) u );
  RMfree2((void**) v );
  RMfree2((void**) cvm );
  RMfree((char*) w );
  RMfree((char*) x );

  return( nerr );
}





/************************************************************************

Function Name: 	main

Description:	main routine for test_usvd

Returns:	

Globals:	

Notes:

************************************************************************/

int main( int argc, char **argv )
{
  int nerr;
  
  read_command_arguments( argc, argv );

  nerr = test_svd();
  printf( "usvd executed with %d errors\n");
  
  return( 0 );
}

/************************************************************************

Function Name: 	read_command_arguments

Description:	reads and processes command line arguments

Returns:	none

Globals:	

Notes:

************************************************************************/

static void read_command_arguments( int argc, char **argv )
{
  int i;
  
  for ( i=1; i<argc; i++ )
    {
      if ( strcmp( argv[i], "-h" ) == 0 || strcmp( argv[i], "-help" ) == 0 )
	{
	  usage( argv );
	  exit( 0 );
	}
      else
	{
	  fprintf( stdout, "Unrecognized command line argument '%s'\n", argv[i] );
	  usage( argv );
	  exit( 1 );
	}      
    }  
}
















