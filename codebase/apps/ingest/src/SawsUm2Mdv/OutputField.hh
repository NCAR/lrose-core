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
// OutputField.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2005
//
///////////////////////////////////////////////////////////////

#ifndef OutputField_hh
#define OutputField_hh

#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <Mdv/MdvxField.hh>
#include "Params.hh"
class InputField;
using namespace std;

////////////////////////
// This class

class OutputField {
  
public:

  OutputField(const string &prog_name,
	      const Params &params,
	      const string &name,
	      const string &long_name,
	      const string &units,
	      Params::grib_field_id_t grib_field_id,
	      Params::units_conversion_t units_conversion,
	      Params::encoding_type_t encoding);
  
  ~OutputField();

  // add a grib field
  
  void addInputField(InputField *field);

  // assemble field
  
  void assemble();

  // derive wspd from u and v

  int deriveWspd(const OutputField &ufld, const OutputField &vfld);

  // interpolate vlevels as appropriate

  void interpVlevels();

  // create MdvxField object from this object
  
  MdvxField *createMdvxField(time_t gen_time,
			     int forecast_lead_time);
  
  // clear

  void clear();

  // print
  
  void print(ostream &out) const;
  void printInputFields(ostream &out) const;

  // get methods

  const string &getName() { return _name; }
  const string &getLongName() { return _longName; }
  const string &getUnits() { return _units; }
  const Params::grib_field_id_t getGribFieldId() {
    return _gribFieldId;
  }
  const Params::units_conversion_t getUnitsConversion() {
    return _unitsConversion;
  }
  const Params::encoding_type_t getEncoding() {
    return _encoding;
  }
  
protected:
private:

  typedef struct {
    double level;
    int ilevel1, ilevel2;
    double wt1, wt2;
  } interp_t;

  const string &_progName;
  const Params &_params;

  string _name;
  string _longName;
  string _units;
  Params::grib_field_id_t _gribFieldId;
  Params::units_conversion_t _unitsConversion;
  Params::encoding_type_t _encoding;

  vector<InputField *> _fields;

  double _dX, _dY;
  double _minX, _minY;
  double _maxX, _maxY;
  int _nX, _nY;
  
  vector<int> _levels;
  vector<fl32> _vlevels;
  fl32 *_data;

  void _useNativeVlevels();
  void _interpVlevels();

};

#endif

