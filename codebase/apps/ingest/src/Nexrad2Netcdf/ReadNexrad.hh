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
//////////////////////////////////////////////////////////////////////////////
//
//  ReadNexrad handles reading the actual nexrad message.
//
//  Once parsed and/or converted to a format we need the data
//  is then passed off to the 
//
//  $Id: ReadNexrad.hh,v 1.3 2016/03/07 01:23:03 dixon Exp $
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _READ_NEXRAD_HH
#define _READ_NEXRAD_HH

#include <string>
#include <cstdio>

#include "Params.hh"
#include "Nexrad2Netcdf.hh"
#include "Ingester.hh"

using namespace std;

class ReadNexrad
{
public:
   ReadNexrad(Params *P);
  ~ReadNexrad();

  //
  // Reads a raw buffer of Nexrad messages
  //
  int readBuffer( ui08 *buffer, size_t physicalBytes, bool volTitleSeen );

  //
  // Signals we are done, write the last output file.
  //
  status_t endOfData();

private:

  Params   *params;
  Ingester *ingester;

  bool volumeTitleSeen;

  //
  // Reads a complete single Msg
  //
  status_t readMsg(ui08 *buffer, size_t numberOfBytes);

  status_t readVcpData(ui08 *buffer, int bufferSize, VCP_data_t **vcp_data);

  status_t readBypassMap(ui08 *buffer, int bufferSize, BypassMap_t **bypassMap);

  status_t readClutterMap(ui08 *buffer, int bufferSize, ClutterMap_t **clutterMap);
};

#endif




