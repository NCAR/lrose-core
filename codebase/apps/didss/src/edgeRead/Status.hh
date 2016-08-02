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
// $Id: Status.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
//
// Status class
/////////////////////////////////////////////////////////////////////////

#ifndef _STATUS_
#define _STATUS_

#include <string>
#include "LatLon.hh"
using namespace std;

class Status {

public:

   Status();
   virtual ~Status();

   int  readMsg( char* buffer );
   void printInfo( FILE* stream );

   double getAzimuth(){ return azimuth; }
   double getElevation(){ return elevation; }

   static const int STATUS_FLAGS;
   static const int BITE_SECTIONS;
   static const int BITE_FLAGS;
   static const int MAX_STRING_LEN;
   static const int STATUS_LEN;

protected:

   unsigned int   prf1;
   unsigned int   prf2;
   unsigned int   range;
   unsigned int   samples;

   float          gw1;
   float          gw2;
   unsigned int   gwPartition;
   unsigned int   rangeAvg;
   unsigned int   gates;

   unsigned int   momentEnable;
   unsigned int   softwareSim;

   unsigned int   scanType;
   double         targetAz;      
   double         targetElev; 
   unsigned int   speed;
   unsigned int   antennaSpeed;
   unsigned int   elevSpeed;  
   unsigned int   startAngle;
   unsigned int   stopAngle; 

   unsigned int   dataTime; 
   char          *siteName;  
   char          *radarType; 
   char          *jobName; 

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

   double         azimuth, elevation;


};

#endif


   
