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
////////////////////////////////////////////////////////////////////
// Mdv/Mdvx.hh
//
// Mdv access class.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
////////////////////////////////////////////////////////////////////

#ifndef Mdvx_hh
#define Mdvx_hh

#define _in_Mdvx_hh

#include <vector>
#include <set>
#include <string>
#include <cstdio>
#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

#include <Mdv/MdvxTimeList.hh>
class MdvxField;
class MdvxChunk;
class MdvxProj;
class MdvxPjg;
class DsMdvxMsg;
class DsMdvServer;
class DsMdvClimoServer;
class WMS2MdvServer;
class Ncf2MdvTrans;
class Ncf2MdvField;
class Mdv2NcfTrans;

class Mdvx
{

  friend class MdvxField;
  friend class MdvxChunk;
  friend class DsMdvServer;
  friend class DsMdvClimoServer;
  friend class DsMdvxMsg;
  friend class WMS2MdvServer;
  friend class Mdv2NcfTrans;
  friend class Ncf2MdvTrans;
  friend class Ncf2MdvField;
  
public:

#include <Mdv/Mdvx_enums.hh>
#include <Mdv/Mdvx_typedefs.hh>
#include <Mdv/Mdvx_constants.hh>

  ///////////////////////
  // default constructor
  
  Mdvx();

  ///////////////////
  // copy constructor
  
  Mdvx(const Mdvx &rhs);

  ///////////////////////
  // destructor

  virtual ~Mdvx();

  // assignment
  
  Mdvx & operator=(const Mdvx &rhs);

  // clear all memory, reset to starting state
  
  virtual void clear();

  // clear field and chunk memory

  void clearFields();
  void clearChunks();

  // add a field
  //
  // The field must have been created using 'new'.
  // The field object becomes 'owned' by the Mdvx object, and will be
  // freed by it.

  void addField(MdvxField *field);

  // delete a field
  //
  // The field to be deleted must already be managed by the
  // Mdvx object. It will be freed.
  //
  // Returns -1 if field does not exist, 0 otherwise.

  int deleteField(MdvxField *toBeDeleted);

  // add a chunk
  //
  // The chunk must have been created using 'new'.
  // The chunk object becomes 'owned' by the Mdvx object, and will be
  // freed by it.
  
  void addChunk(MdvxChunk *chunk);

  // delete a chunk
  //
  // The chunk to be deleted must already be managed by the
  // Mdvx object. It will be freed.
  //
  // Returns -1 if chunk does not exist, 0 otherwise.
  
  int deleteChunk(MdvxChunk *toBeDeleted);
  
  ///////////////////////////
  // clear the master header
  
  void clearMasterHeader();


  //////////////////////////////////////////
  // set master header elements individually

  // valid time is the centroid time
  
  void setValidTime(time_t valid_time);
  
  // generate time for forecast data
  
  void setGenTime(time_t gen_time);
  
  // time of the forecast

  void setForecastTime(time_t forecast_time);

  // forecast lead time in seconds
  
  void setForecastLeadSecs(int lead_secs);
  
  // begin and end time
  
  void setBeginTime(time_t begin_time);
  void setEndTime(time_t end_time);

  // data collection type

  void setDataCollectionType(data_collection_type_t dtype);

  // setting string parts of the master header

  void setDataSetInfo(const char *info);
  void setDataSetName(const char *name);
  void setDataSetSource(const char *source);

  ///////////////////////////////////////////////
  // set the master header using a struct
  // Note: this will overwrite the entire header
  
  void setMasterHeader(const master_header_t &mhdr);

  // update the master header
  // Make the master header consistent with the rest of the object
  
  void updateMasterHeader() const;

  // set the debugging state

  void setDebug(bool debug = true) { _debug = debug; }

  // set the application name

  void setAppName(const string &app_name) { _appName = app_name; }

  // set the heartbeat function

  void setHeartbeatFunction(const heartbeat_t heartbeat_func) { _heartbeatFunc = heartbeat_func; }

  // clear error string
  
  void clearErrStr() const;

  // add messages to error str

  void addToErrStr(const string &message1,
                   const string &message2 = "",
                   const string &message3 = "") const;

  // Is the file a 64-bit version?
  // only valid after a call to checkIs64Bit()

  bool getIs64Bit() const { return _is64Bit; }
  
  ////////////////////////
  // get the formats

