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
// $Id: SimpleBeamWriter.hh,v 1.2 2016/03/06 23:53:41 dixon Exp $
//
///////////////////////////////////////////////////////////////////////////

/**************************************************************************
 * SimpleBeamWriter : Class of objects that write beam data in the Dsr
 *                    format in a simple fashion.  This writer sends the
 *                    data through as is while only trying to detect the
 *                    end of volume since this is the only flag required
 *                    by Dsr2Vol.
 *************************************************************************/


#ifndef SimpleBeamWriter_hh
#define SimpleBeamWriter_hh

#include "BeamWriter.hh"

using namespace std;


class SimpleBeamWriter : public BeamWriter
{

public:

  SimpleBeamWriter();
  virtual ~SimpleBeamWriter();
   
  bool init(DsRadarQueue *radar_queue,
	    const ScanStrategy &scan_strategy,
	    const double max_diff_from_scan,
	    EOVStrategy *end_of_volume_strategy,
	    const double max_legal_elev_drop,
	    const bool get_tilt_num_from_header = true,
	    const bool get_vol_num_from_header = true,
	    const bool get_beam_time_from_header = true,
	    const bool debug = false);
  
  void writeBeam(const HiqRadarMsg &radar_msg,
		 HiqBeamMsg &beam_msg);

  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _objectInitialized;

  bool _getTiltNumFromHeader;
  bool _getVolNumFromHeader;
  bool _getBeamTimeFromHeader;

  time_t _lastParamsUpdate;
  
  double _maxDiffFromScan;
  double _maxLegalElevDrop;
  
  int _prevBeamNum;

  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
