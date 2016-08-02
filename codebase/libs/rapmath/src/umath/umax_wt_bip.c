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
/************************************************************************
 * umax_wt_bip.c - bipartite weighted matching algorithm.  See pg. 205,
 * Lawler, Combinatorial Optimization for details.
 *
 * August 1991
 * NCAR	
 * Gerry Wiener	
 *
 * Small changes (malloc etc) made by Mike Dixon
 *
 **************************************************************************/

#include <limits.h>

#include <rapmath/umath.h>
#include <rapmath/RMmalloc.h>

static void augment(long t, long *label, long *match);
static void clear_set(long *stack, char *set, long *top);
static void push_set(long *stack, char *set, long *top, long elt);
static long pop_set(long *stack, char *set, long *top);

/*
 * flag to indicate no match
 */

#define NO_MATCH_FLAG -1

/*
 * definitions for labels
 */

#define INIT_VAL -1
#define NO_LABEL -2

/*
 * bipartite set numbers
 */

#define SET_S 0
#define SET_T 1

/*
 * structure to describe edges in bipartite matching
 */

typedef struct {
  long size;
  long *vert;
} edge_t;

/*
 * Assume G = (S, T, E) is a bipartite graph with vertex sets S, T and
 * edges E.
 *
 * Inputs:
 *
 * umax_wt_bip(cost, sizeS, sizeT, no_edge_flag, match)
 *
 * cost - the cost matrix - the match should maximize the sum of entries
 *        from the cost matrix
 * sizeS - the number of vertices in S.
 * sizeT - the number of vertices in T
 *         NOTE: sizeS <= sizeT
 * flag - to indicate that an edge is invalid. Should be about half the
 *        max entry in the cost matrix
 *
 * Output: A maximum weighted matching of B.
 *
 * match - a maximum matching of B.
 */

