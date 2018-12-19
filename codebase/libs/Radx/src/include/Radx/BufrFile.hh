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


class DNode {

public: 
  enum DNodeDataType {INT, FLOAT, STRING, OTHER};

    unsigned short des;
    DNode *children;
    DNodeDataType dataType;    
    string somejunksvalue;
    int ivalue;  // currently storing the number of repeats here
    float fvalue;
    unsigned short delayed_repeater;
    DNode *next;


  DNode();

  ~DNode();
};

class BufrFile

{
  
public:

  /// Constructor
  
  BufrFile();


  
  /// Destructor
  
  virtual ~BufrFile();


  void setDebug(bool state);

  /// Set verbose debugging on/off.
  ///
  void setVerbose(bool state); 
  void setVeryVerbose(bool state); 

  void setTablePath(char *path);

  //////////////////////////////////////////////////////////////
  /// \name File operations
  //@{

  /// read the data for the field
  /// 
  void readThatField(string fileName,
             string filePath,
             time_t fileTime,
             string fieldName,
             string standardName,
             string longName,
             string units);

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
  int readSection1_edition2();
  int readSection1_edition3();
  int readSection1_edition4();
  int readDataDescriptors();
  int readDescriptorTables();
  int readData();
  int readSection5();
  bool isEndInSight();

  bool matches_204_31_X(vector<unsigned short> &descriptors);
  bool matches_gsi(vector<unsigned short> &descriptors);
  bool _isGsi;

  int print(ostream &out, bool printRays, bool printData);
  void printSection0(ostream &out);
  void printSection1(ostream &out);
  void printSection2(ostream &out);
  void printSection3(ostream &out);
  void printSection4(ostream &out);

  /// close previously-opened file

  void close();

  bool eof();

  size_t getNumberOfSweeps();
  size_t getTimeDimension();
  // size_t getMaxRangeDimension();

  size_t getNBinsAlongTheRadial(int sweepNumber);
  double getRangeBinOffsetMeters();
  double getRangeBinSizeMeters(int sweepNumber);

  double getElevationForSweep(int sweepNumber);
  int getNAzimuthsForSweep(int sweepNumber);
  double getStartingAzimuthForSweep(int sweepNumber);

  double getStartTimeForSweep(int sweepNumber);
  double getEndTimeForSweep(int sweepNumber);

  time_t getEndUTime(int sweepNumber);
  time_t getStartUTime(int sweepNumber);

  float *getDataForSweepFl32(int sweepNumber);
  double *getDataForSweepFl64(int sweepNumber);
  string getTypeOfProductForSweep(int sweepNumber);

  double getLatitude() { return currentTemplate->getLatitude(); }
  double getLongitude() { return currentTemplate->getLongitude(); }
  double getHeight() { return currentTemplate->getHeight(); }
  int getWMOBlockNumber() { return currentTemplate->getWMOBlockNumber(); }
  int getWMOStationNumber() { return currentTemplate->getWMOStationNumber(); }

  int getHdrYear() { return hdr_year; }
  int getHdrMonth() { return hdr_month; }
  int getHdrDay() { return hdr_day; }
  int getHour() { return hour; }
  int getMinute() { return minute; }

  string getTypeOfStationId() { return typeOfStationId; }
  string getStationId() { return stationId; }

private:

  //double latitude;
  //double longitude;
  //double height;
  int hdr_year;
  int hdr_month;
  int hdr_day;
  int hour;
  int minute;

  //int WMOBlockNumber;
  //int WMOStationNumber;
  string typeOfStationId;
  string stationId;


