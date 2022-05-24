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
#include "test_rand.h"

#define mod_diff(x,y) (((x) -(y) ) &0x7fffffff)  \

static long A[56] = {-1};
long *gb_fptr = A;

long 
gb_flip_cycle(void)
{
  long *ii, *jj;
  for (ii = &A[1], jj = &A[32]; jj <= &A[55]; ii++, jj++)
    *ii = mod_diff(*ii, *jj);
  for (jj = &A[1]; ii <= &A[55]; ii++, jj++)
    *ii = mod_diff(*ii, *jj);
  gb_fptr = &A[54];
  return A[55];
}

void 
gb_init_rand(long seed)

{
  long i;
  long prev = seed, next = 1;
  seed = prev = mod_diff(prev, 0);
  A[55] = prev;
  for (i = 21; i; i = (i + 21) % 55)
    {
      A[i] = next;
      next = mod_diff(prev, next);
      if (seed & 1)
	seed = 0x40000000 + (seed >> 1);
      else
	seed >>= 1;
      next = mod_diff(next, seed);
      prev = A[i];
    }
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
}

long 
gb_unif_rand(long m)
{
  unsigned long t = two_to_the_31 - (two_to_the_31 % m);
  long r;
  do
    {
      r = gb_next_rand();
    }
  while (t <= (unsigned long)r);
  return r % m;
}

/*
 * random number given range
 */

double rand_in_range(double minval, double maxval)

{

  int randval;
  double normval;

  randval = gb_next_rand();
  normval = ((double) randval / (double)  two_to_the_31);

  return (minval + (maxval - minval) * normval);

}










