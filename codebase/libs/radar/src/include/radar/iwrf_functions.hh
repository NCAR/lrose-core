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
 * /file <iwrf_functions.hh>
 *
 * Functions for handling moments and time series radar data.
 *
 * CSU-CHILL/NCAR
 * IWRF - INTEGRATED WEATHER RADAR FACILILTY
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * Feb 2009
 *
 *********************************************************************/

#ifndef _IWRF_FUNCTIONS_HH_
#define _IWRF_FUNCTIONS_HH_

#include <cstdio>
#include <string>
#include <radar/iwrf_data.h>
#include <radar/iwrf_user_interface.h>
using namespace std;

// functions

/////////////////////////////
// check for a missing values

extern bool iwrf_int_is_missing(si32 val);
extern bool iwrf_float_is_missing(fl32 val);

//////////////////////////////////////////////////////////////////
// packet initialization
// sets values to missing as appropriate

extern void iwrf_sync_init(iwrf_sync_t &val);
extern void iwrf_version_init(iwrf_version_t &val);
extern void iwrf_radar_info_init(iwrf_radar_info_t &val);
extern void iwrf_scan_segment_init(iwrf_scan_segment_t &val);
extern void iwrf_antenna_correction_init(iwrf_antenna_correction_t &val);
extern void iwrf_ts_processing_init(iwrf_ts_processing_t &val);
extern void iwrf_xmit_power_init(iwrf_xmit_power_t &val);
extern void iwrf_rx_power_init(iwrf_rx_power_t &val);
extern void iwrf_xmit_sample_init(iwrf_xmit_sample_t &val);
extern void iwrf_xmit_sample_v2_init(iwrf_xmit_sample_v2_t &val);
extern void iwrf_burst_header_init(iwrf_burst_header_t &val);
extern void iwrf_status_xml_init(iwrf_status_xml_t &val);
extern void iwrf_antenna_angles_init(iwrf_antenna_angles_t &val);
extern void iwrf_calibration_init(iwrf_calibration_t &val);
extern void iwrf_event_notice_init(iwrf_event_notice_t &val);
extern void iwrf_phasecode_init(iwrf_phasecode_t &val);
extern void iwrf_xmit_info_init(iwrf_xmit_info_t &val);
extern void iwrf_pulse_header_init(iwrf_pulse_header_t &val);
extern void iwrf_rvp8_ops_info_init(iwrf_rvp8_ops_info_t &val);
extern void iwrf_rvp8_pulse_header_init(iwrf_rvp8_pulse_header_t &val);
extern void iwrf_moments_field_header_init(iwrf_moments_field_header_t &val);
extern void iwrf_moments_ray_header_init(iwrf_moments_ray_header_t &val);
extern void iwrf_moments_field_index_init(iwrf_moments_field_index_t &val);
extern void iwrf_platform_georef_init(iwrf_platform_georef_t &val);
extern void iwrf_georef_correction_init(iwrf_georef_correction_t &val);

////////////////////////////////////////////////////////////
// set packet sequence number

extern void iwrf_set_packet_seq_num(iwrf_packet_info_t &packet, si64 seq_num);

////////////////////////////////////////////////////////////
// set packet time

extern void iwrf_set_packet_time(iwrf_packet_info_t &packet, double dtime);

extern void iwrf_set_packet_time(iwrf_packet_info_t &packet,
				 time_t secs, int nano_secs);

extern void iwrf_set_packet_time_to_now(iwrf_packet_info_t &packet);

//////////////////////////////////////////////////////////////////
// get packet id, check for validity of this packet
// checks the packet length
// returns 0 on success, -1 on failure

extern int iwrf_get_packet_id(const void* buf, int len, int &packet_id);

//////////////////////////////////////////////////////////////////
// check packet id for validity, swapping as required.
// if isSwapped is non-null, it is set true if swapping is active
// returns 0 on success, -1 on failure

extern int iwrf_check_packet_id(si32 packetId,
                                bool *isSwapped = NULL);

// check packet id for validity, swapping in-place as required.
// also swaps the packet_len argument.
// if isSwapped is non-null, it is set true if swapping is active
// returns 0 on success, -1 on failure

extern int iwrf_check_packet_id(si32 &packetId,
                                si32 &packetLen,
                                bool *isSwapped = NULL);

//////////////////////////////////////////////////////////////////
// get packet time as a double

extern double iwrf_get_packet_time_as_double(const iwrf_packet_info_t &packet);

// Check if packet buffer has correct radar id.
// Returns true if correct, false if not.
// A specified radarId of 0 will match all messages.

extern bool iwrf_check_radar_id(const void *buf, int len, int radarId);

