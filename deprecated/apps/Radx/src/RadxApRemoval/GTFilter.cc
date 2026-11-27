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
 * @file GTFilter.cc
 *
 * @class GTFilter
 *
 * GTFilter handles filtering of radar data, using a greater than comparison.
 *  
 * @date 7/21/2008
 *
 */

using namespace std;

#include "GTFilter.hh"


/**
 * Constructor
 */

GTFilter::GTFilter() :
  Filter()
{
}


/**
 * Destructor
 */

GTFilter::~GTFilter() 
{
  // Do nothing
}


/**
 * getFilterFlag()
 */

bool GTFilter::getFilterFlag(const int beam_num, const int gate_num)
{
  float final_int_value =
    _beamList[beam_num]->getInterestValue(gate_num,
					  FilterBeamInfo::FINALC);
  
  if (final_int_value == InterestFunction::MISSING_INTEREST)
  {
    if (_combineType == COMBINE_AND)
      return true;
    else
      return false;
  }
  
  if (final_int_value > _finalThreshold)
    return true;
  
  return false;
}