  mdv_format_t getInternalFormat() const { return _internalFormat; }
  mdv_format_t getReadFormat() const { return _readFormat; }
  mdv_format_t getWriteFormat() const { return _writeFormat; }

  ////////////////////////
  // get the primary times

  // valid time is the centroid time

  time_t getValidTime() const;

  // generate time for forecast data

  time_t getGenTime() const;

  // time of the forecast

  time_t getForecastTime() const;

  // forecast lead time in seconds
  
  time_t getForecastLeadSecs() const;
  
  // Get references for headers exactly as they appear in the file.
  // Use these only after a call to readAllHeaders().
  
  const master_header_t &getMasterHeaderFile() const;

  const vector<Mdvx::field_header_t> &getFieldHeadersFile() const;
  const vector<Mdvx::vlevel_header_t> &getVlevelHeadersFile() const;
  const vector<Mdvx::chunk_header_t> &getChunkHeadersFile() const;

  // get number of fields and chunks as in file

  size_t getNFieldsFile() const { return (_fhdrsFile.size()); }
  size_t getNChunksFile() const { return (_chdrsFile.size()); }

  // get field, vlevel and chunk headers as in file
  // call geNFieldsFile() or getNChunksFile() first to get available sizes

  const field_header_t &getFieldHeaderFile(const int field_num) const;
  const vlevel_header_t &getVlevelHeaderFile(const int field_num) const;
  const chunk_header_t &getChunkHeaderFile(const int chunk_num) const;
  
  // get master header reference - this is a const reference.
  // If you want to alter the master header, make a copy, alter
  // it and then call setMasterHeader()
  
  const master_header_t &getMasterHeader() const { return (_mhdr); }

  // get the data set info as a string

  string getDataSetInfo() const;

  // get number of fields and number of chunks

  size_t getNFields() const { return (_fields.size()); }
  size_t getNChunks() const { return (_chunks.size()); }

  // get name of specified field
  // Returns name on success, NULL on failure
  
  const char *getFieldName(int field_num) const;
  const char *getFieldNameLong(int field_num) const;

  // get a field number given the field name
  // Returns field number on success,  -1 on failure
  
  int getFieldNum(const char *field_name) const;

  // get pointer to field object, by field num or name.
  // By name searches using both long and short names.
  // Returns NULL on failure.
  
  const vector<MdvxField *> &getFields() const { return _fields; }
  MdvxField *getField(int field_num) const
  { return getFieldByNum(field_num); }
  MdvxField *getFieldByNum(int field_num) const;
  
  MdvxField *getField(const char *field_name) const
  { return getFieldByName(field_name); }
  MdvxField *getFieldByName(const char *field_name) const;
  MdvxField *getFieldByName(const string &field_name) const
  { return (getFieldByName(field_name.c_str())); }

  // get projection, using the metadata from the first field
  // returns PROJ_UNKNOWN if no fields

  projection_type_t getProjection() const;

  // get pointer to chunk object, by num, id or name.
  // If multiple possibilities, returns first one.
  // Returns NULL on failure.

  MdvxChunk *getChunkByNum(const int chunk_num) const;
  MdvxChunk *getChunkByInfo(const char *chunk_info) const;
  MdvxChunk *getChunkById(const int chunk_id) const;

  // access to the read time variables

  read_search_mode_t getReadSearchMode() { return (_readSearchMode); }
  time_t getReadSearchTime() { return (_readSearchTime); }
  int getReadSearchMargin() { return (_readSearchMargin); }
  int getReadForecastLeadTime() { return (_readForecastLeadTime); }

  // access to constrain lead times parameters

  bool getConstrainFcastLeadTimes() const {
    return _timeList.getConstrainFcastLeadTimes();
  }
  int getMinFcastLeadTime() const {
    return _timeList.getMinFcastLeadTime();
  }
  int getMaxFcastLeadTime() const {
    return _timeList.getMaxFcastLeadTime();
  }
  bool getSpecifyFcastByGenTime() const {
    return _timeList.getSpecifyFcastByGenTime();
  }

  // access to vsection geometry

  bool getVsectDisableInterp() { return _vsectDisableInterp; }
  int getNVsectWayPts() const { return (_vsectWayPts.size()); }
  int getNVsectSamplePts() const { return (_vsectSamplePts.size()); }
  int getNVsectSegments() const { return (_vsectSegments.size()); }
  double getVsectDxKm() const { return (_vsectDxKm); }
  double getvsectTotalLength() const { return (_vsectTotalLength); }

