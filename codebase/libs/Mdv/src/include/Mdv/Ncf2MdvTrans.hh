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
// Ncf2MdvTrans.hh
//
// Sue Dettling, Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008 
//
///////////////////////////////////////////////////////////////
//
// Ncf2MdvTrans class.
// Translate a CF NetCDF file to Mdvx object
//
///////////////////////////////////////////////////////////////////////

#ifndef NCF2MDV_TRANS_HH
#define NCF2MDV_TRANS_HH

#include <Mdv/Mdvx.hh>
#include <Radx/Radx.hh>
#include <Ncxx/Nc3xFile.hh>
#include <set>

using namespace std;
class RadxVol;
class RadxSweep;
class RadxField;

////////////////////////
// 
// Ncf2MdvTrans object extracts data from a CF-compliant NetCDF file,
// and populates an Mdvx object with the data from the file.

class Ncf2MdvTrans {
  
public:

  //
  // constructor
  //
  Ncf2MdvTrans();
  
  //
  // destructor
  //
  ~Ncf2MdvTrans();
 
  // set global attributes

  void setDebug(bool debug) { _debug = debug; }

  //
  // determine time dimension of netCDF data by reading data file
  // path = name of netCDF file
  // Returns number of times, or -1 for no consistent times dimension or error
  // 
  int inspectTimes(const string &path, Mdvx &mdv);
  
  //
  // Set the time dimension size in the local state to the input value
  // This implies that the times have been inspected and this input is the
  // size of the time dimension
  //

  inline void setExpectedNumTimes(int numTimes)
  {
    _timesInspected = true;
    _expectedNumTimes = numTimes;
  }

  //
  // Set a local value for time index, typically done in concert with 
  // a successful call to inspectTimes() and setExpectedNumTimes()

  inline void setTimeIndex(int index)
  {
    _timeIndex = index;
  }

  // set flag on whether to read in data or not

  void setReadData(bool val) { _readData = val; }

  // read NetCDF CF file, populating Mdvx object
  // returns 0 on success, -1 on failure
  //
  // NOTE: If inspectTimes()/setExpectedNumTimes()/setTimeIndex() 
  // are not being used, and the netCDF data has multiple times,
  // the fields in the Mdvx object will include data for all
  // of the times, and num_data_times in the master
  // header will be set accordingly
  //
  // If inspectTimes()/setExpectedNumTimes()/setTimeIndex() is being used,
  // or the netCDF data is not forecast data, the Mdvx object will have
  // data one time, with num_data_times set to 1
  // 

  int readCf(const string &path, Mdvx &mdv);

  // read Radx file and translate into MDV
  // returns 0 on success, -1 on failure
  
  int readRadx(const string &path, Mdvx &mdv);
  
  // perform translation from RadxVol object into MDV
  
  int translateRadxVol2Mdv(const string &path, RadxVol &vol, Mdvx &mdv);

  // finalization steps in normal circumstances, made public because used
  // outside the lib

  void finalizeNormal(void);

  // clear error string
  
  void clearErrStr();

  // Get the Error String. This has contents when an error is returned.
  
  string getErrStr() const { return _errStr; }

  // get string from component
  
  static string asString(const Nc3TypedComponent *component, int index = 0);

protected:  

  bool _debug;

  // netCDF file

  Nc3File *_ncFile;
  Nc3Error *_ncErr;

  // MDV objects

  Mdvx *_mdv;
  Mdvx::master_header_t _mhdr;
  string _dataSetInfo;

  // time dimensions
  // there may be multiple time dimensions in the file
  // each field could have a different time dimension and values, but that is
  // somewhat pathological

  class TimeDim {
  public:
    string name;          // field name
    Nc3Dim *dim;          // pointer to dimension
    Nc3Var *var;          // pointer to the variable
    vector<time_t> times; // the time values in the time dimension

    TimeDim() {
      dim = NULL;
      var = NULL;
    }
  };
  vector<TimeDim> _timeDims;

  // The time dimension with the least number of times

  TimeDim _timeDim;

  // set of valid times in data set

  set<time_t> _validTimes;

  // flag to indicate actually read in the data

  bool _readData;

  // flag to indicate that a forecast reference time was found

  bool _forecast_reference_time_found;

  // flag set true if times have been inspected and a consistent time dimension
  // was found

  bool _timesInspected;  

  // a moving index within the time dimension, only when _timesInspected=true

  int _timeIndex;

  // The time dimension size, only when _timesInspected=true

  int _expectedNumTimes;

  // array dimensions
  //
  // this is filled in for each field to figure out if a field is a grid
  // or not.
  class ArrayDim {
  public:
    Nc3Dim *xDim;
    Nc3Dim *yDim;
    Nc3Dim *zDim;
    Nc3Var *xVar;
    Nc3Var *yVar;
    Nc3Var *zVar;

    ArrayDim() {
      xDim = NULL;
      yDim = NULL;
      zDim = NULL;
      xVar = NULL;
      yVar = NULL;
      zVar = NULL;
    }

