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
//////////////////////////////////////////////////////////////////////
// $Id: DataMgr.cc,v 1.3 2016/03/06 23:53:42 dixon Exp $
//
// Data managment class
/////////////////////////////////////////////////////////////////////
#include <toolsa/str.h>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "EdgeRead.hh"
#include "EdgePkt.hh"
using namespace std;

DataMgr::DataMgr() 
{
   edgeMsg = NULL;
}

DataMgr::~DataMgr() 
{
  if( edgeMsg )
     delete edgeMsg;
}

int
DataMgr::init( Params& params ) 
{  
   char *logPath = NULL;
   if( strcmp( params.log_file_path, "" ) )
      logPath = STRdup( params.log_file_path );

   edgeMsg = new EdgePkt( params.port, logPath );
   if( edgeMsg->init() != SUCCESS )
      return( FAILURE );

   STRfree( logPath );
   return( SUCCESS );
}

int
DataMgr::readData() 
{
   if( edgeMsg->readUdp() != SUCCESS )
      return( FAILURE );
   if( edgeMsg->printInfo() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
   
}
