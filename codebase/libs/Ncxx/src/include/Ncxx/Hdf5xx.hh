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
// Hdf5xx.hh
//
// Hdf5 utilities
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2017
//
///////////////////////////////////////////////////////////////

#ifndef Hdf5xx_HH
#define Hdf5xx_HH

#include <string>
#include <vector>
#include <iostream>
#include <Ncxx/NcxxPort.hh>

#include <H5public.h>
#include <H5Fpublic.h>
#include <H5Epublic.h>
#include <H5Gpublic.h>

using std::cout;
using std::endl;

#include <Ncxx/H5x.hh>

using namespace H5x;
using namespace std;


///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR HDF5 FILE FORMAT

class Hdf5xx

{
  
public:

  // object type

  typedef enum
    {
     OBJECT_GROUP,
     OBJECT_DATASET,
     OBJECT_NAMED_DATATYPE,
     OBJECT_UNKNOWN
    } hdf5_object_t;
                           
  // class for decoding attributes
  
  class DecodedAttr {
  public:
    DecodedAttr();
    void clear();
    void setName(const string &val) { _name = val; }
    void setAsString(const string &val);
    void setAsInt(NcxxPort::si64 val);
    void setAsDouble(double val);
    NcxxPort::fl64 getAsDouble();
    NcxxPort::si64 getAsInt();
    string getAsString();
    bool isInt() { return _isInt; }
    bool isDouble() { return _isDouble; }
    bool isString() { return _isString; }
    string getName() { return _name; }
  private:
    string _name;
    NcxxPort::si64 _intVal;
    NcxxPort::fl64 _doubleVal;
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
    void setAsInts(const NcxxPort::si64 *vals, size_t len);
    void setAsDoubles(const double *vals, size_t len);
    const NcxxPort::fl64 *getAsDoubles();
    const NcxxPort::si64 *getAsInts();
    size_t getLen() { return _len; }
    bool isInt() { return _isInt; }
    bool isDouble() { return _isDouble; }
    string getName() { return _name; }
  private:
    string _name;
    NcxxPort::si64 *_intVals;
    NcxxPort::fl64 *_doubleVals;
    size_t _len;
    bool _isInt;
    bool _isDouble;
  };

  // HDF5 access
  
  int loadFloatVar(CompType compType,
                   char *buf,
                   const string &varName,
                   NcxxPort::fl64 &floatVal);

  int loadIntVar(CompType compType,
                 char *buf,
                 const string &varName,
                 NcxxPort::si64 &intVal);
  
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
                  NcxxPort::si64 &intVal,
                  NcxxPort::fl64 &floatVal,
                  string &stringVal);
  
  int loadAttribute(H5Object &obj,
                    const string &name,
                    const string &context,
                    DecodedAttr &decodedAttr);
  
  int loadArrayAttribute(H5Object &obj,
                         const string &name,
                         const string &context,
                         ArrayAttr &arrayAttr);
  
  // get attributes of various types
  
  static string getStringAttribute(H5Object &obj,
                                   const string &name);
  
  static int getIntAttribute(H5Object &obj,
                             const string &name);
  
  static double getDoubleAttribute(H5Object &obj,
                                   const string &name);
  
  // get object type, by index
  
  static hdf5_object_t getObjTypeByIdx(Group &group, size_t index);
  
  ///////////////////////////////////////////////////////////////////
  // Enquire about the properties of a variable
  // Returns 0 on success, -1 on failure
  // On success, sets the following:
  //    dims     - dimensions
  //    units    - string
  //    h5class  - H5T_INTEGER or H5T_FLOAT
  //    h5sign   - H5T_SGN_NONE if unsigned integer, otherwise signed
  //               does not apply to floats of course
  //    h5order  - H5T_ORDER_LE or H5T_ORDER_BE
  //    h5size   - length of data type in bytes
  
  int getVarProps(Group &group,
                  const string &dsName,
                  vector<size_t> &dims,
                  string &units,
                  H5T_class_t &h5class,
                  H5T_sign_t &h5sign,
                  H5T_order_t &h5order,
                  size_t &h5size);
  
  ///////////////////////////////////////////////////////////////////
  // Read data set into arrays of various types
  // Fills in dims, msssingVal, vals, units (if available)
  // return 0 on success, -1 on failure

  int readSi16Array(Group &group,
                    const string &dsname,
                    vector<size_t> &dims,
                    NcxxPort::si16 &missingVal,
                    vector<NcxxPort::si16> &vals,
                    string &units);
  
  int readSi32Array(Group &group,
                    const string &dsname,
                    vector<size_t> &dims,
                    NcxxPort::si32 &missingVal,
                    vector<NcxxPort::si32> &vals,
                    string &units);
  
  int readFl32Array(Group &group,
                    const string &dsname,
                    vector<size_t> &dims,
                    NcxxPort::fl32 &missingVal,
                    vector<NcxxPort::fl32> &vals,
                    string &units);
  
  int readFl64Array(Group &group,
                    const string &dsname,
                    vector<size_t> &dims,
                    NcxxPort::fl64 &missingVal,
                    vector<NcxxPort::fl64> &vals,
                    string &units);
  
  /////////////////////////////////////////////////
  // add a string attribute to an object on write
  // returns the attribute
  
  static Attribute addAttr(H5Object &loc,
                           const string &name,
                           const string &val);
  
  // add a 64-bit int attribute to an object
  // returns the attribute
  
  static Attribute addAttr(H5Object &loc,
                           const string &name,
                           NcxxPort::si64 val);

  // add a 64-bit float attribute to an object
  // returns the attribute
  
  static Attribute addAttr(H5Object &loc,
                           const string &name,
                           NcxxPort::fl64 val);
  
  // add a 64-bit float array attribute to an object
  // returns the attribute
  
  static Attribute addAttr(H5Object &loc,
                           const string &name,
                           const vector<NcxxPort::fl64> &vals);
  
  ////////////////////////////////////////////
  /// printing

  // print group details
  
  static void printGroup(Group &group, const string grname,
                         ostream &out,
                         bool printRays, bool printData);
  
  // recursively print the file structure

  static void printFileStructure(Group &grp,
                                 int level,
                                 ostream &out);
  
  static void printDataSet(DataSet &ds, const string dsname,
                           ostream &out,
                           bool printRays, bool printData);
  
  static void printCompoundType(CompType &compType,
                                int ipoint,
                                char *buf,
                                ostream &out);
  
  static void printAttributes(H5Object &obj, ostream &out, int level = 0);
  
  static void printAttribute(Attribute &attr, ostream &out, int level = 0);
  
  static void printDataSet(DataSet &ds, ostream &out);

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

  static void _printDataVals(ostream &out, int nPoints,
                             NcxxPort::fl64 *vals);
  
  static void _printDataVals(ostream &out, int nPoints,
                             NcxxPort::si64 *vals);
  
  static void _printPacked(NcxxPort::fl64 val, int count, string &outStr);
  
  static void _printPacked(NcxxPort::si64 val, int count, string &outStr);
  
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
