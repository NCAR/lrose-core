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
/*********************************************************************
 * PercentObsGEStatCalc: Class for calculating the percent of
 *                       observations greater than or equal to a given
 *                       value statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/climo/PercentObsGEStatCalc.hh>
#include <toolsa/str.h>

using namespace std;


/**********************************************************************
 * Constructor
 */

PercentObsGEStatCalc::PercentObsGEStatCalc(const double cutoff_value,
					   const bool debug_flag,
					   const bool check_z_levels) :
  PercentObsStatCalc(debug_flag, check_z_levels),
  _cutoffValue(cutoff_value)
{
}


/**********************************************************************
 * Destructor
 */

PercentObsGEStatCalc::~PercentObsGEStatCalc(void)
{
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
