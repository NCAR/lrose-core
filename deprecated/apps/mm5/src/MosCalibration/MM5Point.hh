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
//  $Id: MM5Point.hh,v 1.14 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////
#ifndef MM5_POINT_INC
#define MM5_POINT_INC

#include <ctime>
using namespace std;

class MM5Point 
{
public:

  MM5Point( time_t ptime, double lat, double lon, int forecastLeadSecs );
  ~MM5Point(){};
   
   void setU( double uVal ){ u = uVal; }
   void setV( double vVal ){ v = vVal; }
   void setAvgW( double wVal ){ avgW = wVal; }
   void setWspd( double wspdVal ){ wspd = wspdVal; }
   void setTemp( double temp ){ temperature = temp; }
   void setTempDiff_1000_850mb( double tempDiff ){ tempDiff_1000_850mb = tempDiff; }
   void setTempDiff_900_700mb( double tempDiff )
                       { tempDiff_900_700mb = tempDiff; }
   void setPrs( double prs ){ pressure = prs; }
   void setRainRate( double rr ){ rainRate = rr; }
   void setRH( double rh ){ relativeHum = rh; };
   void setClwht( double clwht ){ cldLiquidWaterHt = clwht; }
   void setIce( double ice ){ iceContentHt = ice; }
   void setLI( double li ){ liftedIndex = li; }
   
   time_t getTime(){ return pointTime; }
   double getLat(){ return latitude; }
   double getLon(){ return longitude; }
   
   double getU(){ return u; }
   double getV(){ return v; }
   double getAvgW(){ return avgW; }
   double getWspd(){ return wspd; }
   double getTemp(){ return temperature; }
   double getTempDiff_1000_850mb(){ return tempDiff_1000_850mb; }
   double getTempDiff_900_700mb(){ return tempDiff_900_700mb; }
   double getPrs(){ return pressure; }
   double getRainRate(){ return rainRate; }
   double getRH(){ return relativeHum; }
   double getClwht(){ return cldLiquidWaterHt; }
   double getIce(){ return iceContentHt; }
   double getLI(){ return liftedIndex; }

   double getForecastLead(){ return forecastLeadSecs; }

   //
   // Constants
   //
   static const double MISSING_VAL;
   
private:

   time_t pointTime;
   double latitude;
   double longitude;
   int forecastLeadSecs;
   
   double u;
   double v;
   double avgW;
   double wspd;
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