  const vsect_waypt_t &getVsectWayPt(const int i) const;
  const vsect_samplept_t &getVsectSamplePt(const int i) const;
  const vsect_segment_t &getVsectSegment(const int i) const;

  const vector<vsect_waypt_t> &getVsectWayPts() const;
  const vector<vsect_samplept_t> &getVsectSamplePts() const;
  const vector<vsect_segment_t> &getVsectSegments() const;

  // access to time list data
  //
  // getTimeList() returns array of times, which holds valid times
  // or gen times, depending on the time list mode.
  //
  // getValidTimes() returns array of valid times.
  // getGenTimes() returns array of generate times.

  const MdvxTimeList &getTimeListObj() const { return _timeList; }

  const vector<time_t> &getTimeList() const {
    return _timeList.getValidTimes();
  }
  const vector<time_t> &getValidTimes() const {
    return _timeList.getValidTimes();
  }
  const vector<time_t> &getGenTimes() const {
    return _timeList.getGenTimes();
  }

  // get flag to indicate data is in forecast format

  bool timeListHasForecasts() const {
    return _timeList.hasForecasts();
  }

  // access to forecast times list
  // holds forecast time array, result of setTimeListModeGenPlusForecast().

  const vector<vector<time_t> > &getForecastTimesArray() const {
    return _timeList.getForecastTimesArray();
  }

  // single buffer

  const MemBuf &getSingleBuf() const { return _singleBuf; }

  // XML header and buffer

  const string &getXmlHdr() const { return _xmlHdr; }
  const MemBuf &getXmlBuf() const { return _xmlBuf; }

  // access to the directory and path strings

  // read dir as set by setReadTime()
  const string &getReadDir() const { return _readDir; }

  // read path as set by setReadPath()
  const string &getReadPath() const { return _readPath; }

  // get writeAsForecast flag
  bool getWriteAsForecast() const { return _writeAsForecast; }

  // get ifForecastWriteAsForecast flag
  bool getIfForecastWriteAsForecast() const {
    return _ifForecastWriteAsForecast;
  }

  // path actually  used for the read or write
  const string &getPathInUse() const { return _pathInUse; }

  // get name of application
  const string &getAppName() const { return _appName; }
  
  // Flag to indicate whether there were data files available
  // to read. This allows you to distinguish between a genuine
  // read error and one which occurred through a lack of data.

  bool getNoFilesFoundOnRead() { return _noFilesFoundOnRead; }

  // return data element size (bytes) for given encoding type

  static int dataElementSize(encoding_type_t encoding_type);

  // Get the Error String. This has contents when an error is returned.

  string getErrStr() const { return _errStr; }

  // check for constant dz in field headers

  static bool dzIsConstant(const field_header_t &fhdr,
			   const vlevel_header_t &vhdr);
  
  // compute the epoch from the valid time
  
  static int computeEpoch(time_t validTime);

  // Check if the path is an XML-based file.
  // Returns true or false
  
  static bool isXmlFile(const string &path);

  // Check if the path is a CF-compliant NetCDF file.
  // Returns true or false

  static bool isNcfFile(const string &path);

  // Check if the path is a Radx file - radial radar data.
  // Returns true or false

  static bool isRadxFile(const string &path);

  // deprecated

  int getNTimesInList() const {
    return (getTimeList().size());
  }
  time_t getTimeFromList(int i) const {
    return getTimeList()[i];
  }

  // public function prototypes by group

#include <Mdv/Mdvx_read.hh>      // read interface
#include <Mdv/Mdvx_write.hh>     // write interface
#include <Mdv/Mdvx_timelist.hh>  // timelist interface
#include <Mdv/Mdvx_print.hh>     // print interface
#include <Mdv/Mdvx_BE.hh>        // byte swapping interface
#include <Mdv/Mdvx_xml.hh>       // XML interface
#include <Mdv/Mdvx_ncf.hh>       // NetCDF CF interface
#include <Mdv/Mdvx_vsect.hh>     // Vertical sections
  
  // debug prints

  static void printVsectWayPtsBuf32(const MemBuf &buf,
                                    ostream &out);
  
  static void printVsectWayPtsBuf64(const MemBuf &buf,
                                    ostream &out);
  
