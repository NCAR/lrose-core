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
// $Id: BeamWriter.hh,v 1.13 2016/03/06 23:53:39 dixon Exp $
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

#include "BinetBeamMsg.hh"
#include "BinetRadarMsg.hh"
#include "EOVStrategy.hh"
#include "ScanStrategy.hh"

using namespace std;


class BeamWriter
{

public:

  BeamWriter();
  virtual ~BeamWriter();
   
  bool init(DsRadarQueue *radar_queue,
	    const ScanStrategy &scan_strategy,
	    const double max_diff_from_scan,
	    EOVStrategy *end_of_volume_strategy,
	    const double max_legal_elev_drop,
	    const double max_elev_diff,
	    const double max_azimuth_diff,
	    const double azimuth_increment,
	    const bool get_tilt_num_from_header = true,
	    const bool get_vol_num_from_header = true,
	    const bool get_beam_time_from_header = true,
	    const bool fix_elevations = false,
	    const bool fix_azimuths = false,
	    const bool debug = false);
  
  bool initRadarParams(const int radar_id,
		       const string scan_type_name);
  
  bool initFieldParams(const double dbz_scale,
		       const double dbz_bias,
		       const double vel_scale,
		       const double vel_bias,
		       const double sw_scale,
		       const double sw_bias,
		       const double coherent_dbz_scale,
		       const double coherent_dbz_bias,
		       const double ncp_scale,
		       const double ncp_bias,
		       const double power_scale,
		       const double power_bias);
  
  void setLatLonOverride(const double latitude,
			 const double longitude,
			 const double altitude);
  
  void setDiagnostics(const bool print_summary,
		      const int summary_interval);
  
  void updateParams(const BinetRadarMsg &radar_msg,
		    const BinetBeamMsg &beam_msg);

  void writeBeam(const BinetRadarMsg &radar_msg,
		 BinetBeamMsg &beam_msg);

  
protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  typedef enum
  {
    REFL_FIELD_OFFSET = 0,
    VEL_FIELD_OFFSET,
    SW_FIELD_OFFSET,
    COH_REFL_FIELD_OFFSET,
    NCP_FIELD_OFFSET,
    POWER_FIELD_OFFSET,
    NUM_OUTPUT_FIELDS
  } output_field_offsets_t;

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _objectInitialized;
  bool _radarParamsInitialized;
  bool _fieldParamsInitialized;
  
  bool _debug;
  bool _printSummaryFlag;
  int _summaryInterval;
  
  bool _getTiltNumFromHeader;
  bool _getVolNumFromHeader;
  bool _getBeamTimeFromHeader;

  time_t _lastParamsUpdate;
  
  ScanStrategy _scanStrategy;
  double _maxDiffFromScan;
  EOVStrategy *_eovStrategy;
  double _maxLegalElevDrop;
  double _maxElevDiff;
  double _maxAzimuthDiff;
  
  bool _fixElevations;
  bool _fixAzimuths;
  bool _beamDropping;
  int _prevVolNum;
  double _prevElevation;
  bool _prevElevationBad;
  double _prevAzimuth;
  double _prevAzimuthDiff;
  bool _prevAzimuthBad;
  double _azimuthIncrement;
  int _prevTiltNum;
  int _prevBeamNum;

  bool _overrideLocation;

  double _reflScale;
  double _reflBias;
  double _velScale;
  double _velBias;
  double _swScale;
  double _swBias;
  double _coherentReflScale;
  double _coherentReflBias;
  double _ncpScale;
  double _ncpBias;
  double _powerScale;
  double _powerBias;
  
  DsRadarMsg _radarMsg;
  DsRadarParams *_radarParams;
  DsRadarBeam *_radarBeam;
  
  DsRadarQueue *_radarQueue;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  bool _fixAzimuth(BinetBeamMsg &beam_msg,
		   const bool missing_previous_beams);
  bool _fixElevation(BinetBeamMsg &beam_msg,
		     const bool missing_previous_beams);

  void _printSummary();
  
  bool _putStartEndFlags(const int volume_num,
			 const int tilt_num,
			 const time_t data_time) const;

  static inline ui08 _scaleValue(const double value,
				 const double scale,
				 const double bias)
  {
    double scaled_value = (value - bias) / scale + 0.5;

    if (scaled_value < 1.0)
      return 1;
    if (scaled_value > 255.0)
      return 255;

    return (ui08)scaled_value;
  }

  bool _updateBeamData(const BinetRadarMsg &radar_msg,
		       const BinetBeamMsg &beam_msg,
		       DsRadarBeam &radar_beam) const;

};

#endif
