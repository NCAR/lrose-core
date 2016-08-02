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

#include <math.h>
#include "../include/kd/metric.hh"

using namespace std;

/* module contains implementations of the distance function for the
Euclidean, Manhattan, and L_Infinity metric */


int KD_ptInRect(const KD_real *pt, int dimension, const KD_real **RectQuery)
{
  for (int dc=0; dc < dimension; dc++)
    {
      if (pt[dc] < RectQuery[dc][0] || pt[dc] > RectQuery[dc][1])
	return(0);
    }

  return(1);
}
  

/* returns the square of the Euclidean distance of Points[Index] and NNQPoint */
KD_real KD_EuclidDist2(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP)
{
  KD_real dist,d;
  int j;

  dist=0;
  for (j=0; j<dimension; j++)
    {
      d=Points[Index][j]-NNQPoint[j];
      dist += d*d;
    }

  return(dist);
}


/* returns the manhattan distance between Points[Index] and NNQPoint */
KD_real KD_ManhattDist(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP)
{
  KD_real dist,d;
  int j;

  dist=0;
  for (j=0; j<dimension; j++)
    {
      d=Points[Index][j]-NNQPoint[j];
      dist = dist + fabs(d);
    }

  return(dist);
}


/* returns the square of the Euclidean distance of Points[Index] and NNQPoint */
KD_real KD_LInfinityDist(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP)
{
  KD_real dist,d;
  int j;

  dist=-999999999.0;
  for (j=0; j<dimension; j++)
    {
      d=fabs(Points[Index][j]-NNQPoint[j]);
      if (dist < d)
	{
	  dist = d;
	}
    }
  return(dist);
}


/* returns the generalized distance of Points[Index] and NNQPoint */
KD_real KD_LGeneralDist(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP)
{
  KD_real dist;
  int j;

  dist=0;
  for (j=0; j<dimension; j++)
    {
      dist +=fabs(pow(Points[Index][j]-NNQPoint[j], (KD_real)MinkP));
    }

  return(pow(dist,1.0/(KD_real)MinkP));
}
















