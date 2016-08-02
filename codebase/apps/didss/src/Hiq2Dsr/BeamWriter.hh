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
// $Id: BeamWriter.hh,v 1.6 2016/03/06 23:53:40 dixon Exp $
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

#include "ArcBeamMsg.hh"
#include "EolBeamMsg.hh"
#include "EolRadarMsg.hh"

using namespace std;


class BeamWriter
{

public:

  BeamWriter();
  virtual ~BeamWriter();
   
  bool initRadarParams(const int radar_id,
		       const string scan_type_name,
		       const string dynamic_origin_filename,
			const string dynamic_heading_filename,
			const bool use_dynamic_heading,
			const bool use_dynamic_origin);
  
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
  
  void loadOriginOverride();

  virtual bool processMsg(HiqMsg *msg) = 0;
  
  
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

  bool _radarParamsInitialized;
  bool _fieldParamsInitialized;
  
  bool _debug;
  bool _printSummaryFlag;
  int _summaryInterval;
  
  int _prevVolNum;
  int _prevTiltNum;
  double _prevElevation;
  double _prevAzimuth;

  bool _overrideLocation;
  bool _useDynamicLocation;

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

  double _heading_correction;
  bool _use_dynamic_heading;
  bool _use_dynamic_origin;
  string  _dynamic_origin_filename;
  string _dynamic_heading_filename;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _printSummary();
  
  bool _putStartEndFlags(const int volume_num,
			 const int tilt_num,
			 const time_t data_time) const;

  bool _putStartEndFlags(const int volume_num,
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

};

#endif
