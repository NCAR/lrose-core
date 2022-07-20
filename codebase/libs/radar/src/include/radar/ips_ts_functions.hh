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
/******************************************************************/
/**
 *
 * /file <ips_ts_functions.hh>
 *
 * Support for Independent Pulse Sampling.
 * Functions for handling time series radar data.
 *
 * NCAR/EOL
 *
 * Mike Dixon, EOL, NCAR, Boulder, CO
 * Aug 2019
 *
 *********************************************************************/

#ifndef _IPS_TS_FUNCTIONS_HH_
#define _IPS_TS_FUNCTIONS_HH_

#include <cstdio>
#include <string>
#include <cmath>
#include <radar/ips_ts_data.h>
using namespace std;

// functions

/////////////////////////////
// check for a missing values

extern bool ips_ts_int_is_missing(si32 val);
extern bool ips_ts_float_is_missing(fl32 val);

//////////////////////////////////////////////////////////////////
// packet initialization
// sets values to missing as appropriate

extern void ips_ts_packet_info_init(ips_ts_packet_info_t &val);
extern void ips_ts_sync_init(ips_ts_sync_t &val);
extern void ips_ts_version_init(ips_ts_version_t &val);
extern void ips_ts_radar_info_init(ips_ts_radar_info_t &val);
extern void ips_ts_scan_segment_init(ips_ts_scan_segment_t &val);
extern void ips_ts_processing_init(ips_ts_processing_t &val);
extern void ips_ts_status_xml_init(ips_ts_status_xml_t &val);
extern void ips_ts_calibration_init(ips_ts_calibration_t &val);
extern void ips_ts_event_notice_init(ips_ts_event_notice_t &val);
extern void ips_ts_pulse_header_init(ips_ts_pulse_header_t &val);
extern void ips_ts_platform_georef_init(ips_ts_platform_georef_t &val);
extern void ips_ts_georef_correction_init(ips_ts_georef_correction_t &val);

////////////////////////////////////////////////////////////
// set packet sequence number

extern void ips_ts_set_packet_seq_num(ips_ts_packet_info_t &packet, si64 seq_num);

////////////////////////////////////////////////////////////
// set packet time

extern void ips_ts_set_packet_time(ips_ts_packet_info_t &packet, double dtime);

extern void ips_ts_set_packet_time(ips_ts_packet_info_t &packet,
				 time_t secs, int nano_secs);

extern void ips_ts_set_packet_time_to_now(ips_ts_packet_info_t &packet);

//////////////////////////////////////////////////////////////////
// get packet id, check for validity of this packet
// checks the packet length
// returns 0 on success, -1 on failure

extern int ips_ts_get_packet_id(const void* buf, int len, int &packet_id);

//////////////////////////////////////////////////////////////////
// check packet id for validity, swapping as required.
// if isSwapped is non-null, it is set true if swapping is active
// returns 0 on success, -1 on failure

extern int ips_ts_check_packet_id(si32 packetId,
                                   bool *isSwapped = NULL);

// check packet id for validity, swapping in-place as required.
// also swaps the packet_len argument.
// if isSwapped is non-null, it is set true if swapping is active
// returns 0 on success, -1 on failure

extern int ips_ts_check_packet_id(si32 &packetId,
                                   si32 &packetLen,
                                   bool *isSwapped = NULL);

//////////////////////////////////////////////////////////////////
// get packet time as a double

extern double ips_ts_get_packet_time_as_double(const ips_ts_packet_info_t &packet);

// Check if packet buffer has correct radar id.
// Returns true if correct, false if not.
// A specified radarId of 0 will match all messages.

extern bool ips_ts_check_radar_id(const void *buf, int len, int radarId);

//////////////////////////////////////////////////////////////////
// swapping routines
//
// swap to native as required
// swapping is the responsibility of the user, data is always
// written in native

// check if ID needs swapping

inline bool ipsIdIsSwapped(si32 id) {
  if ((id & 0x0000ffff) == 0x00005555) {
    // is swapped
    return true;
  }
  return false;
}

// swap depending on packet type
// returns 0 on success, -1 on failure

extern int ips_ts_packet_swap(void *buf, int len);

// swap packet info
// returns true is swapped, false if already in native

extern bool ips_ts_packet_info_swap(ips_ts_packet_info_t &packet);

// swap version
// returns true is swapped, false if already in native

extern bool ips_ts_version_swap(ips_ts_version_t &version);

// swap sync
// returns true is swapped, false if already in native

extern bool ips_ts_sync_swap(ips_ts_sync_t &sync);

// swap radar_info
// returns true is swapped, false if already in native

extern bool ips_ts_radar_info_swap(ips_ts_radar_info_t &radar_info);

// swap scan_segment
// returns true is swapped, false if already in native

extern bool ips_ts_scan_segment_swap(ips_ts_scan_segment_t &segment);

// swap ts_processing
// returns true is swapped, false if already in native

extern bool ips_ts_processing_swap(ips_ts_processing_t &processing);

// status as ASCII XML
// returns true is swapped, false if already in native

extern bool ips_ts_status_xml_swap(ips_ts_status_xml_t &val);

// swap calibration
// returns true is swapped, false if already in native

extern bool ips_ts_calibration_swap(ips_ts_calibration_t &calib);

// swap event_notice
// returns true is swapped, false if already in native

extern bool ips_ts_event_notice_swap(ips_ts_event_notice_t &notice);

// swap pulse_header
// returns true is swapped, false if already in native

extern bool ips_ts_pulse_header_swap(ips_ts_pulse_header_t &pulse);

// swap platform_georef
// returns true is swapped, false if already in native

