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
/////////////////////////////////////////////////
// $Id: DataMgr.cc,v 1.8 2016/03/06 23:53:41 dixon Exp $
//
// Data managment class
////////////////////////////////////////////////

#include "DataMgr.hh"
#include "Test2Edge.hh"
#include "StatusPkt.hh"
#include "RayPkt.hh"
using namespace std;

DataMgr::DataMgr() 
{
   statusPkt  = NULL;
   rayPkt     = NULL;
   interval   = 0;
   lastTime   = 0;
}

DataMgr::~DataMgr() 
{
  if( statusPkt )
     delete statusPkt;
  if( rayPkt )
     delete rayPkt;
}

int
DataMgr::init( Params& params ) 
{
   int worked;
   
   switch( params.packet_type ) {
       case Params::STATUS:
	  statusPkt = new StatusPkt( params.port, 
                                     params.log_file_path );

	  worked = statusPkt->init( params.broadcast_address,
                                    params.lat, params.lon, 
                                    params.site, params.radar, 
                                    params.job, 
                                    (unsigned int) params.prf, 
                                    (unsigned int) params.n_gates,
                                    (unsigned int) params.gate_spacing );
	  
          if( worked != SUCCESS ) 
	     return( FAILURE );
	  break;
	  
       case Params::BEAM_DATA:
	  
	  rayPkt = new RayPkt( params.port, 
                               params.log_file_path );
	  
          worked = rayPkt->init( params.broadcast_address,
                                 (int) params.gate_spacing );
	  
          if( worked != SUCCESS )
	     return( FAILURE );
	  break;
   }

   interval = params.interval;

   return( SUCCESS );
}

int
DataMgr::writeData( time_t now ) 
{
   if( statusPkt ) {
      if( now - lastTime > interval ) {
         statusPkt->simulate( now );

	 if( statusPkt->broadcast() != SUCCESS )
	    return( FAILURE );

	 lastTime = now;
      }
   } else if( rayPkt ) {
      if( now - lastTime > interval ) {
         rayPkt->simulate( now );

	 if( statusPkt->broadcast() != SUCCESS )
	    return( FAILURE );

	 lastTime = now;
      }
   }

   return( SUCCESS );
   
}
