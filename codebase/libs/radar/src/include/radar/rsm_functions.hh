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
 * /file <rsm_functions.hh>
 *
 * Functions for handling rsm data.
 *
 * CSU-CHILL/NCAR
 * IWRF - INTEGRATED WEATHER RADAR FACILILTY
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * July 2010
 *
 *********************************************************************/

#ifndef _RSM_FUNCTIONS_HH_
#define _RSM_FUNCTIONS_HH_

#include <cstdio>
#include <string>
#include <radar/iwrf_rsm.h>
using namespace std;

// string representation of enums

extern string rsm_msgtype_to_str(int msgtype);
extern string rsm_acq_id_to_str(int acq_id);
extern string rsm_acq_status_to_str(int acq_status);
extern string rsm_acq_smptype_to_str(int acq_smptype);
extern string rsm_scan_status_to_str(int scan_status);
extern string rsm_scon_begin_cause_to_str(int scon_begin_cause);
extern string rsm_syscon_id_to_str(int syscon_id);
extern string rsm_syscon_state_to_str(int syscon_state);
extern string rsm_txctrl_err_to_str(int txctrl_err);
extern string rsm_event_cause_to_str(int event_cause);
extern string rsm_ant_state_to_str(int ant_state);
extern string rsm_scan_event_to_str(int scan_event);
extern string rsm_ant_id_to_str(int ant_id);
extern string rsm_archiver_id_to_str(int archiver_id);
extern string rsm_archiver_status_to_str(int archiver_status);
extern string rsm_ins_svr_status_to_str(int ins_svr_status);
extern string rsm_ins_pm_status_to_str(int ins_pm_status);
extern string rsm_ins_ts_status_to_str(int ins_ts_status);
extern string rsm_ins_ts_mode_to_str(int ins_ts_mode);
extern string rsm_ins_fc_status_to_str(int ins_fc_status);
extern string rsm_ins_fc_inputmode_to_str(int ins_fc_inputmode);
extern string rsm_ins_fc_fmrate_to_str(int ins_fc_fmrate);
extern string rsm_ins_stalo_mode_to_str(int ins_stalo_mode);
extern string rsm_mserv_id_to_str(int mserv_id);
extern string rsm_xmit_status_to_str(int xmit_status);
extern string rsm_xmit_polmode_to_str(int xmit_polmode);
extern string rsm_dual_xmit_sel_to_str(int dual_xmit_sel);
extern string rsm_tsarch_id_to_str(int tsarch_id);
extern string rsm_tsarch_mode_to_str(int tsarch_mode);
extern string rsm_tsarch_state_to_str(int tsarch_state);
extern string rsm_misc_status_to_str(int misc_status);
extern string rsm_syscon_err_to_str(int err);
extern string rsm_antcon_err_to_str(int err);
extern string rsm_antcon_servo_to_str(int servo);

  
// print rsm msg header

extern void rsm_msghdr_print
  (FILE *out, const rsm_msghdr_t &val);

// print rsm syscon state

extern void rsm_syscon_print(FILE *out, const rsm_syscon_t &val);

// print rsm antcon state

extern void rsm_antcon_print(FILE *out, const rsm_antcon_t &val);

// print rsm instrument power meter data

extern void rsm_ins_pm_print
  (FILE *out, const rsm_ins_pm_t &val);

#endif
