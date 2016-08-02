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
/////////////////////////////////////////////////////////
// GribField
//
// $Id: GribField.hh,v 1.7 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////
#ifndef _GRIB_FLD
#define _GRIB_FLD

#include <string>
#include <map>
#include <vector>

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>

using namespace std;

//
// Forward class declarations
//

class GribField {

public:

  GribField(const bool debug_flag = false);
  ~GribField();

  void init( const int& pid, const int& lid, const time_t& gt, 
	     const int& ft, const int& vlt, const int& uc, 
	     const float& u_rng, const float& l_rng, const string& name, 
	     const string& long_name, const string& units, const Pjg *proj );

  void reset(const int& rec_num, const int& num_levels, const float& dz, const float& min_z);

  void print();

  inline int getParameterId(){ return( _parameterId ); }
  inline int getLevelId(){ return( _levelId ); }
  inline int getForecastTime(){ return( _forecastTime ); }
  inline time_t getGenerateTime(){ return( _generateTime ); }
  inline int getVerticalLeveltype(){ return( _verticalLevelType ); }
  inline string getLongName(){ return( _longName ); }
  inline string getName(){ return( _name ); }
  inline string getUnits(){ return( _units ); }
  inline int getUnitConversion(){ return( _unitConversion ); }
  inline float getUpperLimit(){ return( _upperLimit ); }
  inline float getLowerLimit(){ return( _lowerLimit ); }
  inline int getNx(){ return( _projection->getNx() ); }
  inline int getNy(){ return( _projection->getNy() ); }
  inline int getNz(){ return( _projection->getNz() ); }
  inline int getNumPts(){ return( _projection->getNx()*_projection->getNy()*_projection->getNz() ); }
  inline Pjg *getProjection(){ return( _projection ); }
  inline fl32 *getData(){ return( _data ); }

  void addPlane(const int& level, fl32 *new_data);

  void assemble();

  inline void setUnits(const string& u){ _units = u; }

private:

  bool _debug;

  int _parameterId;
  int _levelId;
  int _verticalLevelType;
  int _forecastTime;
  int _unitConversion;
  time_t _generateTime;
  string _longName;
  string _name;
  string _units;
  float _upperLimit;
  float _lowerLimit;
  Pjg *_projection;
  map< int, fl32*> _planeData;

  fl32* _data;  
  vector<float> _levels;
};

#endif
