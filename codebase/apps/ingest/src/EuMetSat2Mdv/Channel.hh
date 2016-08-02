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
// Channel.hh
//
// Class for representing characteristics of a channel.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#ifndef Channel_HH
#define Channel_HH

#include <string>

using namespace std;

class Channel {
  
public:
  
  const static double kelvinToCelsius;
  const static double planck;
  const static double boltzman;
  const static double speedOfLight;
  const static double C1;
  const static double C2;
  
  // constructor
  
  Channel(int id, const string &name,
	  double wave_length, double wave_number,
          double cal_slope, double cal_offset,
	  double temp_correction_scale, double temp_correction_bias,
	  bool is_passive, double solar_factor);
  
  // Destructor
  
  virtual ~Channel();

  // set slope and offset
  //
  // This will be done in response to reading in a new cal file
  
  void setCalSlope(double slope) { _calSlope = slope; }
  void setCalOffset(double offset) { _calOffset = offset; }

  // compute brightness temperature from radiance
  
  double temp(double radiance) const;
  
  // get methods
  
  int getId() const { return _id; }
  const string &getName() const { return _name; }
  double getCalSlope() const { return _calSlope; }
  double getCalOffset() const { return _calOffset; }
  bool isPassive() const { return _isPassive; }
  double getSolarFactor() const { return _solarFactor; }
  
private:
  
  int _id;
  string _name;
  double _waveLength;
  double _waveNumber;
  double _calSlope;
  double _calOffset;
  double _tempCorrectionScale;
  double _tempCorrectionBias;
  bool _isPassive;
  double _solarFactor;

  double _k1, _k2;
  double _c1, _c2;

};

#endif