//////////////////////////////////////////////////////////////////
// swapping routines
//
// swap to native as required
// swapping is the responsibility of the user, data is always
// written in native

// check if ID needs swapping

inline bool iwrfIdIsSwapped(si32 id) {
  if ((id & 0x0000ffff) == 0x00007777) {
    // is swapped
    return true;
  }
  return false;
}

// swap depending on packet type
// returns 0 on success, -1 on failure

extern int iwrf_packet_swap(void *buf, int len);

// swap packet info
// returns true is swapped, false if already in native

extern bool iwrf_packet_info_swap(iwrf_packet_info_t &packet);

// swap version
// returns true is swapped, false if already in native

extern bool iwrf_version_swap(iwrf_version_t &version);

// swap sync
// returns true is swapped, false if already in native

extern bool iwrf_sync_swap(iwrf_sync_t &sync);

// swap radar_info
// returns true is swapped, false if already in native

extern bool iwrf_radar_info_swap(iwrf_radar_info_t &radar_info);

// swap scan_segment
// returns true is swapped, false if already in native

extern bool iwrf_scan_segment_swap(iwrf_scan_segment_t &segment);

// swap antenna_correction
// returns true is swapped, false if already in native

extern bool iwrf_antenna_correction_swap(iwrf_antenna_correction_t &correction);

// swap ts_processing
// returns true is swapped, false if already in native

extern bool iwrf_ts_processing_swap(iwrf_ts_processing_t &processing);

// swap xmit_power
// returns true is swapped, false if already in native

extern bool iwrf_xmit_power_swap(iwrf_xmit_power_t &power);

// swap rx_power
// returns true is swapped, false if already in native

extern bool iwrf_rx_power_swap(iwrf_rx_power_t &power);

// swap xmit_sample
// returns true is swapped, false if already in native

extern bool iwrf_xmit_sample_swap(iwrf_xmit_sample_t &sample);
extern bool iwrf_xmit_sample_v2_swap(iwrf_xmit_sample_v2_t &sample);

// burst IQ sample
// returns true is swapped, false if already in native

extern bool iwrf_burst_header_swap(iwrf_burst_header_t &val);

// status as ASCII XML
// returns true is swapped, false if already in native

extern bool iwrf_status_xml_swap(iwrf_status_xml_t &val);

// antenn angles for monitoring
// returns true is swapped, false if already in native

extern bool iwrf_antenna_angles_swap(iwrf_antenna_angles_t &val);

// swap calibration
// returns true is swapped, false if already in native

extern bool iwrf_calibration_swap(iwrf_calibration_t &calib);

// swap event_notice
// returns true is swapped, false if already in native

extern bool iwrf_event_notice_swap(iwrf_event_notice_t &notice);

// swap phasecode
// returns true is swapped, false if already in native

extern bool iwrf_phasecode_swap(iwrf_phasecode_t &code);

// swap xmit_info
// returns true is swapped, false if already in native

extern bool iwrf_xmit_info_swap(iwrf_xmit_info_t &info);

// swap pulse_header
// returns true is swapped, false if already in native

extern bool iwrf_pulse_header_swap(iwrf_pulse_header_t &pulse);

// swap rvp8_pulse_header
// returns true is swapped, false if already in native

extern bool iwrf_rvp8_pulse_header_swap(iwrf_rvp8_pulse_header_t &pulse);

// swap rvp8_ops_info
// returns true is swapped, false if already in native

extern bool iwrf_rvp8_ops_info_swap(iwrf_rvp8_ops_info_t &info);

// swap moments_field_header
// returns true is swapped, false if already in native

extern bool iwrf_moments_field_header_swap(iwrf_moments_field_header_t &val);

// swap moments_ray_header
// returns true is swapped, false if already in native

extern bool iwrf_moments_ray_header_swap(iwrf_moments_ray_header_t &val);

// swap moments_field_index
// returns true is swapped, false if already in native

extern bool iwrf_moments_field_index_swap(iwrf_moments_field_index_t &val);

// swap platform_georef
// returns true is swapped, false if already in native

extern bool iwrf_platform_georef_swap(iwrf_platform_georef_t &val);

// swap georef_correction
// returns true is swapped, false if already in native

extern bool iwrf_georef_correction_swap(iwrf_georef_correction_t &val);

//////////////////////////////////////////////////////////////////
// string representation of enums

