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
/////////////////////////////////////////////////////////////
// MM5DataUtil.hh
//
// Contains common MM5 forecast time calculations
//
// Curtis Caravone
// 2/11/2003
//
/////////////////////////////////////////////////////////////

#ifndef MM5DataUtil_H
#define MM5DataUtil_H

#include "MM5Data.hh"

using namespace std;

/********
 * Returns the unix time of the END of the assimilation
 * period.  This calculation depends on several of the
 * MM5 parameters, including whether FDDA was used and
 * whether or not FDDA ramping was used.  For this
 * calculation, the assimilation period does not include
 * the ramping period, if any.
 */
time_t getAssimilationEndTime(const MM5Data &data);

inline time_t getAssimilationEndTime(MM5Data *data) {
    return getAssimilationEndTime(*data);
}

/********
 * Returns the forecast lead time in seconds, as 
 * measured from the END of the assimilation period.
 */
int getAdjustedForecastLeadTime(const MM5Data &data);

inline int getAdjustedForecastLeadTime(MM5Data *data) {
    return getAdjustedForecastLeadTime(*data);
}

#endif

