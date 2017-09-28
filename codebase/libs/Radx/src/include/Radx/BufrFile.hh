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
// BufrFile.hh
//
// BUFR file wrapper
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2017
//
///////////////////////////////////////////////////////////////

#ifndef BufrFile_HH
#define BufrFile_HH

#include <algorithm>
#include <string>
#include <vector>
#include <queue>
#include <cstdio>
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>
#include <Radx/TableMapElement.hh>
#include <Radx/TableMap.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/BufrProduct.hh>

using namespace std;

///////////////////////////////////////////////////////////////
/// CLASS FOR BUFR IO OPERATIONS

class BufrFile

{
  
public:

  /// Constructor
  
  BufrFile();
  
  /// Destructor
  
  virtual ~BufrFile();

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
  
  // int openWrite(const string &path, Nc3File::FileFormat format);

  int readSection0();
  int readSection1();
  int readDataDescriptors();
  int readDescriptorTables();
  int readData();

  int Print();

  /// close previously-opened file

  void close();

  double latitude;
  double longitude;
  double height;
  int year;
  int month;
  int day;
  int hour;
  int minute;

  int WMOBlockNumber;
  int WMOStationNumber;
  string typeOfStationId;
  string stationId;

  int getNumberOfSweeps();
  int getTimeDimension();
  int getRangeDimension();

  int getNBinsAlongTheRadial();
  double getRangeBinOffsetMeters();
  double getRangeBinSizeMeters();

  double getElevationForSweep(int sweepNumber);
  int getNAzimuthsForSweep(int sweepNumber);
  double getStartingAzimuthForSweep(int sweepNumber);

  double getStartTimeForSweep(int sweepNumber);
  double getEndTimeForSweep(int sweepNumber);

  double *getDataForSweep(int sweepNumber);
  string getTypeOfProductForSweep(int sweepNumber);

private:

  bool StuffIt(string fieldName, double value);
  Radx::ui32 ExtractIt(int nBits);
  string ExtractText(int nBits);
  void SkipIt(int nBits);
  double fastPow10(int n);
  Radx::ui32 Apply(TableMapElement f);
  Radx::si32 ApplyNumeric(TableMapElement f);
  Radx::fl32 ApplyNumericFloat(TableMapElement f);
  //  int TraverseOriginal(vector<unsigned short> descriptors);
  int TraverseNew(vector<unsigned short> descriptors);
  //int Traverse(int start, int length); //vector<unsigned short> descriptors);
  int ReplenishBuffer();
  bool NextBit();


  // Product90_243 product90_243;
  BufrProduct currentProduct;
  //vector<int> repeaters;


  //@}

  // TODO: see how offsets are handled in Dorade???
  typedef enum {
  
    YEAR = 12,
    MONTH = 14,
    DAY = 15,
    HOUR = 16,
    MINUTE = 17,
    SECOND = 18
  } Bufr_4_offset;

  typedef enum {
    TABLE_B = 0,
    REPLICATOR = 1,
    TABLE_C = 2,
    TABLE_D = 3
  } Type_of_descriptor;

  
  typedef struct {
    //Type_of_descriptor f;
    Radx::ui08 f;
    Radx::ui08 x;
    Radx::ui08 y;
  } DescriptorKey;
  
  typedef struct {
    Radx::ui08 edition;
    Radx::ui32 nBytes;
  } Section0;

  typedef struct {
    bool hasSection2;
    Radx::ui08 masterTable;
    Radx::ui16 generatingCenter;
    Radx::ui16 originatingSubcenter;
    Radx::ui08 updateSequenceNumber;
    Radx::ui08 dataCategoryType;
    Radx::ui08 dataCategorySubtype;
    Radx::ui08 masterTableVersionNumber;
    Radx::ui08 localTableVersionNumber;
    Radx::ui16 year;
    Radx::ui08 month;
    Radx::ui08 day;
    Radx::ui08 hour;
    Radx::ui08 minute;
    Radx::ui08 seconds;
  } Section1;


  Section0 _s0;
  Section1 _s1; 
  Radx::ui32 _numBytesRead;

  typedef struct node {
    unsigned short des;
    struct node *children;
    //Radx::ui32 nRepeats;
    unsigned short delayed_repeater;
    struct node *next;
  } DNode;

  DNode *GTree;
  int _descend(DNode *tree);
  DNode* buildTree(vector<unsigned short> descriptors, bool reverse);
  void _deleteAfter(DNode *p);
  int moveChildren(DNode *parent, int howManySiblings);
  void printTree(DNode *tree, int level);

#define  MAX_BUFFER_SIZE_BYTES  2048

  //  static const int MAX_BUFFER_SIZE_BYTES = 2048; // 4096; //  2048; // 5; //1024; // 30;
  unsigned char _dataBuffer[MAX_BUFFER_SIZE_BYTES];
  int currentBufferLengthBits;
  int currentBufferLengthBytes;
  int currentBufferIndexBits;
  //int currentBufferIndexBytes;

  std::vector<unsigned short> _descriptorsToProcess;

  TableMap tableMap;

  bool _debug = false;
  

  //pushToRadxVol(double *realData, BufrProduct currentProduct);
  
/*
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
  
  /// read float variable
  /// Returns 0 on success, -1 on failure
  
  int readFloatVar(Nc3Var* &var, const string &name, float &val, 
                   float missingVal, bool required = true);
  
  /// read float value
  /// Returns 0 on success, -1 on failure
  
  int readFloatVal(const string &name, float &val,
                   float missingVal, bool required = true);

  /// read double variable
  /// Returns 0 on success, -1 on failure
  
  int readDoubleVar(Nc3Var* &var, const string &name, double &val, 
                    double missingVal, bool required = true);
  
  /// read double value
  /// Returns 0 on success, -1 on failure
  
  int readDoubleVal(const string &name, double &val,
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
  */
protected:

private:

  void clear();

  // error string

  string _errString; ///< Error string is set on read or write error
  
  // handles
  
  FILE *_file;
  string _pathInUse;
  bool _firstBufferReplenish;


  //Nc3Error *_err;
  //Nc3File::FileFormat _ncFormat;
  /* 
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
  */

};
#endif
