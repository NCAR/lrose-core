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
// rsm_functions.cc
//
// Utility routines for iwrf_rsm structs
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// July 2010

#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <radar/rsm_functions.hh>
#include <radar/iwrf_functions.hh>

using namespace std;

// string representation of rsm_msgtype

string rsm_msgtype_to_str(int msgtype)

{
  
  switch (msgtype) {
    case RSM_MSG_QUERY: return "RSM_MSG_QUERY";
    case RSM_MSG_STATUS: return "RSM_MSG_STATUS";
    case RSM_MSG_WARNING: return "RSM_MSG_WARNING";
    case RSM_MSG_ERROR: return "RSM_MSG_ERROR";
    case RSM_MSG_PRESET: return "RSM_MSG_PRESET";
    case RSM_MSG_KEEPALIVE: return "RSM_MSG_KEEPALIVE";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_acq_id

string rsm_acq_id_to_str(int acq_id)

{
  
  switch (acq_id) {
    case RSM_ACQ_ID_STATUS: return "RSM_ACQ_ID_STATUS";
    case RSM_ACQ_ID_CONN_NOTICE: return "RSM_ACQ_ID_CONN_NOTICE";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_acq_status

string rsm_acq_status_to_str(int acq_status)

{
  
  switch (acq_status) {
    case RSM_ACQ_IDLE: return "RSM_ACQ_IDLE";
    case RSM_ACQ_NOSIGNAL: return "RSM_ACQ_NOSIGNAL";
    case RSM_ACQ_ACQUIRING: return "RSM_ACQ_ACQUIRING";
    case RSM_ACQ_INIT: return "RSM_ACQ_INIT";
    case RSM_ACQ_ERROR: return "RSM_ACQ_ERROR";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_acq_smptype

string rsm_acq_smptype_to_str(int acq_smptype)

{
  
  switch (acq_smptype) {
    case RSM_ACQ_SMPTYPE_VCH: return "RSM_ACQ_SMPTYPE_VCH";
    case RSM_ACQ_SMPTYPE_HCH: return "RSM_ACQ_SMPTYPE_HCH";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_scan_status

string rsm_scan_status_to_str(int scan_status)

{
  
  switch (scan_status) {
    case RSM_SCON_SSTATE_SYSCON_INIT: return "RSM_SCON_SSTATE_SYSCON_INIT";
    case RSM_SCON_SSTATE_IDLE: return "RSM_SCON_SSTATE_IDLE";
    case RSM_SCON_SSTATE_PREP_WAIT: return "RSM_SCON_SSTATE_PREP_WAIT";
    case RSM_SCON_SSTATE_IN_SCAN: return "RSM_SCON_SSTATE_IN_SCAN";
    case RSM_SCON_SSTATE_SCAN_ENDING: return "RSM_SCON_SSTATE_SCAN_ENDING";
    case RSM_SCON_SSTATE_SCAN_TIMEOUT: return "RSM_SCON_SSTATE_SCAN_TIMEOUT";
    case RSM_SCON_SSTATE_ABORTING: return "RSM_SCON_SSTATE_ABORTING";
    case RSM_SCON_SSTATE_TIMER_ABORTING: return "RSM_SCON_SSTATE_TIMER_ABORTING";
    case RSM_SCON_SSTATE_ABORTING_ON_ERROR: return "RSM_SCON_SSTATE_ABORTING_ON_ERROR";
    case RSM_SCON_SSTATE_RESTARTING_ON_ERROR: return "RSM_SCON_SSTATE_RESTARTING_ON_ERROR";
    case RSM_SCON_SSTATE_CONTROLLER_RECONNECT: return "RSM_SCON_SSTATE_CONTROLLER_RECONNECT";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_scon_begin_cause

string rsm_scon_begin_cause_to_str(int scon_begin_cause)

{
  
  switch (scon_begin_cause) {
    case RSM_SCON_BC_OPERATOR_BEGIN: return "RSM_SCON_BC_OPERATOR_BEGIN";
    case RSM_SCON_BC_TIMER_BEGIN: return "RSM_SCON_BC_TIMER_BEGIN";
    case RSM_SCON_BC_NEXTSEG_LINK: return "RSM_SCON_BC_NEXTSEG_LINK";
    case RSM_SCON_BC_RESTART: return "RSM_SCON_BC_RESTART";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_syscon_id

string rsm_syscon_id_to_str(int syscon_id)

{
  
  switch (syscon_id) {
    case RSM_SYSCON_ID_STATE: return "RSM_SYSCON_ID_STATE";
    case RSM_SYSCON_ID_SCAN: return "RSM_SYSCON_ID_SCAN";
    case RSM_SYSCON_ID_VERSION: return "RSM_SYSCON_ID_VERSION";
    case RSM_SYSCON_ID_STATUS: return "RSM_SYSCON_ID_STATUS";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_syscon_state

string rsm_syscon_state_to_str(int syscon_state)

{
  
  switch (syscon_state) {
    case RSM_SYSCON_NO_STATE: return "RSM_SYSCON_NO_STATE";
    case RSM_SYSCON_INITIALIZING: return "RSM_SYSCON_INITIALIZING";
    case RSM_SYSCON_IDLE: return "RSM_SYSCON_IDLE";
    case RSM_SYSCON_BEGIN_SCAN_SENT: return "RSM_SYSCON_BEGIN_SCAN_SENT";
    case RSM_SYSCON_IN_SCAN: return "RSM_SYSCON_IN_SCAN";
    case RSM_SYSCON_BETWEEN_SCANS: return "RSM_SYSCON_BETWEEN_SCANS";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_txctrl_err

string rsm_txctrl_err_to_str(int txctrl_err)

{
  
  switch (txctrl_err) {
    case RSM_TXCTRL_ERR_NO_ERROR: return "RSM_TXCTRL_ERR_NO_ERROR";
    case RSM_TXCTRL_ERR_CONNECT: return "RSM_TXCTRL_ERR_CONNECT";
    case RSM_TXCTRL_ERR_SET_POL: return "RSM_TXCTRL_ERR_SET_POL";
    case RSM_TXCTRL_ERR_SET_INT_CYCLE: return "RSM_TXCTRL_ERR_SET_INT_CYCLE";
    case RSM_TXCTRL_ERR_SET_PRT1: return "RSM_TXCTRL_ERR_SET_PRT1";
    case RSM_TXCTRL_ERR_SET_PRT2: return "RSM_TXCTRL_ERR_SET_PRT2";
    case RSM_TXCTRL_ERR_SET_DUAL_PRT: return "RSM_TXCTRL_ERR_SET_DUAL_PRT";
    case RSM_TXCTRL_ERR_SET_TEST_PULSE: return "RSM_TXCTRL_ERR_SET_TEST_PULSE";
    case RSM_TXCTRL_ERR_SET_PHASE_SEQUENCE: return "RSM_TXCTRL_ERR_SET_PHASE_SEQUENCE";
    case RSM_TXCTRL_ERR_SET_WAVEFORM: return "RSM_TXCTRL_ERR_SET_WAVEFORM";
    case RSM_TXCTRL_ERR_SET_COMMIT: return "RSM_TXCTRL_ERR_SET_COMMIT";
    case RSM_TXCTRL_ERR_TX_ENABLE: return "RSM_TXCTRL_ERR_TX_ENABLE";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_event_cause

string rsm_event_cause_to_str(int event_cause)

{
  
  switch (event_cause) {
    case RSM_EVENT_CAUSE_NOT_SET: return "RSM_EVENT_CAUSE_NOT_SET";
    case RSM_EVENT_CAUSE_DONE: return "RSM_EVENT_CAUSE_DONE";
    case RSM_EVENT_CAUSE_TIMEOUT: return "RSM_EVENT_CAUSE_TIMEOUT";
    case RSM_EVENT_CAUSE_TIMER: return "RSM_EVENT_CAUSE_TIMER";
    case RSM_EVENT_CAUSE_ABORT: return "RSM_EVENT_CAUSE_ABORT";
    case RSM_EVENT_CAUSE_SCAN_ABORT: return "RSM_EVENT_CAUSE_SCAN_ABORT";
    case RSM_EVENT_CAUSE_RESTART: return "RSM_EVENT_CAUSE_RESTART";
    case RSM_EVENT_CAUSE_SCAN_STATE_TIMEOUT: return "RSM_EVENT_CAUSE_SCAN_STATE_TIMEOUT";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ant_state

string rsm_ant_state_to_str(int ant_state)

{
  
  switch (ant_state) {
    case RSM_ANT_NO_STATE: return "RSM_ANT_NO_STATE";
    case RSM_ANT_FAULTED: return "RSM_ANT_FAULTED";
    case RSM_ANT_IDLE: return "RSM_ANT_IDLE";
    case RSM_ANT_SCAN_DONE: return "RSM_ANT_SCAN_DONE";
    case RSM_ANT_HOLD_INIT: return "RSM_ANT_HOLD_INIT";
    case RSM_ANT_HOLD: return "RSM_ANT_HOLD";
    case RSM_ANT_PRE_POSITION: return "RSM_ANT_PRE_POSITION";
    case RSM_ANT_SECTOR_INIT: return "RSM_ANT_SECTOR_INIT";
    case RSM_ANT_IN_SECTOR: return "RSM_ANT_IN_SECTOR";
    case RSM_ANT_PPI_AZ_POSITIONING: return "RSM_ANT_PPI_AZ_POSITIONING";
    case RSM_ANT_PPI_EL_POSITIONING: return "RSM_ANT_PPI_EL_POSITIONING";
    case RSM_ANT_PPI_ACCEL: return "RSM_ANT_PPI_ACCEL";
    case RSM_ANT_PPI_IN_SWEEP: return "RSM_ANT_PPI_IN_SWEEP";
    case RSM_ANT_PROG4_DO_SECTOR: return "RSM_ANT_PROG4_DO_SECTOR";
    case RSM_ANT_PROG4_DO_SURV: return "RSM_ANT_PROG4_DO_SURV";
    case RSM_ANT_PROG4_DO_RHI: return "RSM_ANT_PROG4_DO_RHI";
    case RSM_ANT_DISCONNECTED: return "RSM_ANT_DISCONNECTED";
    case RSM_ANT_PROG4_FAIL: return "RSM_ANT_PROG4_FAIL";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_scan_event

string rsm_scan_event_to_str(int scan_event)

{
  
  switch (scan_event) {
    case RSM_ANT_EVENT_NOT_SET: return "RSM_ANT_EVENT_NOT_SET";
    case RSM_ANT_EVENT_AZ_IN_POSITION: return "RSM_ANT_EVENT_AZ_IN_POSITION";
    case RSM_ANT_EVENT_EL_IN_POSITION: return "RSM_ANT_EVENT_EL_IN_POSITION";
    case RSM_ANT_EVENT_ANTENNA_IN_POSITION: return "RSM_ANT_EVENT_ANTENNA_IN_POSITION";
    case RSM_ANT_EVENT_BEGIN_SWEEP: return "RSM_ANT_EVENT_BEGIN_SWEEP";
    case RSM_ANT_EVENT_END_SWEEP: return "RSM_ANT_EVENT_END_SWEEP";
    case RSM_ANT_EVENT_ERROR_SIGNAL: return "RSM_ANT_EVENT_ERROR_SIGNAL";
    case RSM_ANT_EVENT_ELEVATION_RANGE_ERROR: return "RSM_ANT_EVENT_ELEVATION_RANGE_ERROR";
    case RSM_ANT_EVENT_START_COMMAND: return "RSM_ANT_EVENT_START_COMMAND";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ant_id

string rsm_ant_id_to_str(int ant_id)

{
  
  switch (ant_id) {
    case RSM_ANTCON_ID_AZELONLY: return "RSM_ANTCON_ID_AZELONLY";
    case RSM_ANTCON_ID_STATE: return "RSM_ANTCON_ID_STATE";
    case RSM_ANTCON_ID_SERVO: return "RSM_ANTCON_ID_SERVO";
    case RSM_ANTCON_ID_MOTION: return "RSM_ANTCON_ID_MOTION";
    case RSM_ANTCON_ID_ERROR: return "RSM_ANTCON_ID_ERROR";
    case RSM_ANTCON_ID_MOTION_UPDATE_RATE: return "RSM_ANTCON_ID_MOTION_UPDATE_RATE";
    case RSM_ANTCON_ID_VERSION: return "RSM_ANTCON_ID_VERSION";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_archiver_id

string rsm_archiver_id_to_str(int archiver_id)

{
  
  switch (archiver_id) {
    case RSM_ARCHIVER_ID_STATUS: return "RSM_ARCHIVER_ID_STATUS";
    case RSM_ARCHIVER_ID_FIELDS: return "RSM_ARCHIVER_ID_FIELDS";
    case RSM_ARCHIVER_ID_STATISTICS: return "RSM_ARCHIVER_ID_STATISTICS";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_archiver_status

string rsm_archiver_status_to_str(int archiver_status)

{
  
  switch (archiver_status) {
    case RSM_ARCHIVER_OFFLINE: return "RSM_ARCHIVER_OFFLINE";
    case RSM_ARCHIVER_IDLE: return "RSM_ARCHIVER_IDLE";
    case RSM_ARCHIVER_WRITING: return "RSM_ARCHIVER_WRITING";
    case RSM_ARCHIVER_ERROR: return "RSM_ARCHIVER_ERROR";
    case RSM_ARCHIVER_DISKFULL: return "RSM_ARCHIVER_DISKFULL";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_svr_status

string rsm_ins_svr_status_to_str(int ins_svr_status)

{
  
  switch (ins_svr_status) {
    case RSM_INS_IDLE: return "RSM_INS_IDLE";
    case RSM_INS_POWERMEAS: return "RSM_INS_POWERMEAS";
    case RSM_INS_SETSTALO: return "RSM_INS_SETSTALO";
    case RSM_INS_SETTEST: return "RSM_INS_SETTEST";
    case RSM_INS_INITIALIZING: return "RSM_INS_INITIALIZING";
    case RSM_INS_FCMEAS: return "RSM_INS_FCMEAS";
    case RSM_INS_SETFC: return "RSM_INS_SETFC";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_pm_status

string rsm_ins_pm_status_to_str(int ins_pm_status)

{
  
  switch (ins_pm_status) {
    case RSM_INS_PM_OK: return "RSM_INS_PM_OK";
    case RSM_INS_PM_FAULT: return "RSM_INS_PM_FAULT";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_ts_status

string rsm_ins_ts_status_to_str(int ins_ts_status)

{
  
  switch (ins_ts_status) {
    case RSM_INS_TS_OK: return "RSM_INS_TS_OK";
    case RSM_INS_TS_FAULT: return "RSM_INS_TS_FAULT";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_ts_mode

string rsm_ins_ts_mode_to_str(int ins_ts_mode)

{
  
  switch (ins_ts_mode) {
    case RSM_INS_TS_MODE_CW: return "RSM_INS_TS_MODE_CW";
    case RSM_INS_TS_MODE_PULSED: return "RSM_INS_TS_MODE_PULSED";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_fc_status

string rsm_ins_fc_status_to_str(int ins_fc_status)

{
  
  switch (ins_fc_status) {
    case RSM_INS_FC_OK: return "RSM_INS_FC_OK";
    case RSM_INS_FC_FAULT: return "RSM_INS_FC_FAULT";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_fc_inputmode

string rsm_ins_fc_inputmode_to_str(int ins_fc_inputmode)

{
  
  switch (ins_fc_inputmode) {
    case RSM_INS_FC_INPUTMODE_AUTO: return "RSM_INS_FC_INPUTMODE_AUTO";
    case RSM_INS_FC_INPUTMODE_MANUAL: return "RSM_INS_FC_INPUTMODE_MANUAL";
    case RSM_INS_FC_INPUTMODE_LOWZ: return "RSM_INS_FC_INPUTMODE_LOWZ";
    case RSM_INS_FC_INPUTMODE_HIGHZ: return "RSM_INS_FC_INPUTMODE_HIGHZ";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_fc_fmrate

string rsm_ins_fc_fmrate_to_str(int ins_fc_fmrate)

{
  
  switch (ins_fc_fmrate) {
    case RSM_INS_FC_FMRATE_NORMAL: return "RSM_INS_FC_FMRATE_NORMAL";
    case RSM_INS_FC_FMRATE_LOW: return "RSM_INS_FC_FMRATE_LOW";
    case RSM_INS_FC_FMRATE_TRACK: return "RSM_INS_FC_FMRATE_TRACK";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_ins_stalo_mode

string rsm_ins_stalo_mode_to_str(int ins_stalo_mode)

{
  
  switch (ins_stalo_mode) {
    case RSM_INS_STALO_OK: return "RSM_INS_STALO_OK";
    case RSM_INS_STALO_FAULT: return "RSM_INS_STALO_FAULT";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_mserv_id

string rsm_mserv_id_to_str(int mserv_id)

{
  
  switch (mserv_id) {
    case RSM_MSERV_ID_STATUS: return "RSM_MSERV_ID_STATUS";
    case RSM_MSERV_ID_SCAL: return "RSM_MSERV_ID_SCAL";
    case RSM_MSERV_ID_PCAL: return "RSM_MSERV_ID_PCAL";
    case RSM_MSERV_ID_NCAL: return "RSM_MSERV_ID_NCAL";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_xmit_status

string rsm_xmit_status_to_str(int xmit_status)

{
  
  switch (xmit_status) {
    case RSM_XMT_STATUS_IDLE: return "RSM_XMT_STATUS_IDLE";
    case RSM_XMT_STATUS_UPDATING: return "RSM_XMT_STATUS_UPDATING";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_xmit_polmode

string rsm_xmit_polmode_to_str(int xmit_polmode)

{
  
  switch (xmit_polmode) {
    case RSM_XMT_POLMODE_H: return "RSM_XMT_POLMODE_H";
    case RSM_XMT_POLMODE_V: return "RSM_XMT_POLMODE_V";
    case RSM_XMT_POLMODE_VH: return "RSM_XMT_POLMODE_VH";
    case RSM_XMT_POLMODE_VHS: return "RSM_XMT_POLMODE_VHS";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_dual_xmit_sel

string rsm_dual_xmit_sel_to_str(int dual_xmit_sel)

{
  
  switch (dual_xmit_sel) {
    case RSM_DUAL_XMIT_SEL_1: return "RSM_DUAL_XMIT_SEL_1";
    case RSM_DUAL_XMIT_SEL_2: return "RSM_DUAL_XMIT_SEL_2";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_tsarch_id

string rsm_tsarch_id_to_str(int tsarch_id)

{
  
  switch (tsarch_id) {
    case RSM_TSARCH_ID_MODE: return "RSM_TSARCH_ID_MODE";
    case RSM_TSARCH_ID_FILENAME: return "RSM_TSARCH_ID_FILENAME";
    case RSM_TSARCH_ID_STATUS: return "RSM_TSARCH_ID_STATUS";
    case RSM_TSARCH_ID_MAXGATES: return "RSM_TSARCH_ID_MAXGATES";
    case RSM_TSARCH_ID_VERSION: return "RSM_TSARCH_ID_VERSION";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_tsarch_mode

string rsm_tsarch_mode_to_str(int tsarch_mode)

{
  
  switch (tsarch_mode) {
    case RSM_TSARCH_MODE_OFF: return "RSM_TSARCH_MODE_OFF";
    case RSM_TSARCH_MODE_AUTO: return "RSM_TSARCH_MODE_AUTO";
    case RSM_TSARCH_MODE_ON: return "RSM_TSARCH_MODE_ON";
    default: return "UNKNOWN";
  }

}

// string representation of rsm_tsarch_state

string rsm_tsarch_state_to_str(int tsarch_state)

{
  
  switch (tsarch_state) {
    case RSM_TSARCH_STATE_IDLE: return "RSM_TSARCH_STATE_IDLE";
    case RSM_TSARCH_STATE_WAITING: return "RSM_TSARCH_STATE_WAITING";
    case RSM_TSARCH_STATE_RECORDING: return "RSM_TSARCH_STATE_RECORDING";
    default: return "UNKNOWN";
  }

}

// string representation of misc_status

string rsm_misc_status_to_str(int misc_status)

{
  
  string status;
  if (misc_status & RSM_MISC_STATUS_ANTCON_NOT_CONNECTED_MASK) {
    status += "ANTCON_NOT_CONNECTED";
  }
  if (misc_status & RSM_MISC_STATUS_TXCTRL_NOT_AVAILABLE_MASK) {
    if (status.size() > 0) status += ",";
    status += "TXCTRL_NOT_AVAILABLE";
  }
  if (misc_status & RSM_MISC_STATUS_VOLUME_STARTED) {
    if (status.size() > 0) status += ",";
    status += "VOLUME_STARTED";
  }
  if (status.size() == 0) status = "NONE";
  return status;

}

// string representation of syscon error state

string rsm_syscon_err_to_str(int err)

{
  
  string status;
  if (err & RSM_SYSCON_ERR_STATE_TIMEOUT) {
    status += "TIMEOUT";
  }
  if (status.size() == 0) status = "NONE";
  return status;

}

// string representation of antcon error state

string rsm_antcon_err_to_str(int err)

{
  
  string status;
  if (err & RSM_ANTCON_ERR_ANTENNA_FAULTED) {
    status += "ANTENNA_FAULTED";
  }
  if (err & RSM_ANTCON_ERR_ANTENNA_SEQ_ERROR) {
    if (status.size() > 0) status += ",";
    status += "ANTENNA_SEQ_ERROR";
  }
  if (status.size() == 0) status = "NONE";
  return status;

}

// string representation of misc_status

string rsm_antcon_servo_to_str(int servo)

{
  
  string status;
  if (servo & RSM_ACON_MS_FE_FATAL) {
    status += "FE_FATAL";
  }
  if (servo & RSM_ACON_MS_MINUS_LIMIT) {
    if (status.size() > 0) status += ",";
    status += "MINUS_LIMIT";
  }
  if (servo & RSM_ACON_MS_PLUS_LIMIT) {
    if (status.size() > 0) status += ",";
    status += "PLUS_LIMIT";
  }
  if (servo & RSM_ACON_MS_DES_VEL_ZERO) {
    if (status.size() > 0) status += ",";
    status += "DES_VEL_ZERO";
  }
  if (servo & RSM_ACON_MS_CLOSED_LOOP) {
    if (status.size() > 0) status += ",";
    status += "CLOSED_LOOP";
  }
  if (servo & RSM_ACON_MS_AMP_ENA) {
    if (status.size() > 0) status += ",";
    status += "AMP_ENA";
  }
  if (status.size() == 0) status = "NONE";
  return status;

}

//////////////////////////////////////////////////////
// print rsm msg header

void rsm_msghdr_print(FILE *out, const rsm_msghdr_t &val)
{
  
  rsm_msghdr_t copy = val;
  // rsm_msghdr_swap(copy);
  fprintf(out, "========================= rsm_msghdr ====================\n");
  fprintf(out, "  length: %s\n", iwrf_safe_str(copy.length, 4).c_str());
  fprintf(out, "  type: \'%c\' = %s\n", copy.type,
          rsm_msgtype_to_str(copy.type).c_str());
  fprintf(out, "  module_id: %s\n", iwrf_safe_str(copy.module_id, 11).c_str());
  time_t timestamp = copy.timestamp;
  fprintf(out, "  timestamp: %s\n", DateTime::strm(timestamp).c_str());
  fprintf(out, "========================================================\n");

}

//////////////////////////////////////////////////////
// print rsm syscon state

void rsm_syscon_print(FILE *out, const rsm_syscon_t &val)
{
  
  rsm_syscon_t copy = val;
  // rsm_syscon_swap(copy);
  fprintf(out, "====================== rsm_syscon =====================\n");
  fprintf(out, "  id: %d = %s\n",
          (int) copy.id,
          rsm_syscon_id_to_str(copy.id).c_str());
  switch (copy.id) {
    case RSM_SYSCON_ID_STATE: {
      fprintf(out, "  RSM_SYSCON_ID_STATE:\n");
      fprintf(out, "    rcs_state: %s\n",
              rsm_syscon_state_to_str(copy.data.state.rcs_state).c_str());
      int secsInState =
        copy.data.state.seconds_in_state1 +
        copy.data.state.seconds_in_state2 * 256;
      fprintf(out, "    seconds_in_state: %d\n", secsInState);
      fprintf(out, "    sweep_num: %d\n", (int) copy.data.state.sweep_num);
      break;
    }
    case RSM_SYSCON_ID_SCAN: {
      fprintf(out, "  RSM_SYSCON_ID_SCAN:\n");
      fprintf(out, "    segname: %s\n",
              iwrf_safe_str(copy.data.scan.segname,
                            RSM_MAX_SEGNAME_LENGTH).c_str());
      fprintf(out, "    cause: %s\n",
              rsm_event_cause_to_str(copy.data.scan.cause).c_str());
      fprintf(out, "    scan_mode: %s\n",
              iwrf_scan_mode_to_str(copy.data.scan.scan_mode).c_str());
      int durationSecs =
        copy.data.scan.last_vol_duration1 +
        copy.data.scan.last_vol_duration2 * 256; 
      fprintf(out, "    duration_secs: %d\n", durationSecs);
      break;
    }
    case RSM_SYSCON_ID_STATUS: {
      fprintf(out, "  RSM_SYSCON_ID_STATUS:\n");
      fprintf(out, "    misc_status: %s\n",
              rsm_misc_status_to_str(copy.data.status.misc_status).c_str());
      fprintf(out, "    syscon_error_code: %s\n",
              rsm_syscon_err_to_str(copy.data.status.syscon_error_code).c_str());
      fprintf(out, "    txctrl_error_code: %s\n",
              rsm_txctrl_err_to_str(copy.data.status.txctrl_error_code).c_str());
      break;
    }
    case RSM_SYSCON_ID_VERSION: {
      fprintf(out, "  RSM_SYSCON_ID_VERSION:\n");
      fprintf(out, "    version: %s\n",
              iwrf_safe_str(copy.data.version, RSM_MAX_VER).c_str());
      break;
    }
    default: {}
  }
  fprintf(out, "========================================================\n");

}

//////////////////////////////////////////////////////
// print rsm antcon state

void rsm_antcon_print(FILE *out, const rsm_antcon_t &val)
{
  
  rsm_antcon_t copy = val;
  // rsm_antcon_swap(copy);
  fprintf(out, "====================== rsm_antcon =====================\n");
  fprintf(out, "  id: %d = %s\n",
          (int) copy.id, rsm_ant_id_to_str(copy.id).c_str());
  int iaz = copy.current_az1 + copy.current_az2 * 256;
  double az = iaz / 100.0;
  int iel = copy.current_el1 + copy.current_el2 * 256;
  double el = iel / 100.0;
  fprintf(out, "  el: %g deg\n", el);
  fprintf(out, "  az: %g deg\n", az);
  switch (copy.id) {
    case RSM_ANTCON_ID_AZELONLY: {
      break;
    }
    case RSM_ANTCON_ID_STATE: {
      fprintf(out, "  RSM_ANTCON_ID_STATE:\n");
      fprintf(out, "    ant_state: %d\n", (int) copy.data.state.ant_state);
      fprintf(out, "    sweep_num: %d\n", (int) copy.data.state.sweep_num);
      fprintf(out, "    scan_mode: %s\n",
              iwrf_scan_mode_to_str(copy.data.state.scan_mode).c_str());
      break;
    }
    case RSM_ANTCON_ID_SERVO: {
      fprintf(out, "  RSM_ANTCON_ID_SERVI:\n");
      fprintf(out, "    az_servo: %s\n", 
              rsm_antcon_servo_to_str(copy.data.servo.az_servo).c_str());
      fprintf(out, "    el_servo: %s\n", 
              rsm_antcon_servo_to_str(copy.data.servo.el_servo).c_str());
      fprintf(out, "    last_event: %d\n", (int) copy.data.servo.last_event);
      break;
    }
    case RSM_ANTCON_ID_MOTION: {
      fprintf(out, "  RSM_ANTCON_ID_MOTION:\n");
      double azRate =
        (copy.data.motion.az_rate1 + copy.data.motion.az_rate2 * 256) / 100.0;
      double elRate =
        (copy.data.motion.el_rate1 + copy.data.motion.el_rate2 * 256) / 100.0;
      double azFollowErr =
        (copy.data.motion.az_following_err1 +
         copy.data.motion.az_following_err2 * 256) / 100.0;
      double elFollowErr =
        (copy.data.motion.el_following_err1 +
         copy.data.motion.el_following_err2 * 256) / 100.0;
      fprintf(out, "    az_rate: %g\n", azRate);
      fprintf(out, "    el_rate: %g\n", elRate);
      fprintf(out, "    az_following_err: %g\n", azFollowErr);
      fprintf(out, "    el_following_err: %g\n", elFollowErr);
      break;
    }
    case RSM_ANTCON_ID_MOTION_UPDATE_RATE: {
      fprintf(out, "  RSM_ANTCON_ID_MOTION_UPDATE_RATE:\n");
      fprintf(out, "    motion_update_rate: %d\n",
              (int) copy.data.motion_update_rate);
      break;
    }
    case RSM_ANTCON_ID_ERROR: {
      fprintf(out, "  RSM_ANTCON_ID_ERROR:\n");
      fprintf(out, "    error_code: %s\n",
              rsm_antcon_err_to_str(copy.data.error.error_code).c_str());
      int nUnexpected =
        copy.data.error.unexpected_event_cnt1 +
        copy.data.error.unexpected_event_cnt2 * 256;
      fprintf(out, "    unexpected_event_cnt: %d\n", nUnexpected);
      int nSyncErr =
        copy.data.error.state_sync_errcnt1 +
        copy.data.error.state_sync_errcnt2 * 256;
      fprintf(out, "    state_sync_errcnt: %d\n", nSyncErr);
      break;
    }
    case RSM_ANTCON_ID_VERSION: {
      fprintf(out, "  RSM_ANTCON_ID_VERSION:\n");
      fprintf(out, "    version: %s\n",
              iwrf_safe_str(copy.data.version, RSM_MAX_VER).c_str());
      break;
    }
    default: {}
  }
  fprintf(out, "========================================================\n");

}

//////////////////////////////////////////////////////
// print rsm instrument power meter data

void rsm_ins_pm_print(FILE *out, const rsm_ins_pm_t &val)
{
  
  rsm_ins_pm_t copy = val;
  // rsm_ins_pm_swap(copy);
  fprintf(out, "======================== rsm_ins_pm ===================\n");
  fprintf(out, "  state: %d\n", (int) copy.state);
  int iPowerH = (int) copy.htxp2 * 256 + copy.htxp1;
  int iPowerV = (int) copy.vtxp2 * 256 + copy.vtxp1;
  double powerHDbm = (double) iPowerH / 100.0;
  double powerVDbm = (double) iPowerV / 100.0;
  fprintf(out, "  powerH (dBm) = %10.3g\n", powerHDbm);
  fprintf(out, "  powerV (dBm) = %10.3g\n", powerVDbm);
  fprintf(out, "========================================================\n");

}

