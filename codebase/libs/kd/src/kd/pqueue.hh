// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2001 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Program(RAP) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2001/12/18 19:51:15 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*
 *   Module: pqueue.hh
 *
 *   Author: Gerry Wiener
 *
 *   Date:   11/6/01
 *
 *   Description: 
 *       Adapted from ranger code - http://www.cs.sunysb.edu/~algorith/implement/ranger/implement.shtml
 *
 *            The Algorithm Design Manual 
 *         by Steven S. Skiena, Steve Skiena
 */

#ifndef PQUEUE_HH
#define PQUEUE_HH

#include "../include/kd/datatype.hh"

void PQupheap(KD_real *DistArr, int *FoundArr, int k);
void PQInsert(KD_real distance, int index, KD_real *DistArr, int *FoundArr);
void PQdownheap(KD_real *DistArr, int *FoundArr, int k, int index);
void PQreplace(KD_real distance, KD_real *DistArr, int *FoundArr, int index);
void PQremove(KD_real *pdistance, KD_real *DistArr, int *FoundArr, int *pindex);

#endif /* PQUEUE_HH */