  static void printVsectSamplePtsBuf32(const MemBuf &buf,
                                       ostream &out);
  
  static void printVsectSamplePtsBuf64(const MemBuf &buf,
                                       ostream &out);
  
  static void printVsectSegmentsBuf32(const MemBuf &buf,
                                      ostream &out);
  
  static void printVsectSegmentsBuf64(const MemBuf &buf,
                                      ostream &out);
  
protected:

  // error string
  mutable string _errStr;

  // name of application
  mutable string _appName;

  // debug state
  bool _debug;

  // is the file a 64-bit version?

  bool _is64Bit;

  // heartbeat
  heartbeat_t _heartbeatFunc;
  
  // File headers for inspection.
  // master, field, vlevel and chunk headers exactly as they appear in the
  // file. These are set by calling readAllHeaders().

  master_header_t _mhdrFile;
  vector<field_header_t> _fhdrsFile;
  vector<vlevel_header_t> _vhdrsFile;
  vector<chunk_header_t> _chdrsFile;

  // master header

  master_header_t _mhdr;

  // NOTE: In the master header, the data_set_info string is limited to
  // 512 bytes in length. If the info string exceeds 512 bytes,
  // the data_set_info in the master header is truncated,
  // and the full string is added as a chunk with id CHUNK_DATA_SET_INFO.

  string _dataSetInfo;

  // fields

  vector<MdvxField *> _fields;

  // chunks

  vector<MdvxChunk *> _chunks;

  // format - how is data represented

  mdv_format_t _internalFormat; // how data is stored in mdvx object
  mdv_format_t _readFormat;    // requested format on read
  mutable mdv_format_t _writeFormat;   // requested format on write

  // read request members

  bool _readTimeSet;
  read_search_mode_t _readSearchMode;
  time_t _readSearchTime;
  int _readSearchMargin;
  int _readForecastLeadTime;
  string _readDir;

  bool _readPathSet;
  string _readPath;

  bool _readQualifiersActive;

  bool _readHorizLimitsSet;
  double _readMinLat, _readMinLon, _readMaxLat, _readMaxLon;

  bool _readVlevelLimitsSet;
  double _readMinVlevel, _readMaxVlevel;
  
  bool _readPlaneNumLimitsSet;
  int _readMinPlaneNum, _readMaxPlaneNum;
  
  encoding_type_t _readEncodingType;
  compression_type_t _readCompressionType;
  scaling_type_t _readScalingType;
  double _readScale, _readBias;

  bool _readComposite;
  bool _readFillMissing;

  static const int _defaultMaxVsectSamples = 500;
  int _readNVsectSamples;
  int _readMaxVsectSamples;

  bool _readVsectAsRhi;
  bool _readRhiAsPolar;
  double _readRhiMaxAzError;
  bool _readRhiRespectUserDist;

  vector<int> _readFieldNums;
  vector<string> _readFieldNames;
  vector<int> _readChunkNums;

  bool _readRemapSet;
  coord_t _readRemapCoords;

  bool _readAutoRemap2LatLon;
  
  bool _readDecimate;
  int _readDecimateMaxNxy;

  bool _readSpecifyVlevelType;
  vlevel_type_t _readVlevelType;
  
  bool _readFieldFileHeaders;

  bool _readTimeListAlso;

  bool _readAsSingleBuffer;

  bool _read32BitHeaders;

  // write request members

  bool _writeLdataInfo;
  mutable bool _useExtendedPaths;
  mutable bool _writeAddYearSubdir;
  bool _writeAsForecast; // forces forecast write
  bool _ifForecastWriteAsForecast; /* writes as forecast only if
                                    * data_collection_type is
                                    * EXTRAPOLATED or FORECAST */

  // write path or dir - used if the arg in the write call is NULL

  // Flag to indicate whether there were data files available
  // to read. This allows you to distinguish between a genuine
  // read error and one which occurred through a lack of data.

  bool _noFilesFoundOnRead;

  // path in use - the path being used for reading/writing
  
  mutable string _pathInUse;

  // MemBuf for MDV as a single buffer

  mutable MemBuf _singleBuf;

  // netCDF CF representation

  time_t _ncfValidTime;
  time_t _ncfGenTime;
  time_t _ncfForecastTime;
  int _ncfForecastDelta;
  bool _ncfIsForecast;
  int _ncfEpoch;
  string _ncfFileSuffix;
  mutable MemBuf _ncfBuf;

