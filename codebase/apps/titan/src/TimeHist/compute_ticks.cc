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
/************************************************************************
 * compute_ticks.c
 *
 * Computes the tick mark spacing
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

void compute_ticks(long start_time,
		   long end_time,
		   long *nticks,
		   long *first_tick_time,
		   long *tick_interval)

{

  double delta_mins, delta_secs;
  
  delta_secs = (double) (end_time - start_time);
  delta_mins = delta_secs / 60.0;
  
  if(delta_mins > 480)
    *tick_interval = 120 * 60;
  else if (delta_mins > 240)
    *tick_interval = 60 * 60;
  else if (delta_mins > 120)
    *tick_interval = 30 * 60;
  else if (delta_mins > 90)
    *tick_interval = 20 * 60;
  else if (delta_mins > 60)
    *tick_interval = 15 * 60;
  else if (delta_mins > 30)
    *tick_interval = 10 * 60;
  else if (delta_mins > 10)
    *tick_interval = 5 * 60;
  else if (delta_mins > 5)
    *tick_interval = 2 * 60;
  else
    *tick_interval = 1 * 60;
  
  *nticks = 
    (long) (delta_secs / (double) *tick_interval) + 1;
  
  *first_tick_time =
    (((start_time / *tick_interval) + 1) * *tick_interval);

}