extern string iwrf_packet_id_to_str(int packet_id);
extern string iwrf_xmit_rcv_mode_to_str(int xmit_rcv_mode);
extern string iwrf_xmit_phase_mode_to_str(int xmit_phase_mode);
extern string iwrf_prf_mode_to_str(int prf_mode);
extern string iwrf_pulse_type_to_str(int pulse_type);
extern string iwrf_pol_mode_to_str(int pol_mode);
extern string iwrf_scan_mode_to_str(int scan_mode);
extern string iwrf_scan_mode_to_short_str(int scan_mode);
extern string iwrf_follow_mode_to_str(int follow_mode);
extern string iwrf_radar_platform_to_str(int radar_platform);
extern string iwrf_cal_type_to_str(int cal_type);
extern string iwrf_event_cause_to_str(int event_cause);
extern string iwrf_iq_encoding_to_str(int iq_encoding);
extern string iwrf_moments_encoding_to_str(int moments_encoding);
extern string iwrf_ui_error_to_str(int ui_error);
extern string iwrf_ui_opcode_to_str(int ui_opcode);
extern string iwrf_sample_units_to_str(int sample_units);

//////////////////////////////////////////////////////////////////
// printing routines

// print depending on packet type

extern void iwrf_packet_print
  (FILE *out, const void *buf, int len);

// print packet info

extern void iwrf_packet_info_print
  (FILE *out, const iwrf_packet_info_t &packet);

// print sync packet

extern void iwrf_sync_print
  (FILE *out, const iwrf_sync_t &sync);

// print radar_info

extern void iwrf_radar_info_print
  (FILE *out, const iwrf_radar_info_t &info);

// print scan_segment

extern void iwrf_scan_segment_print
  (FILE *out, const iwrf_scan_segment_t &seg);

// print antenna_correction

extern void iwrf_antenna_correction_print
  (FILE *out, const iwrf_antenna_correction_t &corr);

// print ts_processing

extern void iwrf_ts_processing_print
  (FILE *out, const iwrf_ts_processing_t &proc);

// print xmit_power

extern void iwrf_xmit_power_print
  (FILE *out, const iwrf_xmit_power_t &pwr);

// print rx_power

extern void iwrf_rx_power_print
  (FILE *out, const iwrf_rx_power_t &pwr);

// print xmit_sample

extern void iwrf_xmit_sample_print
  (FILE *out, const iwrf_xmit_sample_t &samp);

extern void iwrf_xmit_sample_v2_print
  (FILE *out, const iwrf_xmit_sample_v2_t &samp);

// print burst_iq

extern void iwrf_burst_header_print
  (FILE *out, const iwrf_burst_header_t &val);
  
// print status_xml

extern void iwrf_status_xml_print
  (FILE *out, const iwrf_status_xml_t &val,
   const string &xmlStr);

extern void iwrf_status_xml_print
  (FILE *out, const void *buf);
  
// print antenna_angles

extern void iwrf_antenna_angles_print
  (FILE *out, const iwrf_antenna_angles_t &val);
  
// print calibration

extern void iwrf_calibration_print
  (FILE *out, const iwrf_calibration_t &calib);
  
// print event_notice

extern void iwrf_event_notice_print
  (FILE *out, const iwrf_event_notice_t &note);

// print iwrf_phasecode

extern void iwrf_phasecode_print
  (FILE *out, const iwrf_phasecode_t &code);
  
// print xmit_info

extern void iwrf_xmit_info_print
  (FILE *out, const iwrf_xmit_info_t &info);

// print pulse_header

extern void iwrf_pulse_header_print
  (FILE *out, const iwrf_pulse_header_t &pulse,
   const iwrf_platform_georef_t *georef = NULL);

// print rvp8_pulse_header

extern void iwrf_rvp8_pulse_header_print
  (FILE *out, const iwrf_rvp8_pulse_header_t &pulse);

// print rvp8_ops_info

extern void iwrf_rvp8_ops_info_print
  (FILE *out, const iwrf_rvp8_ops_info_t &info);

// print moments_field_header

extern void iwrf_moments_field_header_print
  (FILE *out, const iwrf_moments_field_header_t &info);

// print moments_ray_header

extern void iwrf_moments_ray_header_print
  (FILE *out, const iwrf_moments_ray_header_t &info);

// print moments_field_index

extern void iwrf_moments_field_index_print
  (FILE *out, const iwrf_moments_field_index_t &info);

// print platform_georef

extern void iwrf_platform_georef_print
  (FILE *out, const iwrf_platform_georef_t &info);

// print georef_correction

extern void iwrf_georef_correction_print
  (FILE *out, const iwrf_georef_correction_t &info);

// print user interface task-list

extern void iwrf_ui_tasklist_print
  (FILE *out, const iwrf_ui_tasklist_full_t &tlp);

// print UI schedule header

extern void iwrf_ui_schedule_print
  (FILE *out, const iwrf_ui_schedule_info_t &schedule);

