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
/*********************************************************************
 * EolBeamWriter : Class of objects that write beam data in the Dsr format
 *                 using radar messages in the old EOL format.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef EolBeamWriter_hh
#define EolBeamWriter_hh

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarMsg.hh>

#include "BeamWriter.hh"
#include "EOVStrategy.hh"
#include "EolBeamMsg.hh"
#include "MedianFilter.hh"
#include "ScanStrategy.hh"

using namespace std;


class EolBeamWriter : public BeamWriter
{

public:

  EolBeamWriter();
  virtual ~EolBeamWriter();
   
  virtual bool processMsg(HiqMsg *msg);
  
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  EOVStrategy *_eovStrategy;
  ScanStrategy _scanStrategy;
  
  MedianFilter *_medianFilter;
  
  EolRadarMsg *_currRadarMsg;
  EolBeamMsg *_currBeamMsg;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  bool _init(DsRadarQueue *radar_queue,
	     const ScanStrategy &scan_strategy,
	     EOVStrategy *end_of_volume_strategy,
	     const bool debug = false);
  
  bool _updateBeamData(const EolRadarMsg &radar_msg,
		       const EolBeamMsg &beam_msg,
		       DsRadarBeam &radar_beam) const;

  virtual void _updateParams(const EolRadarMsg &radar_msg,
			     const EolBeamMsg &beam_msg);

  virtual void _writeBeam(const EolRadarMsg &radar_msg,
			  EolBeamMsg &beam_msg) = 0;

};

#endif