  //////////////////////////////
  // converting MDV to netCDF CF

  string _ncfInstitution;
  string _ncfReferences;
  string _ncfComment;
  vector<Mdv2NcfFieldTrans> _mdv2NcfTransArray;
  
  mutable bool _ncfCompress;
  mutable int _ncfCompressionLevel;

  nc_file_format_t _ncfFileFormat;
  radial_file_type_t _ncfRadialFileType;

  bool _ncfOutputLatlonArrays;
  bool _ncfOutputMdvAttr;
  bool _ncfOutputMdvChunks;
  bool _ncfOutputStartEndTimes;

  // string and buffer for XML representation

  mutable string _xmlHdr;
  mutable MemBuf _xmlBuf;

  // overwriteValidTime is only set when forecast lead timea are
  // constrained and requestByGenTime is true
  // internal use only
  
  bool _overwriteValidTimeByGenTime;
  time_t _genTimeForOverwrite;
  
  // vertical section state

  bool _vsectDisableInterp;
  vector<vsect_waypt_t> _vsectWayPts;
  vector<vsect_samplept_t> _vsectSamplePts;
  vector<vsect_segment_t> _vsectSegments;
  double _vsectDxKm;
  double _vsectTotalLength;

  // time list
  
  MdvxTimeList _timeList;

  // tmp string for printing

  static const int _printStrLen = 1024;
  static char _printStr[_printStrLen];

  /////////////////////////////////
  // protected function prototypes

  // copy
  
  Mdvx &_copy(const Mdvx &rhs);

  // data set info

  void _setDataSetInfoFromChunks();

  // read
  
  int _computeReadPath();
  
  int _readMasterHeader(master_header_t &mhdr,
                        TaFile &infile);
  
  int _readFieldHeader(const int field_num,
                       field_header_t &fhdr,
                       TaFile &infile);
  
  int _readVlevelHeader(const int field_num,
                        const int first_vlevel_offset,
                        const field_header_t &fhdr,
                        vlevel_header_t &vhdr,
                        TaFile &infile);
  
  int _readChunkHeader(const int chunk_num,
                       const int first_chunk_offset,
                       chunk_header_t &chdr,
                       TaFile &infile);
  
  int _readAllHeadersMdv();

  int _readVolumeMdv(bool fill_missing,
                     bool do_decimate,
                     bool do_final_convert,
                     bool is_vsection = false,
                     double vsection_min_lon = -360.0,
                     double vsection_max_lon = 360.0);
  
  int _readVsectionMdv();
  
  int _convertFormatOnRead(const string &caller);

  int _readRhi(bool respectUserDistance = false);
  int _loadClosestRhi(bool respectUserDistance);
  
  int _compileTimeHeight();

  const MdvxField *_getRequestedField(int request_index);

  // write
  
  int _writeMasterHeader32(TaFile &outfile) const;
  int _writeMasterHeader64(TaFile &outfile) const;
  
  int _writeFieldHeader32(const int field_num,
                          TaFile &outfile) const;
  int _writeFieldHeader64(const int field_num,
                          TaFile &outfile) const;
  
  int _writeVlevelHeader32(const int field_num,
                           TaFile &outfile) const;
  int _writeVlevelHeader64(const int field_num,
                           TaFile &outfile) const;
  
  int _writeChunkHeader32(const int chunk_num,
                          TaFile &outfile) const;
  int _writeChunkHeader64(const int chunk_num,
                          TaFile &outfile) const;

  void _checkEnvBeforeWrite() const;

  void _computeOutputPath(const string &outputDir,
                          string &outputName,
                          string &outputPath,
                          bool &writeAsForecast) const;

  int _writeAsMdv32(const string &outputPath);
  int _writeAsMdv64(const string &outputPath);

  void _doWriteLdataInfo(const string &outputDir,
                         const string &outputPath,
                         const string &dataType);
  bool _getWriteAsForecast();
  
  int _writeBufferToFile(const string &pathStr,
                         size_t len, const void *data) const;

  // print
  
  static const char *_timeStr(const time_t ttime);
  
  // create labelled int for errors
  
  static const char *_labelledInt(const char *label, int num);
  
  // xml
  
  int _writeAsXml(const string &output_path) const;
  
  void _writeToXmlHdr(string &hdr,
                      const string &bufFileName) const;
  
