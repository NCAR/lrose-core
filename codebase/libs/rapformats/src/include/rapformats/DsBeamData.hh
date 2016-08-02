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
//////////////////////////////////////////////////////////////
//
// DsBeamData.hh
// Low level handling of actual beam arrays (1 2 or 4 bytes.)
// (see DsRadarBeam.hh)
//
// Each beam contains data at gates for one or more fields, in an
// assumed order.  (field0,gate0, field1,gate0, ... , )
//
// The actual data is sometimes raw (1 or 2 bytes) with scaling to floating
// point  or floating point (4 bytes) with no scaling.
//
// NOTE: Little to no checking for input args in range (gate/field) is done
// in the methods.
//
// The tricky part is the missing data, and interpreting the data
// either as floating point or not.
// 

/////////////////////////////////////////////////////////////
#ifndef _DsBeamData_H_
#define _DsBeamData_H_

#include <cstdio>
#include <vector>
#include <dataport/port_types.h>
#include <rapformats/DsBeamDataFieldParms.hh>
#include <rapformats/DsRadarBeam.hh>
using namespace std;

class DsBeamData {

public:

  /**
   * constructor
   *
   * bytewidth = 1,2 or 4
   * nfields   = number of data types, >= 1
   * ngates    = number of gates in the beam.
   * p         = field parameters used for scaling to floating point and
   *             handling missing data, one per field. If the data bytewidth
   *             is 4, the scale and bias are not used.
   */
  DsBeamData(const int bytewidth, const int nfields, const int ngates,
	     const vector<DsBeamDataFieldParms> &p);

  DsBeamData(const DsBeamData &b);
  virtual ~DsBeamData();
  void operator=(const DsBeamData &b);
  bool operator==(const DsBeamData &b) const;

  inline int get_byte_width(void) const {return _bytewidth;}
  inline int get_ngates(void) const {return _ngates;} 
  inline int get_nfields(void) const {return _nfields;} 

  /*
   * Return pointer to the beam, cast to ui08 no matter what it really is
   */
  ui08 *get_ptr(void) const; 

  /*
   * Fill the entire beam with missing data
   */
  void fill_with_missing(void);

  /*
   * Set beam to missing for a particular field at a particular gate
   */
  void set_to_missing(const int ifield, const int igate);

  /*
   * Return true if data for a particular field is missing at a gate
   */
  bool is_missing_at(const int ifield, const int igate) const;

  /*
   * Return number of missing data values in the beam
   */
  int num_missing(void) const;

  /*
   * Copy the data from the input RadarBeam into the DsBeamData
   */
  void copy(const DsRadarBeam &b);

  /*
   * Copy contents of one beam into another.
   * Return true for success
   */
  bool copy_contents(const DsBeamData &b);

  /*
   * Return the raw data value at the point, cast to fl32 for all byteWidths
   */
  fl32 get_value(const int ifield,  const int igate) const;

  /*
   * If the beam bytewidth is 2, return the value at the gate/field
   * otherwise return the missing data value for the field, cast to ui16
   */
  ui16 get_ui16_value(const int ifield,  const int igate) const;

  /*
   * Return the scaled floating point value at the gate/field.
   *
   * If data is missing at the point, the floating point missing
   * value specified in the DsBeamDataFieldParms for that field
   * is returned.
   */
  fl32 get_scaled_value(const int ifield,  const int igate) const;

  /*
   * Print scaled, raw values and scale and bias values at a point
   */
  void print_debug_value(const int ifield,  const int igate) const;

  /*
   * Print out all values in the beam
   */
  void print_values(const char *msg) const;

  /*
   * Print out all values in the beam, with each line starting with
   * the current gate index
   */
  void print_values_with_index(const char *msg) const;

  /*
   * Print out all values in the beam, with each line starting with
   * the current gate index
   */
  void print_scaled_values_with_index(const char *msg) const;

  /*
   * Store input scaled value in internal format to gate/field.
   */
  void put_scaled_value(const fl32 f, const int ifield, const int igate);

  /*
   * A fundamental assumption about DsBeamData is that 4 byte data is float
   * point
   */
  inline static bool is_floating_data(const int bytewidth) 
  {
    return bytewidth == 4;
  }

private:

  int _nfields, _ngates, _bytewidth;
  vector<DsBeamDataFieldParms> _parm;
  fl32 *_f4ptr;
  ui16 *_i2ptr;
  ui08 *_i1ptr;
  
  void _write(int ifield, int igate, int value);
  void _free(void);
  void _copy(const DsBeamData &b);
  void _copy_contents(const DsBeamData &b);
  void _print_value(const int i) const;
};
   
#endif