  //bool StuffIt(string fieldName, double value);
  int _getCurrentBytePositionInFile();
  Radx::ui32 ExtractIt(unsigned int nBits);
  string ExtractText(unsigned int nBits);
  double fastPow10(int n);
  string _trim(const std::string& str,
	       const std::string& whitespace = " \t");
  Radx::ui32 Apply(TableMapElement f);
  //Radx::si32 ApplyNumeric(TableMapElement f);
  Radx::fl32 ApplyNumericFloat(TableMapElement f);
  //  int TraverseOriginal(vector<unsigned short> descriptors);
  int TraverseNew(vector<unsigned short> descriptors);
  //int Traverse(int start, int length); //vector<unsigned short> descriptors);
  int ReplenishBuffer();
  bool NextBit();
  void MoveToNextByteBoundary();

  BufrProduct *currentTemplate;
  char *_tablePath;
  int _addBitsToDataWidth;
  int _addBitsToDataScale;
  int _multiplyFactorForReferenceValue;

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
  Radx::ui32 _nBitsRead;
  bool inSection5;
  /*
  enum DNodeDataType {INT, FLOAT, STRING, OTHER};

  typedef struct node {
    unsigned short des;
    struct node *children;
    DNodeDataType dataType;    
    string somejunksvalue;
    int ivalue;  // currently storing the number of repeats here
    float fvalue;
    unsigned short delayed_repeater;
    struct node *next;
  } DNode;
  */

  string _tempStringValue;

  DNode *GTree;
  int _descend(DNode *tree);
  DNode* buildTree(vector<unsigned short> descriptors, bool reverse);
  void _deleteAfter(DNode *p);
  int moveChildren(DNode *parent, int howManySiblings);
  void printTree(DNode *tree, int level);
  void prettyPrint(ostream &out, DNode *p, int level);
  void prettyPrintLeaf(ostream &out, DNode *p, TableMapElement &element, int level);
  void prettyPrintNode(ostream &out, DNode *p, int level);
  void prettyPrintReplicator(ostream &out, DNode *p, int level);
  void prettyPrintTree(ostream &out, DNode *tree, int level);
  void printHeader();
  void freeTree(DNode *tree);

  // subfunctions of _descend 

  void _visitTableBNode(DNode *p, bool *compressionStart);
  void _visitTableCNode(DNode *p);
  void _visitVariableRepeater(DNode *p, unsigned char x);
  void _visitFixedRepeater(DNode *p,  unsigned char x, unsigned char y);
  void _visitTableDNode(DNode *p);
  void _visitReplicatorNode(DNode *p);
  void _verbosePrintTree(DNode *tree);
  void _verbosePrintNode(unsigned short des);


  int prettyPrintLevel;

#define  MAX_BUFFER_SIZE_BYTES  2048

  //  static const int MAX_BUFFER_SIZE_BYTES = 2048; // 4096; //  2048; // 5; //1024; // 30;
  unsigned char _dataBuffer[MAX_BUFFER_SIZE_BYTES];
  int currentBufferLengthBits;
  int currentBufferLengthBytes;
  int currentBufferIndexBits;
  int nOctetsRead;
  //int currentBufferIndexBytes;

  int _bufrMessageCount;

  std::vector<unsigned short> _descriptorsToProcess;

  TableMap tableMap;

  bool _debug;
  bool _verbose;
  bool _very_verbose;
  string _fieldName;
  
  /// Get the path in use after read or write
  
  string getPathInUse() const { return _pathInUse; }
  
  
  //@}

  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Clear error string.
  
  void clearErrStr() { _errString.clear(); }

  /// Get the Error String.
  ///
  /// The contents are only meaningful if an error has returned.
  
  string getErrStr() const { return _errString; }
  
  //@}
  
protected:

private:

  void clear();
  void clearForNextMessage();

  // error string

  string _errString; ///< Error string is set on read or write error
  
  // handles
  
  FILE *_file;
  string _pathInUse;
  bool _firstBufferReplenish;

  // Error strings accumulate information and then 
  // thrown as exceptions.
  
  /// add integer value to error string, with label
  
  void _addErrInt(string label, int iarg,
                  bool cr = true);
  
  /// add double value to error string, with label
  
  void _addErrDbl(string label, double darg,
                  string format, bool cr = true);
  
  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);

};
#endif
