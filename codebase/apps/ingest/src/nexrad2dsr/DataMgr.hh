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
////////////////////////////////////////////////////////////////////////////////
//
//  Class for managing the flow of data: shift, input, reformat, output
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: DataMgr.hh,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATA_MGR_INC_
#define _DATA_MGR_INC_

#include <vector>
#include <rapformats/DsRadarMsg.hh>

#include "Params.hh"
#include "Status.hh"
#include "NexradInput.hh"
#include "Reformat.hh"
#include "DsrOutput.hh"
using namespace std;


class DataMgr
{
public:
   DataMgr();
  ~DataMgr();

   //
   // Return 0 upon success, -1 upon failure
   //
   int   init( Params& params );

   //
   // Return I/O status from processing the radar data
   //
   Status::info_t   processData();

private:

   //
   // Radar data streams and reformatter
   //
   NexradInput              *inputStream;
   DsrOutput                 outputStream;
   Reformat                  reformat;

   //
   // Radar messaging
   // NOTE: the DsRadarMsg declarations MUST come before the reference
   //       declarations of the components below because of the
   //       constructor initialization.  Be careful when reordering.
   //
   DsRadarMsg                inputRadarMsg;
   DsRadarMsg                outputRadarMsg;

   //
   // Components of the radar messages
   //
   DsRadarFlags&             inputRadarFlags;
   DsRadarParams&            inputRadarParams;
   DsRadarBeam&              inputRadarBeam;
   vector<DsFieldParams*>&   inputRadarFields;

   DsRadarFlags&             outputRadarFlags;
   DsRadarParams&            outputRadarParams;
   DsRadarBeam&              outputRadarBeam;
   vector<DsFieldParams*>&   outputRadarFields;

   //
   // Radar processing
   //
   bool                      firstCall;
   bool                      inputParamsChanged;
   bool                      outputParamsChanged;
   bool                      inputFlagsChanged;
   bool                      outputFlagsChanged;
   Status::info_t            inputStatus;
   Status::info_t            outputStatus;

   void                      shiftRadarMsg();

   //
   // Associations between field types, names, units, scale, and bias
   //
   typedef struct { int    type;
                    char*  name;
                    char*  units;
                    float  scale;
                    float  bias;
                  } fieldInfo_t;

   static const fieldInfo_t FIELD_INFO[];

};

#endif
