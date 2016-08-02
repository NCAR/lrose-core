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
////////////////////////////////////////////////////////////////////
//
//
// /file <spol_angles.hh>
//
// For passing angle information between SPOL processes
//
// Mike Dixon, RAL, NCAR, Boulder, CO
// Feb 2011
//
//////////////////////////////////////////////////////////////////////

#ifndef _SPOL_ANGLES_HH_
#define _SPOL_ANGLES_HH_

#include <dataport/port_types.h>
#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////////
//
// spol_angles_short
//
// Short packet with antenna angles.
// For inter-process communication
//
/////////////////////////////////////////////////////////////////////////
  
#define SPOL_SHORT_ANGLE_ID 0x66660001 /**< ID for angles struct */
  
typedef struct spol_short_angle {
  
  si32 id;          /**< SPOL_ANGLES_SHORT_ID */
  si32 len_bytes;   /**< size of struct */
  
  si64 time_secs_utc;    /**< secs since Jan 1 1970 */
  si32 time_nano_secs;   /**< partial secs */

  fl32 elevation;   /**< antenna elevation */
  fl32 azimuth;     /**< antenna azimuth */

  ui32 seq_num;     /**< sequence number */

} spol_short_angle_t;

//////////////////////////////////////////////////////////////////
// swapping
//
// swap to native as required
// swapping is the responsibility of the user, data is always
// written in native

// check if ID needs swapping

inline bool spolIdIsSwapped(si32 id) {
  if ((id & 0x0000ffff) == 0x00006666) {
    // is swapped
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////
// init spol_short_angle struct

extern void spol_init(spol_short_angle_t &val);

//////////////////////////////////////////////////////
// swap packet
// returns true is swapped, false if already in native

extern bool spol_swap(spol_short_angle_t &val);

//////////////////////////////////////////////////////////////////
// get packet time as a double

extern double spol_get_time_as_double(const spol_short_angle_t &val);

//////////////////////////////////////////////////////////////////
// string representation of packet_id 

extern string spol_id_to_str(int packet_id);

//////////////////////////////////////////////////////////////////
// printing routines

extern void spol_print(FILE *out, const spol_short_angle_t &val);

#endif

