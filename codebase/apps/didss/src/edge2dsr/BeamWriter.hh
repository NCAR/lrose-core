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
////////////////////////////////////////////////////////////////////////////
// $Id: BeamWriter.hh,v 1.6 2016/03/06 23:53:42 dixon Exp $
//
///////////////////////////////////////////////////////////////////////////

/**************************************************************************
 * BeamWriter - Class for objects that write beam data to the DsRadar
 *              FMQ based on data received from an EDGE box.
 *************************************************************************/


#ifndef BeamWriter_hh
#define BeamWriter_hh

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarMsg.hh>

#include "EdgeMsg.hh"
#include "EOVStrategy.hh"
#include "ScanStrategy.hh"
using namespace std;


class BeamWriter
{

public:

  BeamWriter();
  virtual ~BeamWriter();
   
  bool init(const string &output_fmq_url,
	    const bool output_fmq_compress,
	    const int output_fmq_nslots,
	    const int output_fmq_size,
            bool write_blocking,
	    DsFmq *archive_queue,
	    const ScanStrategy &scan_strategy,
	    const int n_elevations,
	    const double max_elev_diff,
	    const double max_diff_from_scan,
	    const double max_diff_from_target,
	    MsgLog *msg_log);
  
  bool initRadarParams(const int radar_id,
		       const int n_gates_out,
		       const int polarization_code,
		       const double radar_constant,
		       const double gate_spacing,
		       const double beam_width,
		       const double wavelength,
		       const double peak_xmit_pwr,
		       const double receiver_mds,
		       const double receiver_gain,
		       const double antenna_gain,
		       const double system_gain,
		       const string &radar_name,
		       const string &site_name,
		       const string &scan_type_name);
  
  bool initFieldParams(const double dbz_scale,
		       const double dbz_bias,
		       const double vel_scale,
		       const double vel_bias,
		       const double sw_scale,
		       const double sw_bias,
		       const double uncorr_dbz_scale,
		       const double uncorr_dbz_bias);
  
  void setLatLonOverride(const double latitude,
			 const double longitude,
			 const double altitude);
  
  void setDiagnostics(const bool debug_flag,
		      const int summary_interval);
  
  void setEndOfVolStrategy(EOVStrategy *eov_strategy)
  {
    delete _eovStrategy;
    
    _eovStrategy = eov_strategy;
  }
  
  void writeBeam(const EdgeMsg &edge_msg);
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _objectInitialized;
  bool _radarParamsInitialized;
  bool _fieldParamsInitialized;
  
  bool _debug;
  int _summaryInterval;
  
  time_t _lastParamsUpdate;
  
  int _volumeNum;
  int _prevVolNum;
  bool _endOfVolumeWritten;
  double _prevElevation;
  int _prevTiltNum;
  int _tiltNum;
  bool _endOfTiltWritten;
  
  ScanStrategy *_scanStrategy;
  EOVStrategy *_eovStrategy;
  
  bool _overrideLocation;

  double _elevationDiffForTilt;
  double _elevationDiffForScan;
  double _elevationDiffInTarget;

  DsRadarMsg _radarMsg;
  DsRadarParams *_radarParams;
  DsRadarBeam *_radarBeam;
  
  DsRadarQueue _outputQueue;
  DsFmq *_archiveQueue;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _printSummary();
  void _updateParams(const EdgeMsg &edge_msg);
  
  
};

#endif
