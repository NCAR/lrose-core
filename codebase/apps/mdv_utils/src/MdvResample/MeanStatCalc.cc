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
/**
 *
 * @file MeanStatCalc.cc
 *
 * @class MeanStatCalc
 *
 * Class for calculating the mean value statistic.
 *  
 * @date 8/31/2011
 *
 */

#include <iostream>

#include "MeanStatCalc.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

MeanStatCalc::MeanStatCalc()
{
}


/**********************************************************************
 * Destructor
 */

MeanStatCalc::~MeanStatCalc()
{
}
  

/**********************************************************************
 * calculate()
 */

double MeanStatCalc::calculate(const vector< double > &values) const
{
  // If there are no values in the list, return the bad data value

  if (values.size() == 0)
    return BAD_DATA_VALUE;
  
  // Calculate the mean of the values

  double sum = 0.0;
  int num_values = 0;
  
  for (vector< double >::const_iterator value = values.begin();
       value != values.end(); ++value)
  {
    sum += *value;
    num_values++;
  } /* endfor - value */
  
  return sum / (double)num_values;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
