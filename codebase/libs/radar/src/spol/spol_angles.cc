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
/////////////////////////////////////////////////////////////////
// spol_angles.cc
//
// Utility routines for spol_angle structs
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// Feb 2011

#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <radar/spol_angles.hh>

using namespace std;

//////////////////////////////////////////////////////////////////
// packet initialization
// sets values to missing as appropriate

//////////////////////////////////////////////////////
// init spol_short_angle struct

void spol_init(spol_short_angle_t &val)

{

  MEM_zero(val);
  val.id = SPOL_SHORT_ANGLE_ID; 
  val.len_bytes = sizeof(val);
  
}

//////////////////////////////////////////////////////
// swap packet
// returns true is swapped, false if already in native

bool spol_swap(spol_short_angle_t &val)
  
{
  if (spolIdIsSwapped(val.id)) {
    SWAP_array_32(&val.id, 8 * sizeof(si32));
    SWAP_array_64(&val.time_secs_utc, sizeof(si64));
    SWAP_array_32(&val.time_nano_secs, 4 * sizeof(si32));
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////
// get packet time as a double

double spol_get_time_as_double(const spol_short_angle_t &val)

{
  return (val.time_secs_utc + val.time_nano_secs / 1.0e9);
}

//////////////////////////////////////////////////////////////////
// string representation of packet_id 

string spol_id_to_str(int packet_id)

{
  
  switch (packet_id) {
    case SPOL_SHORT_ANGLE_ID: return "SPOL_SHORT_ANGLE_ID";
    default: return "UNKNOWN";
  }

}

//////////////////////////////////////////////////////////////////
// printing routines

void spol_print(FILE *out, const spol_short_angle_t &val)

{
  
  spol_short_angle_t copy(val);
  spol_swap(copy);

  fprintf(out, "  id: 0x%x (%d)\n", copy.id, copy.id);
  fprintf(out, "  len_bytes: %d\n", copy.len_bytes);
  fprintf(out, "  seq_num: %ud\n", copy.seq_num);

  time_t ptime = copy.time_secs_utc;
  fprintf(out, "  time UTC: %s.%.9d\n",
	  DateTime::strm(ptime).c_str(), copy.time_nano_secs);

  fprintf(out, "  elevation: %g\n", copy.elevation);
  fprintf(out, "  azimuth: %g\n", copy.azimuth);

}

