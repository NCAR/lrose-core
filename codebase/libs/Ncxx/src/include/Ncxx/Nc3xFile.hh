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
// Nc3xFile.hh
//
// NetCDF file wrapper
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2017
//
///////////////////////////////////////////////////////////////

#ifndef Nc3xFile_HH
#define Nc3xFile_HH

#include <string>
#include <vector>
#include <Ncxx/Nc3File.hh>

using namespace std;

///////////////////////////////////////////////////////////////
/// CLASS FOR NETCDF IO OPERATIONS

class Nc3xFile

{
  
public:

  /// Constructor
  
  Nc3xFile();
  
  /// Destructor
  
  virtual ~Nc3xFile();

  //////////////////////////////////////////////////////////////
  /// \name File operations
  //@{

  /// open for reading
  /// Returns 0 on success, -1 on failure
  
  int openRead(const string &path);

  /// open netcdf file for writing
  /// create error object so we can handle errors
  /// set the netcdf format, before a write
  /// format options are:
  ///   Classic - classic format (i.e. version 1 format)
  ///   Offset64Bits - 64-bit offset format
  ///   Netcdf4 - netCDF-4 using HDF5 format
  ///   Netcdf4Classic - netCDF-4 using HDF5 but only netCDF-3 calls
  /// Returns 0 on success, -1 on failure
  
  int openWrite(const string &path, Nc3File::FileFormat format);

  /// close previously-opened file

  void close();

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Attributes
  //@{
  
  /// add string global attribute
  /// Returns 0 on success, -1 on failure

  int addGlobAttr(const string &name, const string &val);

  /// add int, float or double global attribute
  /// Returns 0 on success, -1 on failure
  
  int addGlobAttr(const string &name, int val);
  int addGlobAttr(const string &name, float val);
  int addGlobAttr(const string &name, double val);

  /// add int[], float[] or double[] global attribute
  /// Returns 0 on success, -1 on failure
  
  int addGlobAttr(const string &name, int len, int *vals);
  int addGlobAttr(const string &name, int len, float *vals);
  int addGlobAttr(const string &name, int len, double *vals);

  // read a global attribute
  // Returns 0 on success, -1 on failure
  
  int readGlobAttr(const string &name, string &val);
  int readGlobAttr(const string &name, int &val);
  int readGlobAttr(const string &name, float &val);
  int readGlobAttr(const string &name, double &val);

  /// add attribute of various types
  /// Returns 0 on success, -1 on failure

  int addAttr(Nc3Var *var, const string &name, const string &val);
  int addAttr(Nc3Var *var, const string &name, ncbyte val);
  int addAttr(Nc3Var *var, const string &name, short val);
  int addAttr(Nc3Var *var, const string &name, int val);
  int addAttr(Nc3Var *var, const string &name, long val);
  int addAttr(Nc3Var *var, const string &name, float val);
  int addAttr(Nc3Var *var, const string &name, double val);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name dimensions
  //@{
  
  int addDim(Nc3Dim* &dim, const char *name, int size);
  int readDim(const string &name, Nc3Dim* &dim);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name variables
  //@{
  
  /// Add scalar meta-data variable
  /// Returns 0 on success, -1 on failure
  
  int addMetaVar(Nc3Var* &var, const string &name, 
                 const string &standardName,
                 const string &longName,
                 Nc3Type ncType, 
                 const string &units = "");
  
  // Add scalar meta-data variable
  // Returns var on success, NULL on failure
  
  Nc3Var *addMetaVar(const string &name, 
                    const string &standardName,
                    const string &longName,
                    Nc3Type ncType, 
                    const string &units = "");
  
  /// Add 1-D array meta-data variable
  /// Returns 0 on success, -1 on failure

  int addMetaVar(Nc3Var* &var, const string &name, 
                 const string &standardName,
                 const string &longName,
                 Nc3Type ncType, Nc3Dim *dim, 
                 const string &units = "");
  
  // Add 1-D array meta-data variable
  // Returns var on success, NULL on failure
  
  Nc3Var *addMetaVar(const string &name, 
                    const string &standardName,
                    const string &longName,
                    Nc3Type ncType, 
                    Nc3Dim *dim, 
                    const string &units = "");

  /// Add 2-D array meta-data variable
  /// Returns 0 on success, -1 on failure
  
  int addMetaVar(Nc3Var* &var, const string &name, 
                 const string &standardName,
                 const string &longName,
                 Nc3Type ncType, Nc3Dim *dim0, Nc3Dim *dim1, 
                 const string &units = "");
  
  // Add 2-D array meta-data variable
  // Returns var on success, NULL on failure
  
