/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*
 * Name: TEST_toolsa_mem.c
 *
 * Purpose:
 *
 *      To test the MEM module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/mem.h>
 *      and <toolsa/membuf.h>
 *
 * Usage:
 *
 *       % TEST_toolsa_mem
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Mike Dixon, July 1996
 *
 */

/*
 * include files
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <toolsa/mem.h>

#define NTEST 100

static int test_mem_alloc(void)

{

  int i, j;
  
  int *i1;
  float **f2;
  double ***d3;
  
  fprintf(stdout, "\nStart - test_mem_alloc\n");

  umalloc_debug(3);

  i1 = ucalloc(NTEST, sizeof(int));
  memset(i1, 1, NTEST * sizeof(int));
  
  i1 = urealloc(i1, NTEST * NTEST * sizeof(int));
  memset(i1, 2, NTEST * NTEST * sizeof(int));
  
  i1 = urealloc(i1, NTEST * NTEST * NTEST * sizeof(int));
  memset( i1, 3, NTEST * NTEST * NTEST * sizeof(int));
  
  f2 = (float **) ucalloc2(NTEST, NTEST * NTEST, sizeof(float));
  
  for (i = 0; i < NTEST; i++) {
    memset(f2[i], 4, NTEST * NTEST * sizeof(float));
  }

  d3 = (double ***) ucalloc3(NTEST, NTEST, NTEST, sizeof(double));

  for (i = 0; i < NTEST; i++) {
    for (j = 0; j < NTEST; j++) {
      memset(d3[i][j], 5, NTEST * sizeof(double));
    }
  }
  
  umalloc_verify();
  umalloc_count();
  umalloc_map();

  ufree3((void ***) d3);
  ufree2((void **) f2);
  ufree(i1);

  umalloc_verify();
  umalloc_count();
  umalloc_map();

  fprintf(stdout, "\nDone  - test_mem_alloc\n");

  return (0);

}

static int test_membuf(void)

{

  char *c, *d;
  int i;
  
  MEMbuf *b1, *b2, *b3;

  fprintf(stdout, "\nStart - test_membuf\n");

  b1 = MEMbufCreate();

  c = ucalloc(NTEST * NTEST, sizeof(char));
  memset(c, 1, NTEST * NTEST * sizeof(char));
  
  MEMbufLoad(b1, c, NTEST * NTEST);
  fprintf(stdout, "membuf 1: len is %d\n", (int) MEMbufLen(b1));

  b2 = MEMbufDup(b1);

  for (i = 0; i < NTEST; i++) {
    MEMbufAdd(b2, c, NTEST * NTEST);
  }
  fprintf(stdout, "membuf 2: len is %d\n", (int) MEMbufLen(b2));

  b3 = MEMbufCreate();
  d = MEMbufPrepare(b3, NTEST);
  memcpy(d, c, NTEST);
  fprintf(stdout, "membuf 3: len is %d\n", (int) MEMbufLen(b3));
  
  MEMbufConcat(b3, b2);

  umalloc_verify();
  umalloc_count();
  umalloc_map();

  MEMbufPrint(b1, stdout);
  MEMbufPrint(b2, stdout);
  MEMbufPrint(b3, stdout);

  MEMbufDelete(b1);
  MEMbufDelete(b2);
  MEMbufDelete(b3);
  
  umalloc_verify();
  umalloc_count();
  umalloc_map();

  fprintf(stdout, "\nDone  - test_membuf\n");

  return (0);

}


static void print_usage(char *prog_name, FILE *out)

{
    fprintf(out, "Usage: %s [-many]\n", prog_name);
}

/*--------------------------------*/

/*
 * main program driver
 */

int
main(int argc, char *argv[])
{

  int retval = 0;
  int i;
  int repeats = 1;
  double before, after;
  
  /*
   * check usage
   */

  if (argc > 1) {
    print_usage(argv[0], stderr);
    return (-1);
  }

  /*
   * Test the individual module subroutines
   */

  before = clock() / 1000000.0;

  for (i = 0; i < repeats; i++) {

    fprintf(stdout, "*** TEST_toolsa_mem - loop %d\n", i);

    if (test_mem_alloc()) {
	retval = 1;
    }
    
    if (test_membuf()) {
	retval = 1;
    }
    
    fflush(stdout);

  }
  
  if (retval) {
      fprintf(stderr, "\nMEM module failed test\n");
  } else {
      fprintf(stdout, "\nMEM module passed test\n");
  }
  
  after = clock() / 1000000.0;

  fprintf(stdout, "test_mem took %g CPU secs\n", after - before);

  return(retval);

}