void umax_wt_bip(long **cost, long sizeS, long sizeT,
		 long no_edge_flag, long *match)
{

  long sizeS_plus_sizeT;
  long delta, delta1, delta2;
  int finish = 0;
  long i, ii1, j, jj1, k, s;
  long num_scan;
  long num_unscan;

  long *invS;
  long *invT;
  long *label;
  long *set;
  long *S;
  long *T;

  char *scan_set;
  char *unscan_set;

  long *scan;
  long *unscan;
  long *U;
  long *V;
  long *pi;

  long *scanp;
  char *scan_setp;
  long *unscanp;
  char *unscan_setp;
  long *temp;
  char *tempc;

  edge_t *edge;

  /*
   * allocate arrays
   */

  sizeS_plus_sizeT = sizeS + sizeT;

  S = (long *) RMcalloc (sizeS, sizeof(long));
  U = (long *) RMcalloc (sizeS, sizeof(long));
  T = (long *) RMcalloc (sizeT, sizeof(long));
  V = (long *) RMcalloc (sizeT, sizeof(long));
  pi = (long *) RMcalloc (sizeT, sizeof(long));
  set = (long *) RMcalloc (sizeS_plus_sizeT, sizeof(long));
  label = (long *) RMcalloc (sizeS_plus_sizeT, sizeof(long));
  invS = (long *) RMcalloc (sizeS, sizeof(long));
  invT = (long *) RMcalloc (sizeS_plus_sizeT, sizeof(long));
  scan = (long *) RMcalloc (sizeS_plus_sizeT, sizeof(long));
  unscan = (long *) RMcalloc (sizeS_plus_sizeT, sizeof(long));
  scan_set = (char *) RMcalloc (sizeS_plus_sizeT, sizeof(char));
  unscan_set = (char *) RMcalloc (sizeS_plus_sizeT, sizeof(char));

  /*
   * allocate storage for and initialize the edges of the graph
   */

  edge = (edge_t *) RMcalloc (sizeS, sizeof(edge_t));

  for (i = 0; i < sizeS; i++) {

    edge[i].size = sizeT;

    edge[i].vert = (long *) RMcalloc (sizeT, sizeof(long));

    for (j = 0; j < sizeT; j++)
      edge[i].vert[j] = sizeS+j;

  } /* i */

  /*
   * initialize num_scan and num_unscan
   */

  num_scan = 0;
  num_unscan = 0;

  /* set up pointers for fast switching of scan and unscan sets */

  scanp = scan;
  unscanp = unscan;
  scan_setp = scan_set;
  unscan_setp = unscan_set;

  /*
   * find the size of the bipartite sets
   */

  for (i = 0; i < sizeS; i++) {
    S[i] = i;
    set[i] = SET_S;
  }

  for (i = sizeS; i < sizeS_plus_sizeT; i++) {
    T[i-sizeS] = i;
    set[i] = SET_T;
  }

  for (i = 0; i < sizeS; i++)
    invS[S[i]] = i;

  for (i = 0; i < sizeT; i++)
    invT[T[i]] = i;

  /*
   * initialize the match array
   */

  for (i = 0; i < sizeS; i++)
    match[S[i]] = NO_MATCH_FLAG;

  for (i = 0; i < sizeT; i++)
    match[T[i]] = NO_MATCH_FLAG;

  /*
   * initialize U such that U[i] is the max of the cost[i][j], for all j
   */

  for (i = 0; i < sizeS; i++) {
    U[i] = -1;
    for (j = 0; j < sizeT; j++) {
      if (cost[i][j] > U[i])
	U[i] = cost[i][j];
    }
  } /* i */
    
  /*
   * initialize V
   */

  for (i = 0; i < sizeT; i++)
    V[i] = 0;
    
 begin:

  /*
   * initialize pi
   */

  for (i = 0; i < sizeT; i++)
    pi[i] = INT_MAX;

  /*
   * initialize scan sets
   */

  clear_set(scanp, scan_setp, &num_scan);
  clear_set(unscanp, unscan_setp, &num_unscan);

  /*
   * remove all labels from nodes
   */

  for (i = 0; i < sizeS; i++)
    label[S[i]] = NO_LABEL;

  for (i = 0; i < sizeT; i++)
    label[T[i]] = NO_LABEL;

  /*
   * label the exposed nodes in S with NO_MATCH_FLAG
   */

  for (i = 0; i < sizeS; i++) {
    s = S[i];
    if (match[s] == NO_MATCH_FLAG) {
      label[s] = INIT_VAL;
      push_set(scanp, scan_setp, &num_scan, s);
    }
  } /* i */
  
 stage:
	      
  /*
   * while there are labels to be scanned, scan them
   */

  while (num_scan > 0) {
    
    i = pop_set(scanp, scan_setp, &num_scan);
    
    /*
     * check if vertex i is in set S
     */

    if (set[i] == SET_S) {

      for (k = 0; k < edge[i].size; k++) {

	j = edge[i].vert[k];

	if (match[i] != j) {

	  ii1 = invS[i];
	  jj1 = invT[j];

	  if (U[ii1] + V[jj1] - cost[ii1][jj1] < pi[jj1]) {
	    pi[jj1] = U[ii1] + V[jj1] - cost[ii1][jj1];
	    label[j] = i;
	    push_set(scanp, scan_setp, &num_scan, j);
	  }

	} /* if (match[i] != j) */

      } /* k */

    } else {

      /*
       * vertex i is in set T
       */

      ii1 = invT[i];

      if (pi[ii1] == 0) {

	if (match[i] == NO_MATCH_FLAG) {
	  augment(i, label, match);
	  goto begin;
	} else {
	  label[match[i]] = i;
	  push_set(scanp, scan_setp, &num_scan, match[i]);
	} /* if (match[i] == NO_MATCH_FLAG) */
	
      } else {

	push_set(unscanp, unscan_setp, &num_unscan, i);

      } /* if (pi[ii1] == 0) */

    } /* if (set[i] == SET_S) */

  } /* while */

  /*
   * change in dual variables
   */

  delta1 = INT_MAX;
      
  /*
   * find the minimum of U
   */

  for (i = 0; i < sizeS; i++)
    if (U[i] < delta1)
      delta1 = U[i];

  delta2 = INT_MAX;
      
  /*
   * find the minimum of pi > 0
   */

  for (i = 0; i < sizeT; i++)
    if (0 < pi[i] && pi[i] < delta2)
      delta2 = pi[i];
      
  if ((num_scan == 0 && num_unscan == 0) || finish) {

    /*
     * done
     */

    /*
     * set the first sizeS elements of the match array to indicate
     * the vertex at the end of each edge. If there is no valid
     * edge, set the value to the NO_MATCH_FLAG
     */

    for (i = 0; i < sizeS; i++) {

      if (match[i] != NO_MATCH_FLAG) {

	j = match[i] - sizeS;
	
	if (cost[i][j] == no_edge_flag)
	  match[i] = NO_MATCH_FLAG;
	else
	  match[i] = j;

      } /* if (match[i] != NO_MATCH_FLAG) */

    } /* i */
  
    /*
     * free up allocated space
     */

    for (i = 0; i < sizeS; i++)
      RMfree((char *) edge[i].vert);
    
    RMfree((char *) edge);
    RMfree((char *) S);
    RMfree((char *) U);
    RMfree((char *) T);
    RMfree((char *) V);
    RMfree((char *) pi);
    RMfree((char *) set);
    RMfree((char *) label);
    RMfree((char *) invS);
    RMfree((char *) invT);
    RMfree((char *) scan);
    RMfree((char *) unscan);
    RMfree((char *) scan_set);
    RMfree((char *) unscan_set);

    return;

  } /* if (num_scan == 0 && num_unscan == 0 || finish) */

  delta = MIN(delta1, delta2);

  for (i = 0; i < sizeS; i++)
    if (label[S[i]] != NO_LABEL)
      U[i] -= delta;

  for (j = 0; j < sizeT; j++) {
    if (pi[j] == 0) 
      V[j] += delta;
    else if (pi[j] > 0 && label[T[j]] != NO_LABEL)
      pi[j] -= delta;
  } /* j */

  if (delta <= delta1)  {

    /*
     * switch the scan and unscan sets
     */

    temp = scanp;
    scanp = unscanp;
    unscanp = temp;
    tempc = scan_setp;
    scan_setp = unscan_setp;
    unscan_setp = tempc;
    num_scan = num_unscan;

    /*
     * clear unscan_setp
     */

    clear_set(unscanp, unscan_setp, &num_unscan);

    if (delta == delta1)
      finish = 1;

      goto stage; 

  } /* if (delta <= delta1) */

}

