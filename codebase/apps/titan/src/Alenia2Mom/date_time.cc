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
 * date_time.c
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * July 1997.
 ********************************************************************/

#include "Alenia2Mom.h"
using namespace std;

static int bcd2int(ui08 bcd_byte);

/*************
 * decode_date
 *
 * Loads up date_time_t struct with date and time
 *
 * returns 0 on success,  -1 on failure
 */

int decode_date(alenia_header_t *al_header, date_time_t *beam_time)
     
{
  
  beam_time->year = bcd2int(al_header->year);
  beam_time->month = bcd2int(al_header->month);
  beam_time->day = bcd2int(al_header->day);
  beam_time->hour = bcd2int(al_header->hour);
  beam_time->min = bcd2int(al_header->min);
  beam_time->sec = bcd2int(al_header->sec);
    
  /*
   * make sure year is in full resolution
   */
  
  if (beam_time->year < 1900) {
    if (beam_time->year > 70)
      beam_time->year += 1900;
    else
      beam_time->year += 2000;
  }
  
  /*
   * convert to unix time
   */
  
  uconvert_to_utime(beam_time);
  
  if (Glob->params.time_correction != 0) {
    beam_time->unix_time += Glob->params.time_correction;
    uconvert_from_utime(beam_time);
  }
  
  /*
   * check for validity
   */

  if (!uvalid_datetime(beam_time)) {
    return (-1);
  } else {
    return (0);
  }

}

static int bcd2int(ui08 bcd_byte)

{
  int units, tenths;
  units = bcd_byte & 0x0f;
  tenths= (bcd_byte >> 4) & 0x0f;
  return (tenths * 10 + units);
}

