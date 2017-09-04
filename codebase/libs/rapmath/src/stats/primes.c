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
/***************************************************************************
 ***                                                                        
 *** PrimeII by Stefan Ohlsson.                                             
 ***                                                                        
 *** This algoritm is using the following optimisations:                    
 ***                                                                        
 *** o The tested number is only checked with numbers upto its square-root  
 *** o The fact that all numbers can be written as a product of prime       
 ***   numbers is used:                                                     
 ***     Every number is only tested with prime numbers found earlier       
 *** o Only every other number is checked, numbers divisdable by 2 are not  
 ***   even considered.                                                     
 ***                                                                        
 ***************************************************************************/

#include <toolsa/toolsa_macros.h>
#include <rapmath/stats.h>
#include <rapmath/RMmalloc.h>

/*
** Storage for prime numbers
*/

/*****************
 * STATS_print_primes()
 *
 * Prints the first n primes to file out.
 *
 * Returns 0 on success, -1 on error.
 */

int STATS_print_primes(int n, FILE *out)

{

  long *prim;
  long pmax,pmaxseek,ptst,pnum;
  long a;
  long prim_found;

  prim = (long *) RMmalloc(n * sizeof(long));

  pmax=n;     /* Check upto n */
  ptst=3;           /* Start with 3 */
  pnum=1;
  prim[0]=2;        /* Init buffer */

  printf("Prime number seek upto %ld.\n",pmax);

  while(ptst < pmax) {

    prim_found=TRUE;

    /*
    ** Only check upto the square-root
    */
    pmaxseek=(long)sqrt((double)ptst)+1;
    
    /*
    ** Check with prime numbers found earlier
    */
    for(a=0;prim[a]<pmaxseek;a++) {
      if(!(ptst%prim[a])) {
	prim_found=FALSE;
	break;
      }
    }
    
    /*
    ** Store prime if found
    */
    if(prim_found)  { prim[pnum++]=ptst; fprintf(out, "%ld\n",ptst); }
    
    /*
    ** Next number to test
    */
    ptst+=2;
  }
  
  printf("%ld prime numbers found.\n",pnum);

  free(prim);

  return (0);

}

