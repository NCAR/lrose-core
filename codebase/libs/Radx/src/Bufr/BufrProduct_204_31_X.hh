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
// BufrProduct.hh
//
// BUFR Product
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2017
//
///////////////////////////////////////////////////////////////

#ifndef BufrProduct_204_31_X_HH
#define BufrProduct_204_31_X_HH

#include <Radx/BufrProduct.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxTime.hh>
#include <cstdlib>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////
/// BASE CLASS FOR BUFR DATA ACCESS

class BufrProduct_204_31_X : public BufrProduct
{
  
public:

  /// Constructor
  
  BufrProduct_204_31_X();
  
  /// Destructor
  
  virtual ~BufrProduct_204_31_X();

  void reset();

  //  bool StuffIt(unsigned short des, string fieldName, string &value);

  /*
  //////////////////////////////////////////////////////////////
  /// \name Debugging:
  //@{
  
  /// Set normal debugging on/off.
  ///
  /// If set on, basic debugging messages will be printed to stderr
  /// during file operations.

  void setDebug(bool state) { _debug = state; }

  /// Set verbose debugging on/off.
  ///
  /// If set on, verbose debugging messages will be printed to stderr
  /// during file operations.

  void setVerbose(bool state) {
    _verbose = state;
    if (_verbose) _debug = true;
  }

  //@}

  void allocateSpace(unsigned int n);

  void addData(unsigned char value);

  double *decompressData();
  float *decompressDataFl32();

  void createSweep();

  void putYear(double value);
  void putMonth(double value);
  void putDay(double value);
  void putHour(double value);
  void putMinute(double value);
  void putSecond(double value);

  void printSweepData(ostream &out);

  bool _debug;
  bool _verbose;

  typedef enum {rawData, other} ProductType;
  enum DataType {CM, TV, DBZH, VRAD, TH, WRAD,  KDP, PHIDP, RHOHV,
    OTHER};

  vector<RadxTime *> timeStampStack;

  RadxBuf compressedData; // dataBuffer;

  unsigned char *dataBuffer; // *compressedData; // [2*65]; // TODO: fix this
  unsigned int nData;
  int nDataSegments;
  unsigned int maxData;
  unsigned int totalData;

  string typeOfProduct;
  RadxTime startTime;
  RadxTime endTime;


  typedef struct {
    string typeOfProduct;
    float *data;
  } ParameterData;

  typedef struct {
    string typeOfProduct;
    double *data;
  } ParameterDataFl64;

  typedef struct {
    RadxTime *startTime;
    RadxTime *endTime;
    double    antennaElevationDegrees;
    size_t    nBinsAlongTheRadial;
    double    rangeBinSizeMeters;
    double    rangeBinOffsetMeters;
    size_t    nAzimuths;
    double    antennaBeamAzimuthDegrees;
    vector<ParameterData> parameterData;
    vector<ParameterDataFl64> parameterDataFl64;

  } SweepData;

  vector<SweepData> sweepData;

  // just accumulate data for one sweep and the metadata for it, then
  // dump it into a RadxVol structure

  vector<unsigned int> replicators;
  void trashReplicator();
  void storeReplicator(unsigned int value);


  // current position information
  int currentSweepNumber;  // set at 3;21;203 -> 1;12;0 -> 0;31;1
  int currentParamNumber;
  //int currentDataSection;  ?? just use add?
  //int currentDataValue; ??? just use add?

  int currentState;

  ProductType productType;
  DataType    dataType;

  //size_t getMaxBinsAlongTheRadial();

  void setNBinsAlongTheRadial(size_t nBins);
  size_t getNBinsAlongTheRadial(int sweepNumber);

  void setAntennaElevationDegrees(double value);
  double getAntennaElevationDegrees(int sweepNumber);

  void setRangeBinSizeMeters(double value);
  double getRangeBinSizeMeters(int sweepNumber);

  void setRangeBinOffsetMeters(double value);
  double getRangeBinOffsetMeters();

  void setNAzimuths(size_t value);
  size_t getNAzimuths();

  void setAntennaBeamAzimuthDegrees(double value);
  double getAntennaBeamAzimuthDegrees(int sweepNumber);

private:

  // all of these could potentially change, so lock them down
  size_t nBinsAlongTheRadial;
  double antennaElevationDegrees;
  double rangeBinSizeMeters;
  double rangeBinOffsetMeters;
  size_t nAzimuths;
  double antennaBeamAzimuthDegrees;

  size_t _maxBinsAlongTheRadial;
  */
};
#endif

