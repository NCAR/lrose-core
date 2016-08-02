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


 
#ifndef DECODE_H
#define DECODE_H   


#include <ctime>
#include "Params.hh"
using namespace std;


class Decoders {
 
public:        

  Decoders(const Params *params,
           time_t fileTime, const char *fileName);
  ~Decoders();
  
  //
  // Public decoding routines.
  //
  int TTAA_decode(string code, string Line);  
  int TTBB_decode(string code, string Line);
  int TTCC_decode(string code, string Line);
  int TTDD_decode(string code, string Line);
  int PPAA_decode(string code, string Line);
  int PPBB_decode(string code, string Line);
  int PPCC_decode(string code, string Line);
  int PPDD_decode(string code, string Line);

  //
  // Debugging printout.
  //
  void print(int printIfNoPoints = 0);
  //
  // See if we got sounding data.
  //
  int gotData();
  //
  //
  // Go back to initial state - null internal data.
  //
  void reset();
  //
  // Write the data out to SPDB. Returns 1 if it went OK, else 0.
  //
  int write();
  //
  protected :
  private :
  //
  // Methods.
  //
  // Section 1 decoders.
  //
  void _setYearMonthFromFilename(time_t fileTime, const char *fileName);
  int _decodeTime( const char *token );
  int _decodeWindPressureHundreds( const char *token );
  int _decodeWindPressureTens( const char *token );
  int _decodeIsEndMessage( const char *p );
  double _decode_GetNext_AA_Pressure( double Pr );
  double _decode_GetNext_CC_Pressure( double Pr );

  //
  // Decoders for AA messages.
  //
  double _decode_AA_Temp(const char *TTTDD );
  double _decode_AA_DewPoint(const char *TTTDD );
  void  _decode_AA_Wind(const char *ddfff, double *u, double *v );
  double _decode_AA_99Pressure(const char *code_99PPP );
  void  _decode_AA_PPhhh(const char *PPhhh, double *press, double *ht );
  void  _decode_CC_PPhhh(const char *PPhhh, double *press, double *ht );
  //
  // Decoders for BB messages
  //
  void  _decode_BB_Source( const char *YYGGa );
  double _decode_BB_nnPPP( const char *nnPPP );
  void  _decode_AA_44nPP( const char *code_44nPP, int *n, double *pres );
  void  _decode_CC_44nPP( const char *code_44nPP, int *n, double *pres );
  //
  // Decoders for DD messages.
  //
  int _decode_DD_ITUUU( const char *ITUUU, double *H1, double *H2, 
			double *H3);
  //
  // Station locator.
  //
  int _getLocation();
  //
  // Data.
  //
  const Params *_params;
  int _year, _month; // Set in constructor.
  int _hour, _day, _unitsKnots; // Decoded from section 1.
  time_t _dataTime;
  double _windPressure;
  int _numPoints;
  int _stationID;
  double _lat, _lon, _alt;
  double _surfPres;
  double _presScale;
  //
  static const double _badVal;
  //
  // Arrays to fill. Statically allocated, which is not great, but for 
  // the small number of points I'm dealing with I think it's OK.
  //
  static const int _maxNumPoints = 100;
  //
  double _pressure[_maxNumPoints];
  double _temp[_maxNumPoints];
  double _rh[_maxNumPoints];
  double _u[_maxNumPoints];
  double _v[_maxNumPoints];
  double _h[_maxNumPoints];
  double _dp[_maxNumPoints];
  //
  // Source info
  //
  const char *_source;

  //
  // Buffer.
  //
  char work[32];

};


#endif

