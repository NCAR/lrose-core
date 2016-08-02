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
// InputField.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////

#ifndef _INPUT_FLD
#define _INPUT_FLD

#include <string>
#include <map>
#include <vector>
#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include "Params.hh"

using namespace std;

typedef enum {
  GRIB_SFC = 1,
  GRIB_ISOBARIC = 100,
  GRIB_MSL = 102,
  GRIB_ALTITUDE_ABOVE_MSL = 103,
  GRIB_SPECF_HEIGHT_ABOVE_GROUND = 105
} grib_vlevel_type_t;

class InputField {

public:

  InputField(const string &prog_name,
	    const Params &params,
	    int field_id,
	    int vlevel_type,
	    time_t generate_time, 
	    int forecast_time,
	    int units_conversion,
	    const string& name, 
	    const string& long_name,
	    const string& units,
	    const Pjg *proj);

  ~InputField();

  // add a plane

  void addPlane(const int& level, const fl32 *new_data);
  
  // assemble data from input map

  void assemble();

  // get methods

  inline int getFieldId() const { return _fieldId; }
  inline int getForecastTime() const { return _forecastTime; }
  inline time_t getGenerateTime() const { return _generateTime; }
  inline string getLongName() const { return _longName; }
  inline string getName() const { return _name; }
  inline string getUnits() const { return _units; }
  inline int getUnitConversion() const { return _unitConversion; }
  inline int getNx() const { return _projection->getNx(); }
  inline int getNy() const { return _projection->getNy(); }
  inline double getDx() const { return _projection->getDx(); }
  inline double getDy() const { return _projection->getDy(); }
  inline int getNz() const { return _levels.size(); }
  inline Pjg *getProjection()  const { return _projection; }

  const vector<int> &getLevels() const { return _levels; }
  const vector<fl32 *> &getData() const { return _data; }

  /// Find the array index of the supplied level.
  /// Returns -1 on failure

  int findLevelIndex(int level);
  
  // print state
  
  void print(ostream &out);
  
private:

  const string &_progName;
  const Params &_params;
  int _fieldId;
  grib_vlevel_type_t _vlevelType;
  time_t _generateTime;
  int _forecastTime;
  int _unitConversion;
  string _name;
  string _longName;
  string _units;
  Pjg *_projection;

  // data sorted by level, largest first, because
  // data is in pressure levels

  map<int, fl32*, greater<int> > _planeData;

  vector<int> _levels;
  vector<fl32 *> _data;
  
};

#endif
