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
/////////////////////////////////////////////////////////////
//  Gemtronik2Netcdf application
//
//  Jason Craig, RAP, NCAR, Boulder, CO, 80307, USA
//  March 2012
//
//  $Id: Gemtronik2Netcdf.hh,v 1.3 2016/03/07 01:23:00 dixon Exp $
//
/////////////////////////////////////////////////////////////
#ifndef _GEMTRONIK2NETCDF_HH_
#define _GEMTRONIK2NETCDF_HH_

#include <string>
#include <vector>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>

#include "Params.hh"
#include "ReadGemtronik.hh"

using namespace std;

extern MsgLog msgLog;

// 
// Macros for message logging 
// 
#define POSTMSG          msgLog.postMsg
#define DEBUG_ENABLED    msgLog.isEnabled( DEBUG )
#define INFO_ENABLED     msgLog.isEnabled( INFO )


class Gemtronik2Netcdf
{
public:
   
   //
   // Constructor
   //
  Gemtronik2Netcdf(Params *P, const char *programeName);

   //
   // Destructor
   //
  ~Gemtronik2Netcdf();

   //
   // Execution
   //
   int run(vector<string> inputFileList, time_t startTime, time_t endTime);

private:

  bool iscompressed( char *fileName );

  time_t getVolumeTime( char *fileName );

  ReadGemtronik::VolumeEnum_t getFileType( char *fileName );

  Params            *params;
  
  const char *progName;

  ReadGemtronik *readGemtronik;

};

#endif
