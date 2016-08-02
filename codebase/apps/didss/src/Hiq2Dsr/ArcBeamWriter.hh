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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:40 $
//   $Id: ArcBeamWriter.hh,v 1.2 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ArcBeamWriter : Class of objects that write beam data in the Dsr format
 *                 using radar messages in the new ARC format.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef ArcBeamWriter_hh
#define ArcBeamWriter_hh

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarMsg.hh>

#include "BeamWriter.hh"

using namespace std;


class ArcBeamWriter : public BeamWriter
{

public:

  ArcBeamWriter(const bool get_beam_time_from_header = false);
  virtual ~ArcBeamWriter();
   
  bool init(DsRadarQueue *radar_queue,
	    const bool debug = false);
  
  virtual bool processMsg(HiqMsg *msg);
  
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _objectInitialized;
  
  bool _getBeamTimeFromHeader;

  time_t _lastParamsUpdate;
  int _prevBeamNum;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  bool _updateBeamData(const ArcBeamMsg &beam_msg,
		       DsRadarBeam &radar_beam) const;

  virtual void _updateParams(const ArcBeamMsg &beam_msg);
  
  virtual void _writeBeam(ArcBeamMsg &beam_msg);

};

#endif
