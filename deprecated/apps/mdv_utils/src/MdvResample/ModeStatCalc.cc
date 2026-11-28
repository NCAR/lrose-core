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
 * @file ModeStatCalc.cc
 *
 * @class ModeStatCalc
 *
 * Class for calculating the mode value statistic.
 *  
 * @date 8/31/2011
 *
 */

#include <iostream>
#include <map>

#include "ModeStatCalc.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

ModeStatCalc::ModeStatCalc(StatCalc *calc) :
  _calc(calc)
{
}


/**********************************************************************
 * Destructor
 */

ModeStatCalc::~ModeStatCalc()
{
  delete _calc;
}
  

/**********************************************************************
 * calculate()
 */

double ModeStatCalc::calculate(const vector< double > &values) const
{
  // If there aren't any values in the list, we can just return the bad
  // data value

  if (values.size() == 0)
    return BAD_DATA_VALUE;
  
  // If there is only 1 value in the list, save ourselves some trouble and
  // just return that value

  if (values.size() == 1)
    return values[0];
  
  // Create a histogram of the data values

  map< double, int > histogram;
  
  for (vector< double >::const_iterator value = values.begin();
       value != values.end(); ++value)
  {
    if (histogram.find(*value) == histogram.end())
      histogram[*value] = 1;
    else
      histogram[*value]++;
  } /* endfor - value */
  
  // Go through the histogram and find the maximum number of values

  int max_num_values = 0;
  
  for (map< double, int >::const_iterator hist_iter = histogram.begin();
       hist_iter != histogram.end(); ++hist_iter)
  {
    if (max_num_values < (*hist_iter).second)
      max_num_values = (*hist_iter).second;
  } /* endfor - hist_iter */
  
  // Create a list of the mode values

  vector< double > mode_values;
  
  for (map< double, int >::const_iterator hist_iter = histogram.begin();
       hist_iter != histogram.end(); ++hist_iter)
  {
    if ((*hist_iter).second == max_num_values)
      mode_values.push_back((*hist_iter).first);
  } /* endfor - hist_iter */
  
  // If there was only one mode value, we can return it here

  if (mode_values.size() == 0)
    return mode_values[0];
  
  // Use the calculator to calculate our final mode value

  return _calc->calculate(mode_values);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
