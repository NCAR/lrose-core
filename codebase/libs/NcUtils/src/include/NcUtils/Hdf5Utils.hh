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
// Hdf5Utils.hh
//
// Hdf5 utilities
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////

#ifndef Hdf5Utils_HH
#define Hdf5Utils_HH

#include <string>
#include <vector>
#include <iostream>
#include <NcUtils/DataPort.hh>

// fix for HDF5 before version 1.8.12
#include "H5public.h"
// #if H5_VERSION_LE(1,8,11)
// H5_VERSION_LE macro does not work, use folling instead
#if (((H5_VERS_MAJOR==1) && (H5_VERS_MINOR==8) && (H5_VERS_RELEASE<=11)) || \
     ((H5_VERS_MAJOR==1) && (H5_VERS_MINOR<8)) || \
     (H5_VERS_MAJOR<1))
#define H5Location H5Object
#endif

#include <H5Fpublic.h>
#include <H5Epublic.h>
#include <H5Gpublic.h>

#ifndef H5_NO_NAMESPACE
#ifndef H5_NO_STD
    using std::cout;
    using std::endl;
#endif  // H5_NO_STD
#endif

#include <H5Cpp.h>

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif
using namespace std;


///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of NcUtilsFile handles I/O for netcdf files.

class Hdf5Utils

{
  
public:

  // class for decoding attributes
  
  class DecodedAttr {
  public:
    DecodedAttr();
    void clear();
    void setName(const string &val) { _name = val; }
    void setAsString(const string &val);
    void setAsInt(DataPort::si64 val);
    void setAsDouble(double val);
    DataPort::fl64 getAsDouble();
    DataPort::si64 getAsInt();
    string getAsString();
    bool isInt() { return _isInt; }
    bool isDouble() { return _isDouble; }
    bool isString() { return _isString; }
    string getName() { return _name; }
  private:
    string _name;
    DataPort::si64 _intVal;
    DataPort::fl64 _doubleVal;
    string  _stringVal;
    bool _isInt;
    bool _isDouble;
    bool _isString;
  };

  class ArrayAttr {
  public:
    ArrayAttr();
    void clear();
    void setName(const string &val) { _name = val; }
    void setAsInts(const DataPort::si64 *vals, size_t len);
    void setAsDoubles(const double *vals, size_t len);
    const DataPort::fl64 *getAsDoubles();
    const DataPort::si64 *getAsInts();
    size_t getLen() { return _len; }
    bool isInt() { return _isInt; }
    bool isDouble() { return _isDouble; }
    string getName() { return _name; }
  private:
    string _name;
    DataPort::si64 *_intVals;
    DataPort::fl64 *_doubleVals;
    size_t _len;
    bool _isInt;
    bool _isDouble;
  };

  // HDF5 access
  
  int loadAttribute(H5Location &obj,
                    const string &name,
                    const string &context,
                    DecodedAttr &decodedAttr);
  
  int loadArrayAttribute(H5Location &obj,
                         const string &name,
                         const string &context,
                         ArrayAttr &arrayAttr);
  
  static void appendAttrNames(H5Location &obj,
                              const H5std_string attr_name,
                              void *operator_data);

  int loadFloatVar(CompType compType,
                   char *buf,
                   const string &varName,
                   DataPort::fl64 &floatVal);

  int loadIntVar(CompType compType,
                        char *buf,
                        const string &varName,
                        DataPort::si64 &intVal);
  
  int loadStringVar(CompType compType,
                    char *buf,
                    const string &varName,
                    string &stringVal);
  
  int loadCompVar(CompType compType,
                  char *buf,
                  const string &varName,
                  bool &isInt,
                  bool &isFloat,
                  bool &isString,
                  DataPort::si64 &intVal,
                  DataPort::fl64 &floatVal,
                  string &stringVal);
  
  ////////////////////////////////////////////
  /// printing

  void printGroup(Group &group, const string grname,
                  ostream &out,
                  bool printRays, bool printData);
  
  void printDataSet(DataSet &ds, const string dsname,
                    ostream &out,
                    bool printRays, bool printData);
  
  void printCompoundType(CompType &compType,
                         int ipoint,
                         char *buf,
                         ostream &out);
  
  void printAttributes(H5Location &obj, ostream &out);
  
  void printAttribute(Attribute &attr, ostream &out);
  
  void printDataSet(DataSet &ds, ostream &out);

  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Clear error string.
  
  void clearErrStr();

  /// Get the Error String.
  ///
  /// The contents are only meaningful if an error has returned.
  
  string getErrStr() const { return _errStr; }
  
  //@}

protected:
private:
  
  // error string

  string _errStr; ///< Error string is set on read or write error
  
  // debug state
  
  bool _debug; ///< normal debug flag
  bool _verbose; ///< verbose debug flag

  void _printDataVals(ostream &out, int nPoints,
                      DataPort::fl64 *vals) const;
  
  void _printDataVals(ostream &out, int nPoints,
                      DataPort::si64 *vals) const;
  
  void _printPacked(DataPort::fl64 val, int count, string &outStr) const;
  
  void _printPacked(DataPort::si64 val, int count, string &outStr) const;
  
  /// add integer value to error string, with label
  
  void _addErrInt(string label, int iarg,
                  bool cr = true);
  
  /// add double value to error string, with label

  void _addErrDbl(string label, double darg,
                  string format, bool cr = true);

  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);
  
private:

};

#endif