    bool isGrid(void) const
    {
      return (xVar != NULL && yVar != NULL);
    }
  };

  // CfRadial support
  
  bool _isRhi;
  int _nAngles;
  double _minAngle;
  double _deltaAngle;
  double _minAngleRes;
  double _maxAngleRes;
  int _halfGap;

  typedef struct {
    int startIaz;
    int endIaz;
  } az_sector_t;

  bool _trimToSector;
  int _dataStartIaz;
  int _dataEndIaz;
  int _nAzTrimmed;

  // error string

  string _errStr;

  /////////////////////////////////
  // Initialization step before reading, return 0 for good, -1 for error
  
  int _initializeForRead(const string &path, Mdvx &mdv);

  // clear memory

  void _clear();

  // init Mdvx object

  void _initMdv(const string &path, Mdvx &mdv);

  /// Parse Nc data

  int _parseNc();

  /// open netcdf file
  /// create error object so we can handle errors
  /// Returns 0 on success, -1 on failure

  int _openNcFile(const string &path);

  /// close netcdf file if open
  /// remove error object if it exists
  
  void _closeNcFile();

  // set values in master header

  int _setMasterHeader();

  // set time information from Ncf file
  // return 0 for good, -1 for error

  int _setTimeInfo();

  // set time information if found in one variable

  void _setTimeInfoForVar(Nc3Var *var);

  // search for data fields with time dimension, hoping for exactly one
  // consistent result. returns time dimension, or -1 for inconsistent or
  // error

  int _matchTimeInfoToData();

  // add the data fields, return 0 for good, -1 for error
  
  int _addDataFields();

  //  Add one field to mdv state, return 0 for good, -1 for bad
  
  int _addOneField(Nc3Var *dataVar, int &fieldNum);


  // adjust nTimes for a field return -1 for bad, 0 for good

  int _adjustTimeInfo(Nc3Var *dataVar, int &fieldNum, vector<int> &nTimes);

  // Initialization for adding one field, check if it is a grid
  // return 0 if field should be added, -1 if not

  int _addOneFieldInit(Nc3Var *dataVar, int &fieldNum, ArrayDim &arrayDim);

  // Look a a field and adjust arrayDim if appropriate

  void _inspectDim(Nc3Dim *dim, int jdim, ArrayDim &arrayDim);

  // Look a a field in another way and adjust arrayDim if appropriate

  void _reInspectDim(Nc3Dim *dim, int jdim, ArrayDim &arrayDim);

  //////////////////////////
  // Check if variable is a field that should be added
  //
  // returns true if yes.

  bool _shouldAddField(Nc3Var *dataVar, int &fieldNum);


  // find the time dimension for this variable
  // if set, it will be the first dimension
  TimeDim * _findTimeDim(Nc3Var *dataVar);


  //////////////////////////
  // Add infor for one field at one time to _mdv object
  // return 0 for good, -1 for error

  int _addOneTimeDataField(int itime, TimeDim *tdim, 
			   Nc3Var* dataVar, ArrayDim &arrayDim);

  // finalize fields
  //   (a) convert to vert section as needed
  //   (b) compress appropriately

  int _finalizeFields();
  int _finalizeFieldsRhi(bool respectUserDistance = false);

  // add chunks
  // returns 0 on success, -1 on failure

  int _addChunks();

  // add chunk for global attributes in XML
  
  void _addGlobalAttrXmlChunk();
  
  // create XML for global attributes

  string _getGlobalAttrXml();

  // add attribute to string
  
  void _addAttr2Str(Nc3Att *att, const string &requiredName,
                    string &str, const string &label);
  
  // set vals from attribute

  void _setSi32FromAttr(Nc3Att *att, const string &requiredName, si32 &val);
  void _setFl32FromAttr(Nc3Att *att, const string &requiredName, fl32 &val);
  void _setSi64FromAttr(Nc3Att *att, const string &requiredName, si64 &val);
  void _setFl64FromAttr(Nc3Att *att, const string &requiredName, fl64 &val);

  // translate CfRadial using Radx

  int _translateRadxVol2Mdv(const string &path, RadxVol &vol);
  void _computeAngRes(const RadxVol &vol);
  void _setMasterHeaderCfRadial(const string &path, const RadxVol &vol);
  void _addFieldCfRadial(const RadxVol &vol, const RadxField &radxField,
                         int origByteWidth);
  void _loadRayLookup(const RadxVol &vol,
                      const RadxSweep &sweep,
                      int *rayLut);
  void _addRadarParamsCfRadial(const RadxVol &vol);
  void _addCalibCfRadial(const RadxVol &vol);
  void _addElevArrayCfRadial(const RadxVol &vol);
  void _findEmptySectors(const RadxVol &vol);

  int _getDsRadarType(Radx::PlatformType_t ptype);
  int _getDsScanMode(Radx::SweepMode_t mode);
  int _getDsFollowMode(Radx::FollowMode_t mode);
  int _getDsPolarizationMode(Radx::PolarizationMode_t mode);
  int _getDsPrfMode(Radx::PrtMode_t mode);

private:

};

#endif
