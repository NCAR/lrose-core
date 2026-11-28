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
// Windtracer2ncd.hh
//
// Windtracer2ncd object
//
//
///////////////////////////////////////////////////////////////


#ifndef Windtracer2ncd_H
#define Windtracer2ncd_H

#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>
#include <vector>

using namespace std;

class Windtracer2ncd {
  
public:
  
  // constructor.
  Windtracer2ncd (Params *TDRP_params);

  // 
  void  Windtracer2ncdInit( char *filename); 
  void  Windtracer2ncdFile( char *filename); 
  
  // destructor.
  ~Windtracer2ncd();

  
protected:
  
private:

  Params *      _params;

  int _year, _month, _day;
  int _hour, _min, _sec, _msec;
  float _el, _az;

  int _numBeams;
  int _numFields;
  int _maxLen;
  int _numGates;

  float _firstRange, _deltaRange;

  vector <char *> _fieldNames;
  vector <float *> _fields; 

  float *_elevations;
  float *_azimuths;
  float *_years;
  float *_months;
  float *_days;
  float *_hours;
  float *_mins;
  float *_secs;
  float *_msecs;

  int _startYear, _startMonth, _startDay;
  int _startHour, _startMinute, _startSec;

  int _readShort(FILE **fp);
  int _readLong(FILE **fp);
  char *_idToStr( int ID );
  void _byteSwap4(void *p);
  void _readKeyword(FILE *fp, char *key, double *val, int tryRead);
 
  void _processSpecial(FILE *fp, bool noisy);
  void _processHeader(FILE *fp);
  void _processText(FILE *fp, int len);
  void _processExistingText();


};

#endif
