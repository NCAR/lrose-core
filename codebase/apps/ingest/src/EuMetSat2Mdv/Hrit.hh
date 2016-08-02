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
// Hrit.hh
//
// Hrit object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
///////////////////////////////////////////////////////////////

#ifndef Hrit_H
#define Hrit_H

#include <dataport/port_types.h>
#include <eumetsat/CxRITFileCompressed.h>
#include <eumetsat/CxRITFileDecompressed.h>
#include <string>
#include <vector>
#include <iostream>
#include "Params.hh"
#include "ChannelSet.hh"

using namespace std;

class Hrit {
  
public:

  typedef enum {
    IMAGE_TYPE = 0,
    GTS_TYPE = 1,
    ALPHA_NUM_TYPE = 2,
    ENCR_KEY_TYPE = 3,
    PROLOGUE = 128,
    EPILOGUE = 129
  } file_type_code_t;

  typedef struct {
    int line_num;
    time_t time;
    int msecs;
    int validity;
    int radiometric_quality;
    int geometric_quality;
  } line_quality_t;

  // constructor
  
  Hrit (const string &prog_name,
        const Params &params,
        ChannelSet &channelSet);

  // destructor
  
  ~Hrit();

  // read in a file
  //
  // returns 0 on success, -1 on failure

  int read(const char *input_path);

  // get methods

  file_type_code_t getFileTypeCode() const { return _fileTypeCode; }
  int getTotalHeaderLength() const { return _totalHeaderLength; }
  ui64 getDataFieldLengthBits() const { return _dataFieldLengthBits; }
  ui64 getDataFieldLengthBytes() const { return _dataFieldLengthBytes; }
  
  int getBitsPerPixel() const { return _bitsPerPixel; }
  int getNCols() const { return _nCols; }
  int getNLines() const { return _nLines; }
  int getCompressionFlag() const { return _compressionFlag; }
  
  const char *getProjName() const { return _projName; }
  double getGeosSubLon() const { return _geosSubLon; }
  int getColScalingFactor() const { return _colScalingFactor; }
  int getLineScalingFactor() const { return _lineScalingFactor; }
  int getColOffset() const { return _colOffset; }
  int getLineOffset() const { return _lineOffset; }

  time_t getDataTime() const { return _dataTime; }

  const string &getDataDef() const { return _dataDef; }

  const string &getAnnotation() const { return _annotation; }
  const string &getXritChannelId() const { return _xritChannelId; }
  const string &getAnnotVersion() const { return _annotVersion; }
  const string &getAnnotDisseminator() const { return _annotDisseminator; }
  const string &getAnnotSatName() const { return _annotSatName; }
  const string &getAnnotChannel() const { return _annotChannel; }
  const string &getAnnotSegment() const { return _annotSegment; }
  const string &getAnnotTimeStr() const { return _annotTimeStr; }
  time_t getAnnotTime() const { return _annotTime; }

  const string &getAncillaryText() const { return _ancillaryText; }
  const string &getImageCompensation() const { return _imageCompensation; }
  const string &getObsTime() const { return _obsTime; }
  const string &getImageQuality() const { return _imageQuality; }

  int getSpectralChannelId() const { return _spectralChannelId; }
  int getSegmentSeqNum() const { return _segmentSeqNum; }
  int getPlannedStartSegmentSeqNum() const {
    return _plannedStartSegmentSeqNum;
  }
  int getPlannedEndSegmentSeqNum() const { return _plannedEndSegmentSeqNum; }
  int getDataRepresentation() const { return _dataRepresentation; }

  int getKeyNumber() const { return _keyNumber; }
  int getKeySeed() const { return _keySeed; }

  int getGpScId() const { return _gpScId; }

  const vector<line_quality_t> &getLineQuality() const { return _lineQuality; }

  const ui16 *getData() const { return _shortData; }

  // print information on an HRIT file object

  void printFileInfo(const DISE::CxRITFile &hritFile, ostream &out);

  // tokenize a string into sub-strings

  static void tokenize(const string &str, const string &spacer, vector<string> &toks);
  
protected:
  
private:

  const string &_progName;
  const Params &_params;
  ChannelSet &_channelSet;

  file_type_code_t _fileTypeCode;
  int _totalHeaderLength;
  ui64 _dataFieldLengthBits;
  ui64 _dataFieldLengthBytes;

  bool _imageStructureHeaderFound;
  bool _navHeaderFound;
  bool _imageDataHeaderFound;
  bool _annotationHeaderFound;
  bool _timeHeaderFound;
  bool _ancillaryTextHeaderFound;
  bool _keyHeaderFound;
  bool _segmentIdHeaderFound;
  bool _lineQualityHeaderFound;

  int _bitsPerPixel;
  int _nCols;
  int _nLines;
  int _nPixels;
  int _compressionFlag;
  
  char _projName[32];
  double _geosSubLon;
  int _colScalingFactor;
  int _lineScalingFactor;
  int _colOffset;
  int _lineOffset;

  time_t _dataTime;

  string _dataDef;

  string _annotation;
  string _xritChannelId;
  string _annotVersion;
  string _annotDisseminator;
  string _annotSatName;
  string _annotChannel;
  string _annotSegment;
  string _annotTimeStr;
  time_t _annotTime;

  string _ancillaryText;
  string _imageCompensation;
  string _obsTime;
  string _imageQuality;

  int _keyNumber;
  int _keySeed;

  int _gpScId;
  int _spectralChannelId;
  int _segmentSeqNum;
  int _plannedStartSegmentSeqNum;
  int _plannedEndSegmentSeqNum;
  int _dataRepresentation;

  vector<line_quality_t> _lineQuality;

  ui08 *_bitData;
  ui16 *_shortData;

  int _loadPrimaryHeader(const ui08 *inBuf, int bufLen);
  int _loadNextHeader(const ui08 *inBuf, int bufLen, int pos, int &len);
  int _loadImageStructureHeader(const ui08 *hdr, int len);
  int _loadNavHeader(const ui08 *hdr, int len);
  int _loadImageDataHeader(const ui08 *hdr, int len);
  int _loadAnnotationHeader(const ui08 *hdr, int len);
  int _loadTimeHeader(const ui08 *hdr, int len);
  int _loadAncillaryText(const ui08 *hdr, int len);
  int _loadKeyHeader(const ui08 *hdr, int len);
  int _loadSegmentIdHeader(const ui08 *hdr, int len);
  int _loadLineQualityHeader(const ui08 *hdr, int len);

  void _loadShortDataBuffer(const ui08 *bitData);
  void _loadShortDataBuffer2(const ui08 *bitData);
  
  string _removeTrailingUnderscores(const string &str);

  int _readCalibration(const char *prologue_path,
                       const ui08 *inBuf, int bufLen);

};

#endif
