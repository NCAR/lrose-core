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
 * /file <chill_to_iwrf.hh>
 *
 * Utility routines for converting chill time series
 * data to iwrf time series format
 * Functions for handling moments and time series radar data.
 *
 * CSU-CHILL/NCAR
 * IWRF - INTEGRATED WEATHER RADAR FACILILTY
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * Feb 2009
 *
 *********************************************************************/

#ifndef _CHILL_TO_IWRF_
#define _CHILL_TO_IWRF_

#include <cstdio>
#include <string>
#include <radar/iwrf_data.h>
#include <radar/iwrf_functions.hh>
#include <radar/chill_types.h>
using namespace std;

// functions

// string representation of chill enums

extern string chill_scan_type_to_str(int scan_type);
extern string chill_follow_mode_to_str(int follow_mode);
extern string chill_pol_mode_to_str(int pol_mode);
extern string chill_pulse_type_to_str(int pulse_type);
extern string chill_test_type_to_str(int test_type);
extern string chill_event_notice_cause_to_str(int event_notice_cause);

// printing

extern void chill_generic_packet_header_print
  (ostream &out,
   const generic_packet_header_t &hdr);

extern void chill_radar_info_print(ostream &out,
                                   const radar_info_t &info);

extern void chill_scan_seg_print(ostream &out,
                                 const scan_seg_t &seg);

extern void chill_proc_info_print(ostream &out,
                                  const processor_info_t &proc);

extern void chill_power_update_print(ostream &out,
                                     const power_update_t &pup);

extern void chill_xmit_sample_print(ostream &out,
                                    const xmit_sample_t &xmit);

extern void chill_event_notice_print(ostream &out,
                                     const event_notice_t &event);

extern void chill_cal_terms_print(ostream &out,
                                  const cal_terms_t &cal);

extern void chill_phasecode_print(ostream &out,
                                  const phasecode_t &code);

extern void chill_xmit_info_print(ostream &out,
                                  const xmit_info_t &info);

extern void chill_ant_corr_print(ostream &out,
                                 const antenna_correction_t &corr);
  
extern void chill_sdb_version_print(ostream &out,
                                    const sdb_version_hdr_t &version);
  
//////////////////////////////////////////////
// load up IWRF structs from CHILL structs

extern void chill_iwrf_radar_info_load(const radar_info_t &info,
                                       si64 seq_num,
                                       iwrf_radar_info_t &iwrf);

extern void chill_iwrf_scan_seg_load(const scan_seg_t &info,
                                     si64 seq_num,
                                     iwrf_scan_segment_t &iwrf);

extern void chill_iwrf_ant_corr_load(const antenna_correction_t &corr,
                                     si64 seq_num,
                                     iwrf_antenna_correction_t &iwrf);

extern void chill_iwrf_ts_proc_load(const processor_info_t &proc,
                                    si64 seq_num,
                                    iwrf_ts_processing_t &iwrf);

extern void chill_iwrf_xmit_power_load(const power_update_t &pwr,
                                       si64 seq_num,
                                       iwrf_xmit_power_t &iwrf);

extern void chill_iwrf_xmit_sample_load(const xmit_sample_t &sample,
                                        si64 seq_num,
                                        iwrf_xmit_sample_t &iwrf);

extern void chill_iwrf_xmit_info_load(const xmit_info_t &info,
                                      si64 seq_num,
                                      iwrf_xmit_info_t &iwrf);

extern void chill_iwrf_calibration_load(const radar_info_t &rinfo,
                                        const cal_terms_t &cal,
                                        const power_update_t &pwr,
                                        si64 seq_num,
                                        iwrf_calibration_t &iwrf);

extern void chill_iwrf_event_notice_load(const event_notice_t &event,
                                         const scan_seg_t &scan,
                                         si64 seq_num,
                                         iwrf_event_notice_t &iwrf);

extern void chill_iwrf_phasecode_load(const phasecode_t &code,
                                      si64 seq_num,
                                      iwrf_phasecode_t &iwrf);


#endif