// print user interface task operations

extern void iwrf_ui_task_operations_print
  (FILE *out, const iwrf_ui_task_operations_t &val);

//////////////////////////////////////////////////////////////////
// printing the format of each struct

extern void iwrf_print_all_formats(FILE *out);

// print format of packet info

extern void iwrf_packet_info_print_format
  (FILE *out, const iwrf_packet_info_t &val);

// print format of sync packet

extern void iwrf_sync_print_format
  (FILE *out, const iwrf_sync_t &val);

// print format of radar_info

extern void iwrf_radar_info_print_format
  (FILE *out, const iwrf_radar_info_t &val);

// print format of scan_segment

extern void iwrf_scan_segment_print_format
  (FILE *out, const iwrf_scan_segment_t &val);

// print format of antenna_correction

extern void iwrf_antenna_correction_print_format
  (FILE *out, const iwrf_antenna_correction_t &val);

// print format of ts_processing

extern void iwrf_ts_processing_print_format
  (FILE *out, const iwrf_ts_processing_t &val);

// print format of xmit_power

extern void iwrf_xmit_power_print_format
  (FILE *out, const iwrf_xmit_power_t &val);

// print format of rx_power

extern void iwrf_rx_power_print_format
  (FILE *out, const iwrf_rx_power_t &val);

// print format of xmit_sample

extern void iwrf_xmit_sample_print_format
  (FILE *out, const iwrf_xmit_sample_t &val);

extern void iwrf_xmit_sample_v2_print_format
  (FILE *out, const iwrf_xmit_sample_v2_t &val);

// print format of burst_iq

extern void iwrf_burst_header_print_format
  (FILE *out, const iwrf_burst_header_t &val);
  
// print format of status_xml

extern void iwrf_status_xml_print_format
  (FILE *out, const iwrf_status_xml_t &val);
  
// print format of antenna_angles

extern void iwrf_antenna_angles_print_format
  (FILE *out, const iwrf_antenna_angles_t &val);
  
// print format of calibration

extern void iwrf_calibration_print_format
  (FILE *out, const iwrf_calibration_t &val);
  
// print format of event_notice

extern void iwrf_event_notice_print_format
  (FILE *out, const iwrf_event_notice_t &val);

// print format of iwrf_phasecode

extern void iwrf_phasecode_print_format
  (FILE *out, const iwrf_phasecode_t &val);
  
// print format of xmit_info

extern void iwrf_xmit_info_print_format
  (FILE *out, const iwrf_xmit_info_t &val);

// print format of pulse_header

extern void iwrf_pulse_header_print_format
  (FILE *out, const iwrf_pulse_header_t &val);

// print format of rvp8_pulse_header

extern void iwrf_rvp8_pulse_header_print_format
  (FILE *out, const iwrf_rvp8_pulse_header_t &val);

// print format of rvp8_ops_info

extern void iwrf_rvp8_ops_info_print_format
  (FILE *out, const iwrf_rvp8_ops_info_t &val);

// print format of moments_field_header

extern void iwrf_moments_field_header_print_format
  (FILE *out, const iwrf_moments_field_header_t &val);

// print format of moments_ray_header

extern void iwrf_moments_ray_header_print_format
  (FILE *out, const iwrf_moments_ray_header_t &val);

// print format of moments_field_index

extern void iwrf_moments_field_index_print_format
  (FILE *out, const iwrf_moments_field_index_t &val);

// print format of platform_georef

extern void iwrf_platform_georef_print_format
  (FILE *out, const iwrf_platform_georef_t &val);

// print format of georef_correction

extern void iwrf_georef_correction_print_format
  (FILE *out, const iwrf_georef_correction_t &val);

// print format of user interface task-list

extern void iwrf_ui_task_operations_print_format
  (FILE *out, const iwrf_ui_task_operations_t &val);

///////////////////////////////////////////////////////
// Safe string for character data.
// Returns a string formed safely from a char* array.
// Null-termination of the input string is guaranteed.

extern string iwrf_safe_str(const char *str, int maxLen);

///////////////////////////////////////////////////////
// convert to time string

extern string iwrf_time_str(const time_t *ptime, si32 *nano_secs);

// macro for comparing logical equality between iwrf structures
// this returns the result of comparing the body of the
// structs using memcmp, i.e. 0 on equality

#define iwrf_compare(a, b) \
  memcmp((char *) &(a) + sizeof(iwrf_packet_info_t), \
	 (char *) &(b) + sizeof(iwrf_packet_info_t), \
	 sizeof(a) - sizeof(iwrf_packet_info_t))

#endif
