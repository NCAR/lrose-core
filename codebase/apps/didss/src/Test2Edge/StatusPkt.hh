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
//////////////////////////////////////////////////////////////////////////
// $Id: StatusPkt.hh,v 1.8 2016/03/06 23:53:41 dixon Exp $
//
// Status Packet class
/////////////////////////////////////////////////////////////////////////

#ifndef _STATUS_PKT_
#define _STATUS_PKT_

#include <string>
#include "EdgePkt.hh"
#include "LatLon.hh"
using namespace std;

class StatusPkt : public EdgePkt {

public:

   StatusPkt( int portNum, char* logName );
   virtual ~StatusPkt();

   int  init( char* broadcastAddress,
              double latitude, double longitude, 
              char* site, char* radar, char* job,
              unsigned int prf, unsigned int nGates,
              unsigned int gateSpacing );
   void simulate( time_t now );
   int  broadcast();

   static const unsigned int RANGE_MAX;
   static const int STATUS_FLAGS;
   static const int BITE_SECTIONS;
   static const int BITE_FLAGS;
   static const int STRING_PRINT_LEN;
   static const int STATUS_LEN;

protected:

   unsigned int   prf1;
   unsigned int   prf2;
   unsigned int   range;
   unsigned int   samples;

   unsigned int   gw1;
   unsigned int   gw2;
   unsigned int   gwPartition;
   unsigned int   rangeAvg;
   unsigned int   gates;

   unsigned int   momentEnable;
   unsigned int   softwareSim;

   unsigned int   scanType;
   unsigned int   targetAz;      
   unsigned int   targetElev; 
   unsigned int   speed;
   unsigned int   antennaSpeed;
   unsigned int   elevSpeed;  
   unsigned int   startAngle;
   unsigned int   stopAngle; 

   unsigned int   dataTime; 
   string         siteName;  
   string         radarType; 
   string         jobName; 

   LatLon        *lon;
   LatLon        *lat;
   int            antennaHeight;

   unsigned int   scdFlag; 

   unsigned int   sigprocFlag;
   unsigned int   interfaceType;
   unsigned int   radarPower;
   unsigned int   servo;
   unsigned int   radiate;

   unsigned int   flags;
   unsigned int   tcfZ;
   unsigned int   tcfU;
   unsigned int   tcfV;
   unsigned int   tcfW;
   unsigned int   clutterFilter;
   unsigned int   sqi;
   unsigned int   pulseWidth;		/* 100 usec */
   unsigned int   fold;

   unsigned int  *rcuStatus;
   unsigned int **rcuBite;
   unsigned int   rcuBiteDt;


};

#endif


   
