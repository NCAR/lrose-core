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
/*
 *   Module: metric.hh
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

#ifndef METRIC_HH
#define METRIC_HH

#include "datatype.hh"

enum KD_METRICS {KD_EUCLIDEAN, KD_MANHATTAN, KD_L_INFINITY, KD_L_P};

int KD_ptInRect(const KD_real *pt, int dimension, const KD_real **RectQuery);
KD_real KD_EuclidDist2(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP);
KD_real KD_ManhattDist(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP);
KD_real KD_LInfinityDist(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP);
KD_real KD_LGeneralDist(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP);

#endif /* METRIC_HH */
