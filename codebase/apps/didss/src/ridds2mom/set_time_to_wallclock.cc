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
/*******************************************************************
 * set_time_to_wallclock.c
 *
 * Sets time in input to wallclock.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * June 1997.
 ********************************************************************/

#include "ridds2mom.h"
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <didss/ds_message.h>
#include <ctime>
using namespace std;

/*------------------------------------------------------------------------*/

void set_time_to_wallclock(ui08 *buffer)

{

  RIDDS_data_hdr *data_hdr;

  si32 millisecs_past_midnight; /* Collection time for this radial in
				 * millisecs of day past midnight (GMT). */

  si16 julian_date;             /* Modified julian date from 1/1/70 */

  date_time_t ttime;

  data_hdr = (RIDDS_data_hdr *) buffer;

  ttime.unix_time = time(NULL);
  uconvert_from_utime(&ttime);

  julian_date = ttime.unix_time / 86400 + 1;
  
  millisecs_past_midnight =
    ttime.hour * 3600000 + ttime.min * 60000 + ttime.sec * 1000;

  data_hdr->julian_date = BE_from_si16(julian_date);
  data_hdr->millisecs_past_midnight = BE_from_si32(millisecs_past_midnight);

}
