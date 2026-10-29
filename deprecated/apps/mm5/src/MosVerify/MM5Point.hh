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
///////////////////////////////////////////////////////////
//
//  MM5Point - class containing data from an mm5 record
//
//  $Id: MM5Point.hh,v 1.4 2016/03/07 01:33:51 dixon Exp $
//
///////////////////////////////////////////////////////////
#ifndef MM5_POINT_INC
#define MM5_POINT_INC

#include <ctime>
using namespace std;

class MM5Point 
{
public:

   MM5Point( time_t ptime, double lat, double lon );
   ~MM5Point(){};
   
   void setU( double uVal ){ u = uVal; }
   void setV( double vVal ){ v = vVal; }
   void setWspd( double wspdVal ){ wspd = wspdVal; }
   void setWdir( double wdirVal ){ wdir = wdirVal; }
   void setTemp( double temp ){ temperature = temp; }
   void setPrs( double prs ){ pressure = prs; }
   void setRH( double rh ){ relativeHum = rh; };
   
   time_t getTime(){ return pointTime; }
   double getLat(){ return latitude; }
   double getLon(){ return longitude; }
   
   double getU(){ return u; }
   double getV(){ return v; }
   double getWspd(){ return wspd; }
   double getWdir(){ return wdir; }
   double getTemp(){ return temperature; }
   double getPrs(){ return pressure; }
   double getRH(){ return relativeHum; }

   //
   // Constants
   //
   static const double MISSING_VAL;
   
private:

   time_t pointTime;
   double latitude;
   double longitude;
   
   double u;
   double v;
   double avgW;
   double wspd;
   double wdir;
   double temperature;
   double tempDiff_1000_850mb;
   double tempDiff_900_700mb;
   double pressure;
   double rainRate;
   double relativeHum;
   double cldLiquidWaterHt;
   double iceContentHt;
   double liftedIndex;
   
};

#endif
