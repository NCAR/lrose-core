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
/******************************************************************
 * swap_nexrad.c
 *
 * Byte swapping routines for NEXRAD structs
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * May 1997
 *
 */

#include "nexrad_port.h"
#include <dataport/bigend.h>
using namespace std;

void be_to_NEXRAD_vol_number(NEXRAD_vol_number *num)
{
  BE_to_array_16(&num->vol_number, 2);
}

void be_from_NEXRAD_vol_number(NEXRAD_vol_number *num)
{
  BE_from_array_16(&num->vol_number, 2);
}

void be_to_NEXRAD_vol_title(NEXRAD_vol_title *title)
{
  BE_to_array_32(&title->julian_date, 4);
  BE_to_array_32(&title->millisecs_past_midnight, 4);
  BE_to_array_32(&title->filler1, 4);
}

void be_from_NEXRAD_vol_title(NEXRAD_vol_title *title)
{
  BE_from_array_32(&title->julian_date, 4);
  BE_from_array_32(&title->millisecs_past_midnight, 4);
  BE_from_array_32(&title->filler1, 4);
}

void be_to_NEXRAD_msg_hdr(NEXRAD_msg_hdr *hdr)
{
  BE_to_array_16(&hdr->message_len, 2);
  BE_to_array_16(&hdr->seq_num, 2);
  BE_to_array_16(&hdr->julian_date, 2);
  BE_to_array_32(&hdr->millisecs_past_midnight, 4);
  BE_to_array_16(&hdr->num_message_segs, 2);
  BE_to_array_16(&hdr->message_seg_num, 2);
}

void be_from_NEXRAD_msg_hdr(NEXRAD_msg_hdr *hdr)
{
  BE_from_array_16(&hdr->message_len, 2);
  BE_from_array_16(&hdr->seq_num, 2);
  BE_from_array_16(&hdr->julian_date, 2);
  BE_from_array_32(&hdr->millisecs_past_midnight, 4);
  BE_from_array_16(&hdr->num_message_segs, 2);
  BE_from_array_16(&hdr->message_seg_num, 2);
}

void be_to_NEXRAD_data_hdr(NEXRAD_data_hdr *data)
{
  BE_to_array_32(&data->millisecs_past_midnight, 4);
  BE_to_array_32(&data->sys_gain_cal_const, 4);
  BE_to_array_16(&data->julian_date, NEXRAD_NSI16S_AFTER_JULIAN_DATE * 2);
  BE_to_array_16(&data->ref_ptr, NEXRAD_NSI16S_AFTER_REF_PTR * 2);
}

void be_from_NEXRAD_data_hdr(NEXRAD_data_hdr *data)
{
  BE_from_array_32(&data->millisecs_past_midnight, 4);
  BE_from_array_32(&data->sys_gain_cal_const, 4);
  BE_from_array_16(&data->julian_date, NEXRAD_NSI16S_AFTER_JULIAN_DATE * 2);
  BE_from_array_16(&data->ref_ptr, NEXRAD_NSI16S_AFTER_REF_PTR * 2);
}

void be_to_NEXRAD_hdr(NEXRAD_hdr *hdr)
{
  BE_to_array_32(hdr, sizeof(NEXRAD_hdr));
}

void be_from_NEXRAD_hdr(NEXRAD_hdr *hdr)
{
  BE_from_array_32(hdr, sizeof(NEXRAD_hdr));
}