  Nc3Var *addMetaVar(const string &name,
                    const string &standardName,
                    const string &longName,
                    Nc3Type ncType,
                    Nc3Dim *dim0, Nc3Dim *dim1,
                    const string &units = "");

  /// read int variable, set var and val
  /// Returns 0 on success, -1 on failure

  int readIntVar(Nc3Var* &var, const string &name,
                 int &val, int missingVal, bool required = true);
  
  /// read int variable, set val
  /// Returns 0 on success, -1 on failure

  int readIntVal(const string &name, int &val, 
                 int missingVal, bool required = true);
  
  /// read int variable, set val, with units if available
  /// Returns 0 on success, -1 on failure

  int readIntVal(const string &name, int &val, string &units, 
                 int missingVal, bool required = true);
  
  /// read float variable
  /// Returns 0 on success, -1 on failure
  
  int readFloatVar(Nc3Var* &var, const string &name, float &val, 
                   float missingVal, bool required = true);
  
  /// read float value
  /// Returns 0 on success, -1 on failure
  
  int readFloatVal(const string &name, float &val,
                   float missingVal, bool required = true);

  /// read float value, with units if available
  /// Returns 0 on success, -1 on failure
  
  int readFloatVal(const string &name, float &val, string &units,
                   float missingVal, bool required = true);

  /// read double variable
  /// Returns 0 on success, -1 on failure
  
  int readDoubleVar(Nc3Var* &var, const string &name, double &val, 
                    double missingVal, bool required = true);
  
  /// read double value
  /// Returns 0 on success, -1 on failure
  
  int readDoubleVal(const string &name, double &val,
                    double missingVal, bool required = true);

  /// read double value, with units if available
  /// Returns 0 on success, -1 on failure
  
  int readDoubleVal(const string &name, double &val, string &units,
                    double missingVal, bool required = true);

  /// read a scalar string variable
  /// Returns 0 on success, -1 on failure

  int readStringVar(Nc3Var* &var, const string &name, string &val);
  
  /// write a scalar double variable
  /// Returns 0 on success, -1 on failure

  int writeVar(Nc3Var *var, double val);

  /// write a scalar float variable
  /// Returns 0 on success, -1 on failure
  
  int writeVar(Nc3Var *var, float val);

  /// write a scalar int variable
  /// Returns 0 on success, -1 on failure
  
  int writeVar(Nc3Var *var, int val);

  /// write a 1-D vector variable
  /// number of elements specified in dimension
  /// Returns 0 on success, -1 on failure

  int writeVar(Nc3Var *var, const Nc3Dim *dim, const void *data);

  /// write a 1-D vector variable
  /// number of elements specified in arguments
  /// Returns 0 on success, -1 on failure
  
  int writeVar(Nc3Var *var, const Nc3Dim *dim, size_t count, 
               const void *data);
  
  /// write a string variable
  /// Returns 0 on success, -1 on failure

  int writeStringVar(Nc3Var *var, const void *data);
  
  /// compress a variable

  int compressVar(Nc3Var *var, int compressionLevel);

  //@}

  ///////////////////////////////
  /// \name Strings from nc items
  //@{
  
  /// convert type enum to strings

  static string ncTypeToStr(Nc3Type nctype);

  /// get string representation of component

  static string asString(const Nc3TypedComponent *component, int index = 0);
  
  //@}

  ////////////////////////
  /// \name Handles:
  //@{
  
  /// Get the path in use after read or write
  
  string getPathInUse() const { return _pathInUse; }
  
  /// Get the Nc format after a write
  
  Nc3File::FileFormat getNc3FileFormat() const { return _ncFormat; }
  
  /// Get the Nc3File object
  
  Nc3File *getNc3File() { return _ncFile; }

  /// Get the Nc3Error object
  
  Nc3Error *getNc3Error() { return _err; }

  //@}

  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Clear error string.
  
  void clearErrStr() { _errStr.clear(); }

  /// Get the Error String.
  ///
  /// The contents are only meaningful if an error has returned.
  
  string getErrStr() const { return _errStr; }
  
  //@}

protected:

private:

  void clear();

  // error string

  string _errStr; ///< Error string is set on read or write error
  
  // handles
  
  Nc3File *_ncFile;
  string _pathInUse;
  Nc3Error *_err;
  Nc3File::FileFormat _ncFormat;
  
  /// add integer value to error string, with label
  
  void _addErrInt(string label, int iarg,
                  bool cr = true);
  
  /// add double value to error string, with label
  
  void _addErrDbl(string label, double darg,
                  string format, bool cr = true);
  
  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);
  
  // set fill value appropriately for the variable type
  
  void _setMetaFillvalue(Nc3Var *var);

};

#endif
