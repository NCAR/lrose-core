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
////////////////////////////////////////////////////////////////////
//
// MetarPoint - class that contains the metar data for a given point
//
// $Id: MetarPoint.hh,v 1.3 2016/03/07 01:33:51 dixon Exp $
//
////////////////////////////////////////////////////////////////////
#ifndef METAR_POINT_INC
#define METAR_POINT_INC

#include <ctime>
using namespace std;

class MetarPoint
{
public:

   MetarPoint( time_t ptime, double lat, double lon );
   ~MetarPoint(){};

   void setWspd( double wspdVal ){ wspd = wspdVal; }
   void setWdir( double wdirVal ){ wdir = wdirVal; }
   void setTemp( double temp ){ temperature = temp; }
   void setPrs( double prs ){ pressure = prs; }
   void setRH( double rh ){ relativeHum = rh; }
   void setCeiling( double ceil ){ ceiling = ceil; }
   void setVis( double vis ){ visibility = vis; }
   
   time_t getTime(){ return pointTime; }
   double getLat(){ return latitude; }
   double getLon(){ return longitude; }
   
   double getWspd(){ return wspd; }
   double getWdir(){ return wdir; }
   double getTemp(){ return temperature; }
   double getPrs(){ return pressure; }
   double getRH(){ return relativeHum; }
   double getCeiling(){ return ceiling; }
   double getVis(){ return visibility; }

   //
   // Constants
   //
   static const double MISSING_VAL;
   
private:

   time_t pointTime;
   double latitude;
   double longitude;

   double wspd;
   double wdir;
   double temperature;
   double pressure;
   double relativeHum;
   double ceiling;
   double visibility;   
   
};

#endif