/*
 * augment.c - routine used to find an augmenting path, see Lawler,
 * Combinatorial Optimization pg 195
 *
 * August 1991
 * NCAR	
 * Gerry Wiener	
 */

/*
 * given an exposed vertex t, a label array and a match array, augment
 * the current match
 */

static void augment(long t, long *label, long *match)
{
  long s;

  for (;;) {
    s = label[t];
    match[t] = s;
    match[s] = t;
    t = label[s];
    if (t == INIT_VAL)
      break;
  }

}
	    
/*
 * functions implementing set operations.
 *
 * These functions utilize a stack and an array of chars.  Once an
 * element is pushed onto the stack, its presence on the stack is noted
 * in the array, set.  Note that the elements of the set are restricted
 * to numbers 0 through the dimension of the array set - 1.
 *
 * November 1991
 * NCAR	
 * Gerry Wiener	
 *
 */

/* push elt onto stack and record presence on stack in set */

static void push_set(long *stack, char *set, long *top, long elt)
{

  if (set[elt] == 1) {
    return;
  } else {
    stack[(*top)++] = elt;
    set[elt] = 1;
  }
  return;
}

/* pop elt off stack and record absence in set */

static long pop_set(long *stack, char *set, long *top)
{
  long elt;

  elt = stack[--(*top)];
  set[elt] = 0;
  return(elt);

}

/* clear stack and set */

static void clear_set(long *stack, char *set, long *top)
{

  long i;

  for (i=0; i<*top; i++)
    set[stack[i]] = 0;

  *top = 0;

  return;

}
