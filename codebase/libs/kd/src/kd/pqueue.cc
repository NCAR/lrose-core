// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#include "pqueue.hh"
#include <iostream>
using namespace std;

//  Code to implement the abstract data type priority queue for use in j nearest     
//  neighbor searching.  Actual implementation is done using heaps.                  
//                                                                                   
//  Adapted from Sedgewick's: Algorithms in C p. 148-160.                            



//   The heap data structure consists of two priority queues.  One for the j-smallest
//   distances encountered, one to keep the indexes into the points array of the  
//   points corresponding to the j-smallest distances. 
//
//   KD_real *DistArr;   j-smallest distances encountered 



// Fix the heap assuming a potential problem at index k
void PQupheap(KD_real *DistArr, int *FoundArr, int k)
{
  KD_real v;
  int j;

  v=DistArr[k];
  DistArr[0] = 999999999999999.0;
  j=FoundArr[k];

  while(DistArr[k/2] <= v)
    {
      DistArr[k] = DistArr[k/2];
      FoundArr[k] = FoundArr[k/2];
      k=k/2;
    }

  DistArr[k] = v;
  FoundArr[k] = j;
}


// Insert distance and index onto their respective heaps
void PQInsert(KD_real distance, int index, KD_real *DistArr, int *FoundArr)
{
  FoundArr[0]=FoundArr[0]+1;	// The next available index
  DistArr[FoundArr[0]] = distance;
  FoundArr[FoundArr[0]] = index;
  PQupheap(DistArr,FoundArr,FoundArr[0]);
}



// Fix heap assuming a potential problem at the root
void PQdownheap(KD_real *DistArr, int *FoundArr, int k, int index)
{
  int j,N;
  KD_real v;

  v=DistArr[k];

  N = FoundArr[0];  /* tricky patch to maintain the data structure */
  FoundArr[0]=index;

  while (k <= N/2)
    {
      j=k+k;
      if (j < N && DistArr[j] <DistArr[j+1]) j++;
      if (v>=DistArr[j]) break;
      DistArr[k]=DistArr[j]; 
      FoundArr[k]=FoundArr[j];
      k=j;
    }

  DistArr[k] = v;
  FoundArr[k]= index;
  FoundArr[0]=N;  /* restore data struct */
}

// Replace largest item with new item unless 
void PQreplace(KD_real distance, KD_real *DistArr, int *FoundArr, int index)
{
  DistArr[0]=distance;
  PQdownheap(DistArr,FoundArr,0,index);
}


// Remove largest item
void PQremove(KD_real *pdistance, KD_real *DistArr, int *FoundArr, int *pindex)
{
  int N = FoundArr[0];

  *pdistance = DistArr[1];
  *pindex = FoundArr[1];
  DistArr[1] = DistArr[N];
  FoundArr[1] = FoundArr[N];
  PQdownheap(DistArr,FoundArr,1,FoundArr[1]);
  FoundArr[0] = N-1;	// Delete one item
}