extern bool ips_ts_platform_georef_swap(ips_ts_platform_georef_t &val);

// swap georef_correction
// returns true is swapped, false if already in native

extern bool ips_ts_georef_correction_swap(ips_ts_georef_correction_t &val);

//////////////////////////////////////////////////////////////////
// string representation of enums

extern string ips_ts_packet_id_to_str(int packet_id);

extern string ips_ts_prf_mode_to_str(ips_ts_prf_mode_t prf_mode);
extern string ips_ts_prf_mode_to_str(int prf_mode_int);
extern string ips_ts_pulse_shape_to_str(ips_ts_pulse_shape_t pulse_shape);
extern string ips_ts_pulse_shape_to_str(int pulse_shape_int);
extern string ips_ts_pol_mode_to_str(ips_ts_pol_mode_t pol_mode);
extern string ips_ts_pol_mode_to_str(int pol_mode_int);
extern string ips_ts_scan_mode_to_str(ips_ts_scan_mode_t scan_mode);
extern string ips_ts_scan_mode_to_str(int scan_mode_int);
extern string ips_ts_scan_mode_to_short_str(ips_ts_scan_mode_t scan_mode);
extern string ips_ts_scan_mode_to_short_str(int scan_mode_int);
extern string ips_ts_radar_platform_to_str(ips_ts_radar_platform_t radar_platform);
extern string ips_ts_iq_encoding_to_str(ips_ts_iq_encoding_t iq_encoding);
extern string ips_ts_iq_encoding_to_str(int iq_encoding_int);

//////////////////////////////////////////////////////////////////
// printing routines

// print depending on packet type

extern void ips_ts_packet_print
  (FILE *out, const void *buf, int len);

// print packet info

extern void ips_ts_packet_info_print
  (FILE *out, const ips_ts_packet_info_t &packet);

// print sync packet

extern void ips_ts_sync_print
  (FILE *out, const ips_ts_sync_t &sync);

// print radar_info

extern void ips_ts_radar_info_print
  (FILE *out, const ips_ts_radar_info_t &info);

// print scan_segment

extern void ips_ts_scan_segment_print
  (FILE *out, const ips_ts_scan_segment_t &seg);

// print ts_processing

extern void ips_ts_processing_print
  (FILE *out, const ips_ts_processing_t &proc);

// print status_xml

extern void ips_ts_status_xml_print
  (FILE *out, const ips_ts_status_xml_t &val,
   const string &xmlStr);

extern void ips_ts_status_xml_print
  (FILE *out, const void *buf);
  
// print calibration

extern void ips_ts_calibration_print
  (FILE *out, const ips_ts_calibration_t &calib);
  
// print event_notice

extern void ips_ts_event_notice_print
  (FILE *out, const ips_ts_event_notice_t &note);

// print pulse_header

extern void ips_ts_pulse_header_print
  (FILE *out, const ips_ts_pulse_header_t &pulse,
   const ips_ts_platform_georef_t *georef = NULL);

// print platform_georef

extern void ips_ts_platform_georef_print
  (FILE *out, const ips_ts_platform_georef_t &info);

// print georef_correction

extern void ips_ts_georef_correction_print
  (FILE *out, const ips_ts_georef_correction_t &info);

//////////////////////////////////////////////////////////////////
// printing the format of each struct

extern void ips_ts_print_all_formats(FILE *out);

// print format of packet info

extern void ips_ts_packet_info_print_format
  (FILE *out, const ips_ts_packet_info_t &val);

// print format of sync packet

extern void ips_ts_sync_print_format
  (FILE *out, const ips_ts_sync_t &val);

// print format of version packet

extern void ips_ts_version_print_format
  (FILE *out, const ips_ts_version_t &val);

// print format of radar_info

extern void ips_ts_radar_info_print_format
  (FILE *out, const ips_ts_radar_info_t &val);

// print format of scan_segment

extern void ips_ts_scan_segment_print_format
  (FILE *out, const ips_ts_scan_segment_t &val);

// print format of ts_processing

extern void ips_ts_processing_print_format
  (FILE *out, const ips_ts_processing_t &val);

// print format of status_xml

extern void ips_ts_status_xml_print_format
  (FILE *out, const ips_ts_status_xml_t &val);
  
// print format of calibration

extern void ips_ts_calibration_print_format
  (FILE *out, const ips_ts_calibration_t &val);
  
// print format of event_notice

extern void ips_ts_event_notice_print_format
  (FILE *out, const ips_ts_event_notice_t &val);

// print format of pulse_header

extern void ips_ts_pulse_header_print_format
  (FILE *out, const ips_ts_pulse_header_t &val);

// print format of platform_georef

extern void ips_ts_platform_georef_print_format
  (FILE *out, const ips_ts_platform_georef_t &val);

// print format of georef_correction

extern void ips_ts_georef_correction_print_format
  (FILE *out, const ips_ts_georef_correction_t &val);

///////////////////////////////////////////////////////
// Safe string for character data.
// Returns a string formed safely from a char* array.
// Null-termination of the input string is guaranteed.

extern string ips_ts_safe_str(const char *str, int maxLen);

///////////////////////////////////////////////////////
// convert to time string

extern string ips_ts_time_str(const time_t *ptime, si32 *nano_secs);

// macro for comparing logical equality between ips structures
// this returns the result of comparing the body of the
// structs using memcmp, i.e. 0 on equality

#define ips_ts_compare(a, b) \
  memcmp((char *) &(a) + sizeof(ips_ts_packet_info_t), \
	 (char *) &(b) + sizeof(ips_ts_packet_info_t), \
	 sizeof(a) - sizeof(ips_ts_packet_info_t))

#endif
