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
 * @file PercentileStatCalc.cc
 *
 * @class PercentileStatCalc
 *
 * Class for calculating the percentile value statistic.
 *  
 * @date 8/31/2011
 *
 */

#include <algorithm>
#include <iostream>

#include "PercentileStatCalc.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

PercentileStatCalc::PercentileStatCalc(const double percentile) :
  _percentile(percentile / 100.0)
{
}


/**********************************************************************
 * Destructor
 */

PercentileStatCalc::~PercentileStatCalc()
{
}
  

/**********************************************************************
 * calculate()
 */

double PercentileStatCalc::calculate(const vector< double > &values) const
{
  // If there are no values to consider, return the bad data value

  if (values.size() == 0)
    return BAD_DATA_VALUE;
  
  // If there is only one data value, we can just return it and not do all
  // of the other work

  if (values.size() == 1)
    return values[0];
  
  // Make a copy of the data array and sort it

  vector< double > sorted_values = values;
  sort(sorted_values.begin(), sorted_values.end());
  
  // Find the desired value in the array

  int percentile_index =
    (int)(((double)sorted_values.size() * _percentile) + 0.5) - 1;

  if (percentile_index < 0)
    percentile_index = 0;
  
  if (percentile_index >= (int)sorted_values.size())
    percentile_index = sorted_values.size() - 1;

  return sorted_values[percentile_index];
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
