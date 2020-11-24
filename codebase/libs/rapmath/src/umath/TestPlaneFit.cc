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

Module:	TestPlaneFit

Author:	Mike Dixon

Date:	Nov 2020

Description: test driver for PlaneFit

************************************************************************/

/* System include files / Local include files */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <rapmath/PlaneFit.hh>
#include <rapmath/stats.h>
using namespace std;

/* External functions / Internal global functions / Internal static functions */

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

static int testPlaneFit()
{


  double aa = 0.5;
  double bb = -0.3;
  double cc = 25.0;

  PlaneFit plane;

  for (double yy = -20.0; yy <= 20.0; yy += 2) {
    for (double xx = -20.0; xx <= 20.0; xx += 2) {
      double err = STATS_normal_gen(0, 0.5);
      double zz = aa * xx + bb * yy + cc + err;
      plane.addPoint(xx, yy, zz);
    } // xx
  } // yy

  if (plane.performFit()) {
    cerr << "ERROR - performFit failed" << endl;
    return -1;
  }

  cerr << "FIT a: " << plane.getCoeffA() << endl;
  cerr << "FIT b: " << plane.getCoeffB() << endl;
  cerr << "FIT c: " << plane.getCoeffC() << endl;

  return 0;

}

/************************************************************************

Function Name: 	main

Description:	main routine for TestPlaneFit

Returns:	

Globals:	

Notes:

************************************************************************/

int main( int argc, char **argv )
{
  read_command_arguments(argc, argv);
  if (testPlaneFit()) {
    fprintf(stderr, "testPlaneFit failed\n");
  } else {
    fprintf(stderr, "testPlaneFit SUCCESS\n");
  }
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
  
  for ( i=1; i<argc; i++ ) {
    if ( strcmp( argv[i], "-h" ) == 0 || strcmp( argv[i], "-help" ) == 0 ) {
      usage( argv );
      exit( 0 );
    } else {
      fprintf( stdout, "Unrecognized command line argument '%s'\n", argv[i] );
      usage( argv );
      exit( 1 );
    }
  }      
}  
