  void _writeToXmlMasterHdr(string &hdr) const;
  
  void _writeToXmlFieldHdr(string &hdr, int fieldNum) const;
  
  void _writeToXmlChunkHdr(string &hdr, int chunkNum) const;
  
  int _readVolumeXml(bool fill_missing,
                     bool do_decimate,
                     bool do_final_convert,
                     bool is_vsection = false,
                     double vsection_min_lon = -360.0,
                     double vsection_max_lon = 360.0);

  int _readXmlToMasterHdr(const string &xml,
                          int &forecast_delta);
  
  int _loadXmlReadFieldNums(const vector<string> &fieldXmls);

  int _readXmlToFieldHeaders(const string &xml,
                             time_t forecast_time,
                             int forecast_delta,
                             field_header_t &fhdr,
                             vlevel_header_t &vhdr);
  
  MdvxField *_readXmlField(const field_header_t &fhdr,
                           const vlevel_header_t &vhdr,
                           TaFile &bufFile);
  
  int _loadXmlReadChunkNums();

  int _readXmlToChunkHeader(const string &xml,
                            chunk_header_t &chdr);
    
  MdvxChunk *_readXmlChunk(const chunk_header_t &chdr,
                           TaFile &bufFile);
  
  static string _xmlProjType2Str(int proj_type);
  static string _xmlVertType2Str(int vert_type);
  static string _xmlProjType2XUnits(int proj_type);
  static string _xmlProjType2YUnits(int proj_type);
  static string _xmlVertTypeZUnits(int vert_type);
  static string _xmlEncodingType2Str(int encoding_type);
  static string _xmlCollectionType2Str(int collection_type);
  static string _xmlCompressionType2Str(int compression_type);
  static string _xmlScalingType2Str(int scaling_type);
  static string _xmlTransformType2Str(int transform_type);
  static string _xmlChunkId2Str(int chunk_id);

  static int _xmlProjType2Int(const string &proj_type);
  static int _xmlVertType2Int(const string &vert_type);
  static int _xmlEncodingType2Int(const string &encoding_type);
  static int _xmlCollectionType2Int(const string &collection_type);
  static int _xmlCompressionType2Int(const string &compression_type);
  static int _xmlScalingType2Int(const string &scaling_type);
  static int _xmlTransformType2Int(const string &transform_type);

  // netcdf
  
  int _readVolumeIntoNcfBuf();
  int _setTimesNcf();
  int _writeNcfBufToFile(const string &output_path) const;

  int _writeAsNcf(const string &output_path) const;
  int _writeToNcfBuf(const string &path);
  int _readFromNcfBuf(const string &path);
  int _readAllHeadersNcf(const string &path);
  int _readNcf(const string &path);
  int _readAllHeadersRadx(const string &path);
  int _readRadx(const string &path);

  // vertical sections

  int _computeNVsectSamples() const;
  void _addVsectChunks32();
  void _addVsectChunks64();
  int _loadVsectInfoFromChunks();

  // copy between 32-bit and 64-bit versions, and vice versa
  
  static void _copyMasterHeader32to64(const master_header_32_t &mhdr32,
                                      master_header_64_t &mhdr64);
  
  static void _copyMasterHeader64to32(const master_header_64_t &mhdr64,
                                      master_header_32_t &mhdr32);
  
  static void _copyFieldHeader32to64(const field_header_32_t &fhdr32,
                                     field_header_64_t &fhdr64);
  
  static void _copyFieldHeader64to32(const field_header_64_t &fhdr64,
                                     field_header_32_t &fhdr32);
  
  static void _copyVlevelHeader32to64(const vlevel_header_32_t &vhdr32,
                                      vlevel_header_64_t &vhdr64);
  
  static void _copyVlevelHeader64to32(const vlevel_header_64_t &vhdr64,
                                      vlevel_header_32_t &vhdr32);
  
  static void _copyChunkHeader32to64(const chunk_header_32_t &chdr32,
                                     chunk_header_64_t &chdr64);
  
  static void _copyChunkHeader64to32(const chunk_header_64_t &chdr64,
                                     chunk_header_32_t &chdr32);

  // copy the main headers to the file headers
  // useful for non-MDV reads, e.g. NetCDF

  void _copyMainHeadersToFileHeaders();

private:

};

#undef _in_Mdvx_hh

#endif


