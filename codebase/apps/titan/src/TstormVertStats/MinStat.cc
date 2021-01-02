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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:01:42 $
//   $Id: MinStat.cc,v 1.2 2016/03/04 02:01:42 dixon Exp $
//   $Revision: 1.2 $
//   $MinState: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MinStat: Min statistic calculator
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "MinStat.hh"

using namespace std;


const string MinStat::STAT_NAME = "Min";


/*********************************************************************
 * Constructors
 */

MinStat::MinStat(const bool debug_flag) :
  Stat(STAT_NAME, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

MinStat::~MinStat()
{
}


/*********************************************************************
 * calcStatistic() - Calculate the given statistic on the given data
 *                   values.
 *
 * Returns the calculated statistic on success, MISSING_DATA_VALUE on
 * failure.
 */

double MinStat::calcStatistic(const vector< double > &data_values)
{
  double min_val = 0.0;
  bool min_val_found = false;
  
  vector< double >::const_iterator data_iter;
  
  for (data_iter = data_values.begin(); data_iter != data_values.end();
       ++data_iter)
  {
    double data_value = *data_iter;
    
    if (min_val_found)
    {
      if (data_value < min_val)
	min_val = data_value;
    }
    else
    {
      min_val = data_value;
      min_val_found = true;
    }
      
  } /* endfor - data_iter */
    
  if (!min_val_found)
    return MISSING_DATA_VALUE;

  return min_val;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
