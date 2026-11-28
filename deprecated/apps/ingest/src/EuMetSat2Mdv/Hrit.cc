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
///////////////////////////////////////////////////////////////
// Hrit.cc
//
// Hrit object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
///////////////////////////////////////////////////////////////


#include "Hrit.hh"
#include <dataport/bigend.h>
#include <toolsa/Path.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <cerrno>

using namespace std;

// Constructor

Hrit::Hrit(const string &prog_name,
           const Params &params,
           ChannelSet &channelSet) :
        _progName(prog_name),
        _params(params),
        _channelSet(channelSet)

{

  MEM_zero(_projName);

  _fileTypeCode = IMAGE_TYPE;
  _totalHeaderLength = 0;
  _dataFieldLengthBits = 0;
  _dataFieldLengthBytes = 0;
  _bitsPerPixel = 0;
  _nCols = 0;
  _nLines = 0;
  _compressionFlag = 0;
  _geosSubLon = 0;
  _colScalingFactor = 0;
  _lineScalingFactor = 0;
  _colOffset = 0;
  _lineOffset = 0;
  _dataTime = 0;
  _annotTime = 0;
  _keyNumber = 0;
  _keySeed = 0;
  _gpScId = 0;
  _spectralChannelId = 0;
  _segmentSeqNum = 0;
  _plannedStartSegmentSeqNum = 0;
  _plannedEndSegmentSeqNum = 0;
  _dataRepresentation = 0;

  _imageStructureHeaderFound = false;
  _navHeaderFound = false;
  _imageDataHeaderFound = false;
  _annotationHeaderFound = false;
  _timeHeaderFound = false;
  _ancillaryTextHeaderFound = false;
  _keyHeaderFound = false;
  _segmentIdHeaderFound = false;
  _lineQualityHeaderFound = false;

  _shortData = NULL;

}

// destructor

Hrit::~Hrit()

{

  if (_shortData != NULL) {
    delete[] _shortData;
  }

}

//////////////////////////////////////////////////
// Read a file
//
// returns 0 on success, -1 on failure

int Hrit::read(const char *input_path)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Reading file: " << input_path << endl;
  }

  _imageStructureHeaderFound = false;
  _navHeaderFound = false;
  _imageDataHeaderFound = false;
  _annotationHeaderFound = false;
  _timeHeaderFound = false;
  _ancillaryTextHeaderFound = false;
  _keyHeaderFound = false;
  _segmentIdHeaderFound = false;
  _lineQualityHeaderFound = false;

  // open file, uncompress as needed
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopenUncompress(input_path, "r")) == NULL) {
    cerr << "ERROR - Hrit::read" << endl;
    cerr << "  Cannot open input file: " << input_path << endl;
    perror("  ");
    return -1;
  }

  // get file size in bytes

  struct stat fileStat;
  if (ta_fstat(fileno(in), &fileStat)) {
    cerr << "ERROR - Hrit::read" << endl;
    cerr << "  Cannot stat input file: " << input_path << endl;
    perror("  ");
    inFile.fclose();
    return -1;
  }
  int nBytesFile = fileStat.st_size;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  File size in bytes: " << nBytesFile << endl;
  }

  // read file into buffer
    
  ui08 *inBuf = new ui08[nBytesFile];
  if (inFile.fread(inBuf, 1, nBytesFile) != nBytesFile) {
    int errNum = errno;
    cerr << "ERROR - Hrit::read" << endl;
    cerr << "  Cannot read input file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    inFile.fclose();
    delete[] inBuf;
    return -1;
  }
  
  // close input file

  inFile.fclose();

  // load in primary header

  if (_loadPrimaryHeader(inBuf, nBytesFile)) {
    delete[] inBuf;
    return -1;
  }
  
  int expectedFileLen = _totalHeaderLength + _dataFieldLengthBytes;
  if (expectedFileLen > nBytesFile) {
    cerr << "ERROR - Hrit::read" << endl;
    cerr << "  File length too short, nbytes file: " << nBytesFile << endl;
    cerr << "  Expected length: " << expectedFileLen << endl;
    delete[] inBuf;
    return -1;
  }

  // load other headers

  int pos = 16;
  int iret = 0;
  while(pos < (_totalHeaderLength - 3)) {
    int len;
    if (_loadNextHeader(inBuf, nBytesFile, pos, len)) {
      iret = -1;
    }
    pos += len;
  }

  if (iret) {
    delete[] inBuf;
    return -1;
  }

  // check that we have a High res file
  
  if (_xritChannelId != "H") {
    delete[] inBuf;
    return -1;
  }

  // check file types, handle accordingly

  if (_fileTypeCode == PROLOGUE) {
    if (_params.calibration_source == Params::PROLOGUE_FILE) {
      _readCalibration(input_path, inBuf, nBytesFile);
    }
    delete[] inBuf;
    return -1;
  }
  
  if (_fileTypeCode != IMAGE_TYPE) {
    if (_params.debug >= Params::DEBUG_VERBOSE_2) {
      cerr << "WARNING: not IMAGE_TYPE file." << endl;
      cerr << "  Ignoring file: " << input_path << endl;
    }
    delete[] inBuf;
    return -1;
  }

  // check that we have sufficient header information

  if (!_imageStructureHeaderFound ||
      !_navHeaderFound ||
      !_annotationHeaderFound ||
      !_timeHeaderFound) {
    if (_params.debug >= Params::DEBUG_VERBOSE_2) {
      cerr << "WARNING: not sufficent header information" << endl;
      cerr << "  Ignoring file: " << input_path << endl;
    }
    delete[] inBuf;
    return -1;
  }

  // create bit data buffer

  ui08 *bitData = NULL;

  if (_compressionFlag == 0) {

    // copy in uncompressed data

    bitData = new ui08[_dataFieldLengthBytes];
    memcpy(bitData, inBuf + _totalHeaderLength, _dataFieldLengthBytes);
    
  } else {

    // read in compressed data, uncompress
    
    DISE::CxRITFile comp = DISE::CxRITFile(input_path);	
    DISE::CxRITFileDecompressed decomp(comp);

#ifdef DEBUG_PRINT
    if (_params.debug >= Params::DEBUG_VERBOSE_2) {
      cerr << "============ Decompressed file info ==============" << endl;
      printFileInfo(decomp, cerr);
    }
#endif

    const Util::CDataField &dataField = decomp.GetDataField();
    const unsigned char *data = dataField.Data();
    _dataFieldLengthBits = dataField.GetLength();
    if (_dataFieldLengthBits % 8 == 0) {
      _dataFieldLengthBytes = _dataFieldLengthBits / 8;
    } else {
      _dataFieldLengthBytes = _dataFieldLengthBits / 8 + 1;
    }
    bitData = new ui08[_dataFieldLengthBytes];
    memcpy(bitData, data, _dataFieldLengthBytes);

  }

  delete[] inBuf;

  // check data size

  int nPixelsData = _dataFieldLengthBits / _bitsPerPixel;
  if (nPixelsData != _nPixels) {
    cerr << "ERROR - Hrit::read" << endl;
    cerr << "  Input file: " << input_path << endl;
    cerr << "  Incorrect number of data pixels found: " << nPixelsData << endl;
    cerr << "  Number of pixels expected: " << _nPixels << endl;
    return -1;
  }

  // load short data buffer

  _loadShortDataBuffer(bitData);
  delete[] bitData;
  
  return 0;

}

//////////////////////////////////////////////////
// load primary header

int Hrit::_loadPrimaryHeader(const ui08 *inBuf, int bufLen)

{


  if (bufLen < 16) {
    cerr << "ERROR - Hrit::_loadPrimaryHeader" << endl;
    cerr << "  File too short for primary header" << endl;
    cerr << "  Need at least 16 bytes" << endl;
    cerr << "  File size: " << bufLen << endl;
    return -1;
  }

  const ui08 *primary = inBuf;

  ui08 hType = primary[0];
  ui16 hLen;
  memcpy(&hLen, primary + 1, sizeof(hLen));
  BE_to_array_16(&hLen, sizeof(hLen));
  
  if (hType != 0) {
    cerr << "ERROR - Hrit::_loadPrimaryHeader" << endl;
    cerr << "  Primary header should have type of 0" << endl;
    cerr << "  Type found: " << hType << endl;
    return -1;
  }

  if (hLen != 16) {
    cerr << "ERROR - Hrit::_loadPrimaryHeader" << endl;
    cerr << "  Primary header should have length of 16" << endl;
    cerr << "  Length found: " << hLen << endl;
    return -1;
  }

  _fileTypeCode = (file_type_code_t) primary[3];

  ui32 totLen;
  memcpy(&totLen, primary + 4, sizeof(totLen));
  BE_to_array_32(&totLen, sizeof(totLen));
  _totalHeaderLength = (int) totLen;

  ui64 nBits;
  memcpy(&nBits, primary + 8, sizeof(nBits));
  BE_to_array_64(&nBits, sizeof(nBits));
  _dataFieldLengthBits = nBits;
  if (nBits % 8 == 0) {
    _dataFieldLengthBytes = nBits / 8;
  } else {
    _dataFieldLengthBytes = nBits / 8 + 1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "================================" << endl;
    cerr << "  Primary header" << endl;
    cerr << "  Header type: " << (int) hType << endl;
    cerr << "  Header len: " << (int) hLen << endl;
    switch (_fileTypeCode) {
      case IMAGE_TYPE:
        cerr << "  fileTypeCode: IMAGE" << endl;
        break;
      case GTS_TYPE:
        cerr << "  fileTypeCode: GTS" << endl;
        break;
      case ALPHA_NUM_TYPE:
        cerr << "  fileTypeCode: ALPHA NUMERIC" << endl;
        break;
      case ENCR_KEY_TYPE:
        cerr << "  fileTypeCode: ENCRYPTION KEY" << endl;
        break;
      case PROLOGUE:
        cerr << "  fileTypeCode: PROLOGUE" << endl;
        break;
      case EPILOGUE:
        cerr << "  fileTypeCode: EPILOGUE" << endl;
        break;
      default:
        cerr << "  fileTypeCode: UNKNOWN: " << _fileTypeCode << endl;
    }
    cerr << "  totalHeaderLength: " << _totalHeaderLength << endl;
    cerr << "  dataFieldLengthBits: " << _dataFieldLengthBits << endl;
    cerr << "  dataFieldLengthBytes: " << _dataFieldLengthBytes << endl;
  }

  if (bufLen > 16) {
    ui08 nextHeaderType = primary[16];
    if (_params.debug >= Params::DEBUG_VERBOSE_2) {
      cerr << "================================" << endl;
      cerr << "Next header type: " << (int) nextHeaderType << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// load next header

int Hrit::_loadNextHeader(const ui08 *inBuf, int bufLen, int pos, int &len)

{

  const ui08 *hdr = inBuf + pos;

  int hType = (int) hdr[0];
  ui16 hLen;
  memcpy(&hLen, hdr + 1, sizeof(hLen));
  BE_to_array_16(&hLen, sizeof(hLen));

  len = (int) hLen;

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "=========================================" << endl;
    cerr << "Header type: " << hType << endl;
    cerr << "       len: " << len << endl;
  }

  if (hType == 1) {
    if (_loadImageStructureHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _imageStructureHeaderFound = true;
  } else if (hType == 2) {
    if (_loadNavHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _navHeaderFound = true;
  } else if (hType == 3) {
    if (_loadImageDataHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _imageDataHeaderFound = true;
  } else if (hType == 4) {
    if (_loadAnnotationHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _annotationHeaderFound = true;
  } else if (hType == 5) {
    if (_loadTimeHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _timeHeaderFound = true;
  } else if (hType == 6) {
    if (_loadAncillaryText(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _ancillaryTextHeaderFound = true;
  } else if (hType == 7) {
    if (_loadKeyHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _keyHeaderFound = true;
  } else if (hType == 128) {
    if (_loadSegmentIdHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _segmentIdHeaderFound = true;
  } else if (hType == 129) {
    if (_loadLineQualityHeader(hdr, len)) {
      cerr << "ERROR - _loadNextHeader" << endl;
      return -1;
    }
    _lineQualityHeaderFound = true;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// image structure header

int Hrit::_loadImageStructureHeader(const ui08 *hdr, int len)

{

  if (len != 9) {
    cerr << "ERROR - _loadNavHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be 51" << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 3;

  ui08 bval = *ptr;
  _bitsPerPixel = bval;
  ptr++;

  ui16 sval;
  memcpy(&sval, ptr, sizeof(sval));
  BE_to_array_16(&sval, sizeof(sval));
  _nCols = sval;
  ptr += 2;

  memcpy(&sval, ptr, sizeof(sval));
  BE_to_array_16(&sval, sizeof(sval));
  _nLines = sval;
  ptr += 2;

  bval = *ptr;
  _compressionFlag = bval;

  _nPixels = _nCols * _nLines;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ------------------------" << endl;
    cerr << "  IMAGE STRUCTURE HEADER" << endl;
    cerr << "    bitsPerPixel: " << _bitsPerPixel << endl;
    cerr << "    nCols: " << _nCols << endl;
    cerr << "    nLines: " << _nLines << endl;
    cerr << "    nPixels: " << _nPixels << endl;
    cerr << "    compressionFlag: " << _compressionFlag << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// load nav header

int Hrit::_loadNavHeader(const ui08 *hdr, int len)

{

  if (len != 51) {
    cerr << "ERROR - _loadNavHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be 51" << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 3;
  memcpy(_projName, ptr, 32);
  _projName[31] = '\0';
  ptr += 32;
  
  ui32 val;
  memcpy(&val, ptr, sizeof(val));
  BE_to_array_32(&val, sizeof(val));
  _colScalingFactor = val;
  ptr += 4;

  memcpy(&val, ptr, sizeof(val));
  BE_to_array_32(&val, sizeof(val));
  _lineScalingFactor = val;
  ptr += 4;

  memcpy(&val, ptr, sizeof(val));
  BE_to_array_32(&val, sizeof(val));
  _colOffset = val;
  ptr += 4;

  memcpy(&val, ptr, sizeof(val));
  BE_to_array_32(&val, sizeof(val));
  _lineOffset = val;

  // check projection type

  if (strncmp(_projName, "GEOS", 4)) {
    cerr << "ERROR: _loadNavHeader" << endl;
    cerr << "  Only GEOS projection supported" << endl;
    cerr << "  Proj name: " << _projName << endl;
    return -1;
  }

  // set GEOS sub-longitude

  if (sscanf(_projName, "GEOS(%lg)", &_geosSubLon) != 1) {
    cerr << "ERROR: _loadNavHeader" << endl;
    cerr << "  Cannot decode GOES sub-longitude" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "  ------------------------" << endl;
    cerr << "  NAV HEADER" << endl;
    cerr << "    projName: " << _projName << endl;
    cerr << "    colScalingFactor: " << _colScalingFactor << endl;
    cerr << "    lineScalingFactor: " << _lineScalingFactor << endl;
    cerr << "    colOffset: " << _colOffset << endl;
    cerr << "    lineOffset: " << _lineOffset << endl;
    cerr << "    geosSubLon: " << _geosSubLon << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// load data def header

int Hrit::_loadImageDataHeader(const ui08 *hdr, int len)

{

  if (len < 3) {
    cerr << "ERROR - _loadImageDataHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be at least 3" << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 3;
  int defLen = len - 3;
  char def[defLen + 1];
  MEM_zero(def);
  memcpy(def, ptr, defLen);
  _dataDef = def;

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "  ------------------------" << endl;
    cerr << "  IMAGE DATA HEADER" << endl;
    cerr << "    data def: " << _dataDef << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// load annotation header

int Hrit::_loadAnnotationHeader(const ui08 *hdr, int len)

{

  if (len < 3) {
    cerr << "ERROR - _loadAnnotationHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be at least 3" << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 3;
  int annotLen = len - 3;
  char annot[annotLen + 1];
  MEM_zero(annot);
  memcpy(annot, ptr, annotLen);
  _annotation = annot;

  vector<string> toks;
  tokenize(annot, "-", toks);
  if (toks.size() < 7) {
    cerr << "ERROR - _loadAnnotationHeader" << endl;
    cerr << "  Should be at least 7 tokens delimited by '_'" << endl;
    cerr << "  annotStr: " << _annotation << endl;
    return -1;
  }

  _xritChannelId = _removeTrailingUnderscores(toks[0]);
  _annotVersion = _removeTrailingUnderscores(toks[1]);
  _annotDisseminator = _removeTrailingUnderscores(toks[2]);
  _annotSatName = _removeTrailingUnderscores(toks[3]);
  _annotChannel = _removeTrailingUnderscores(toks[4]);
  _annotSegment = _removeTrailingUnderscores(toks[5]);
  _annotTimeStr = _removeTrailingUnderscores(toks[6]);

  int iret = 0;
  int year, month, day, hour, min;
  if (sscanf(_annotTimeStr.c_str(), "%4d%2d%2d%2d%2d", &year, &month, &day, &hour, &min) != 5) {
    cerr << "ERROR - _loadAnnotationHeader" << endl;
    cerr << "  Cannot decode annotation time string: " << _annotTimeStr << endl;
    iret = -1;
  }
  DateTime atime(year, month, day, hour, min, 0);
  _annotTime = atime.utime();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ------------------------" << endl;
    cerr << "  ANNOTATION HEADER" << endl;
    cerr << "    satName: " << _annotSatName << endl;
    cerr << "    channel: " << _annotChannel << endl;
    cerr << "    segment: " << _annotSegment << endl;
  }
    
  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "    annotation str: " << _annotation << endl;
    cerr << "    xritChannelId: " << _xritChannelId << endl;
    cerr << "    version: " << _annotVersion << endl;
    cerr << "    disseminator: " << _annotDisseminator << endl;
    cerr << "    timeStr: " << _annotTimeStr << endl;
    cerr << "    time: " << DateTime::strn(_annotTime) << endl;
  }
    
  return iret;

}

//////////////////////////////////////////////////
// load time header

int Hrit::_loadTimeHeader(const ui08 *hdr, int len)

{

  if (len != 10) {
    cerr << "ERROR - _loadTimeHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be 10" << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 4;

  ui16 day;
  memcpy(&day, ptr, sizeof(day));
  BE_to_array_16(&day, sizeof(day));
  ptr += 2;
  
  ui32 msecs;
  memcpy(&msecs, ptr, sizeof(msecs));
  BE_to_array_32(&msecs, sizeof(msecs));
  
  DateTime epoch(1958, 1, 1, 0, 0, 0);
  _dataTime = (time_t) ((double) epoch.utime() + ((double) day * 86400) + (msecs / 1000.0) + 0.5);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ------------------------" << endl;
    cerr << "  TIME HEADER" << endl;
    cerr << "    data time: " << DateTime::strn(_dataTime) << endl;
    cerr << "    day count: " << day << endl;
    cerr << "    msecs: " << msecs << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// load ancillary text

int Hrit::_loadAncillaryText(const ui08 *hdr, int len)

{

  if (len < 3) {
    cerr << "ERROR - _loadAncillaryText" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be at least 3" << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 3;
  int textLen = len - 3;
  char text[textLen + 1];
  MEM_zero(text);
  memcpy(text, ptr, textLen);
  _ancillaryText = text;

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "  ------------------------" << endl;
    cerr << "  ANCILLARY TEXT" << endl;
    cerr << "    text: " << _ancillaryText << endl;
  }
  
  return 0;

}


//////////////////////////////////////////////////
// load key header

int Hrit::_loadKeyHeader(const ui08 *hdr, int len)

{

  if (len != 12) {
    cerr << "ERROR - _loadKeyHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be 12" << endl;
    return -1;
  }
  
  const ui08 *ptr = hdr + 3;

  ui08 val08 = *ptr;
  _keyNumber = val08;
  ptr++;

  ui64 val64;
  memcpy(&val64, ptr, sizeof(val64));
  BE_to_array_64(&val64, sizeof(val64));
  _keySeed = val64;

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "  ------------------------" << endl;
    cerr << "  KEY HEADER" << endl;
    cerr << "    keyNumber: " << _keyNumber << endl;
    cerr << "    keySeed: " << _keySeed << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// load image segment identification header

int Hrit::_loadSegmentIdHeader(const ui08 *hdr, int len)

{

  if (len != 13) {
    cerr << "ERROR - _loadSegmentIdHeader" << endl;
    cerr << "  Incorrect header length: " << len << endl;
    cerr << "  Should be 13" << endl;
    return -1;
  }
  
  const ui08 *ptr = hdr + 3;
  
  ui16 val16;
  memcpy(&val16, ptr, sizeof(val16));
  BE_to_array_16(&val16, sizeof(val16));
  _gpScId = (int) val16;
  ptr += 2;

  ui08 val08 = *ptr;
  _spectralChannelId = (int) val08;
  ptr++;

  memcpy(&val16, ptr, sizeof(val16));
  BE_to_array_16(&val16, sizeof(val16));
  _segmentSeqNum = (int) val16;
  ptr += 2;

  memcpy(&val16, ptr, sizeof(val16));
  BE_to_array_16(&val16, sizeof(val16));
  _plannedStartSegmentSeqNum = (int) val16;
  ptr += 2;

  memcpy(&val16, ptr, sizeof(val16));
  BE_to_array_16(&val16, sizeof(val16));
  _plannedEndSegmentSeqNum = (int) val16;
  ptr += 2;

  val08 = *ptr;
  _dataRepresentation = (int) val08;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ------------------------" << endl;
    cerr << "  SEGMENT ID HEADER" << endl;
    cerr << "    gpScId: " << _gpScId << endl;
    cerr << "    spectralChannelId: " << _spectralChannelId << endl;
    cerr << "    segmentSeqNum: " << _segmentSeqNum << endl;
    cerr << "    plannedStartSegmentSeqNum: " << _plannedStartSegmentSeqNum << endl;
    cerr << "    plannedEndSegmentSeqNum: " << _plannedEndSegmentSeqNum << endl;
    cerr << "    dataRepresentation: " << _dataRepresentation << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// load line quality header

int Hrit::_loadLineQualityHeader(const ui08 *hdr, int len)

{

  int minLen = 3 + _nLines * 13;
  if (len < minLen) {
    cerr << "ERROR - _loadSegmentIdHeader" << endl;
    cerr << "  Header length too short: " << len << endl;
    cerr << "  Should be at least: " << minLen << endl;
    return -1;
  }

  const ui08 *ptr = hdr + 3;
  _lineQuality.clear();

  for (int ii = 0; ii < _nLines; ii++) {
  
    line_quality_t qual;

    ui32 val32;
    memcpy(&val32, ptr, sizeof(val32));
    BE_to_array_32(&val32, sizeof(val32));
    qual.line_num = (int) val32;
    ptr += 4;

    ui16 day;
    memcpy(&day, ptr, sizeof(day));
    BE_to_array_16(&day, sizeof(day));
    ptr += 2;
    
    ui32 msecs;
    memcpy(&msecs, ptr, sizeof(msecs));
    BE_to_array_32(&msecs, sizeof(msecs));
    ptr += 4;
  
    DateTime epoch(1958, 1, 1, 0, 0, 0);
    qual.time = (time_t) ((double) epoch.utime() + ((double) day * 86400) +
                          (msecs / 1000.0) + 0.5);
    qual.msecs = msecs % 1000;

    ui08 val08 = *ptr;
    qual.validity = (int) val08;
    ptr++;
    
    val08 = *ptr;
    qual.radiometric_quality = (int) val08;
    ptr++;
    
    val08 = *ptr;
    qual.geometric_quality = (int) val08;
    ptr++;

    _lineQuality.push_back(qual);

  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ------------------------" << endl;
    cerr << "  LINE QUALITY HEADER FOUND" << endl;
    cerr << endl;
    if (_params.debug >= Params::DEBUG_VERBOSE_2) {
      fprintf(stderr, "%9s %10s %12s %10s %12s %12s\n",
              "Line num", "date", "time", "quality",
              "radiometric", "geometric");
      for (int ii = 0; ii < (int) _lineQuality.size(); ii++) {
        fprintf(stderr, "%9d %s.%.3d %10d %12d %12d\n",
                _lineQuality[ii].line_num,
                DateTime::strn(_lineQuality[ii].time).c_str(),
                _lineQuality[ii].msecs,
                _lineQuality[ii].validity,
                _lineQuality[ii].radiometric_quality,
                _lineQuality[ii].geometric_quality);
      } // ii 
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// load up short data buffer from bit data buffer

void Hrit::_loadShortDataBuffer(const ui08 *bitData)

{

  // We will use the fact that every _bitsPerPixel bytes, the
  // byte boundary will match a data boundary

  // Prepare arrays for byte position, offsets, shifts and masks.
  // We will copy the bytes into a 4-byte , mask off bits not applicable,
  // and shift right, then set a 2-byte value to the result.

  int startByte;
  int endByte;
  int nBytes;
  int offset;
  int rshift[8];
  int startCopy[8];
  ui32 mask[8];

  int blockSize = _bitsPerPixel;
  ui32 one = 1;

  for (int ii = 0; ii < 8; ii++) {

    int startBit = ii * blockSize;
    int endBit = startBit + blockSize - 1;
    startByte = startBit / 8;
    endByte = endBit / 8;
    nBytes = endByte - startByte + 1;
    int byteEndBit = (endByte * 8) + 7;

    startCopy[ii] = startByte - (4 - nBytes);
    
    rshift[ii] = byteEndBit - endBit;
    offset =  32 - rshift[ii] - blockSize;

    mask[ii] = 0;
    for (int jj = 0; jj < blockSize; jj++) {
      mask[ii] |= (one << (31 - (jj + offset)));
    }

#ifdef JUNK
    cerr << "---->> ii: " << ii << endl;
    cerr << "  startBit, endBit, byteEndBit: "
         << startBit << ", " << endBit << ", " << byteEndBit << endl;
    cerr << "  startByte, endByte, nBytes: "
         << startByte << ", " << endByte << ", " << nBytes << endl;
    cerr << "  offset: " << offset << endl;
    cerr << "  rshift: " << rshift[ii] << endl;
    cerr << "  startCopy: " << startCopy[ii] << endl;

    cerr << "  mask: " << mask[ii] << ": ";
    for (int jj = 31; jj >= 0; jj--) {
      if (mask[ii] & (one << jj)) {
        cerr << "1";
      } else {
        cerr << "0";
      }
    }
    cerr << endl;
#endif

  } // ii

  // make a buffer 4 bytes longer than the number of bits

  ui08 blockBuf[blockSize + 4];
  MEM_zero(blockBuf);
  
  // copy in from the data array in chunks blockSize bytes long

  if (_shortData != NULL) {
    delete[] _shortData;
  }
  _shortData = new ui16[_nPixels];

  int nBlocks = _dataFieldLengthBytes / blockSize;
  if ((_dataFieldLengthBytes % blockSize) != 0) {
    nBlocks++;
  }

  int blockStart  = 0;
  int pixelNum = 0;
  for (int iblock = 0; iblock < nBlocks; iblock++, blockStart += blockSize) {
    
    int nCopy = _dataFieldLengthBytes - blockStart;
    if (nCopy > blockSize) {
      nCopy = blockSize;
    }
    memcpy(blockBuf, bitData + blockStart, nCopy);

    for (int ibyte = 0; ibyte < 8; ibyte++) {

      // copy in 4 bytes, ending with the last byte which contains
      // data of interest

      ui32 val32;
      memcpy(&val32, blockBuf + startCopy[ibyte], 4);

      // swap to big-endian

      BE_to_array_32(&val32, 4);
      
      // mask out bits outside the range of interest

      ui32 val32_1 = (val32 & mask[ibyte]);

      // right shift to move the bits to the low end

      ui32 val32_2 = (val32_1 >> rshift[ibyte]);
      
      _shortData[pixelNum] = (ui16) val32_2;
      pixelNum++;
      if (pixelNum >= _nPixels) {
        return;
      }

#ifdef DEBUG_PRINT

      if (val32_2 != 0) {
        
        cerr << "+++++++++ pixelNum: " << pixelNum << endl;
        cerr << "startCopy: " << startCopy[ibyte] << endl;
        cerr << "rshift: " << rshift[ibyte] << endl;
        
        cerr << "  mask: " << mask[ibyte] << ": ";
        for (int jj = 31; jj >= 0; jj--) {
          if (mask[ibyte] & (one << jj)) {
            cerr << "1";
          } else {
            cerr << "0";
          }
        }
        cerr << endl;
        
        cerr << "-- val32  : " << val32 << " : ";
        for (int jj = 31; jj >= 0; jj--) {
          if (val32 & (one << jj)) {
            cerr << "1";
          } else {
            cerr << "0";
          }
        }
        cerr << endl;
        
        cerr << "-- val32_1: " << val32_1 << " : ";
        for (int jj = 31; jj >= 0; jj--) {
          if (val32_1 & (one << jj)) {
            cerr << "1";
          } else {
            cerr << "0";
          }
        }
        cerr << endl;
        
        cerr << "-- val32_2: " << val32_2 << " : ";
        for (int jj = 31; jj >= 0; jj--) {
          if (val32_2 & (one << jj)) {
            cerr << "1";
          } else {
            cerr << "0";
          }
        }
        cerr << endl;
        cerr << iblock << ", " << ibyte << ", " << val32_2
             << ", " << pixelNum << endl;
        
      } // if (val32_2 != 0) 

#endif
      
    } // ibyte

  } // iblock

}

//////////////////////////////////////////////////
// load up short data buffer from bit data buffer
//
// Simple version for testing and comparison

void Hrit::_loadShortDataBuffer2(const ui08 *bitData)

{

  if (_shortData != NULL) {
    delete[] _shortData;
  }
  _shortData = new ui16[_nPixels];

  int one = 1;
  
  for (int ii = 0; ii < _nPixels; ii++) {

    ui64 msb = ii * 10;
    ui64 lsb = msb + 9;
    
    ui16 val16 = 0;

    for (ui64 jj = msb; jj <= lsb; jj++) {
      
      int bitPos = 10 - (int) (jj - msb) - 1;
      int ibyte = jj / 8;
      int ibit = 7 - (jj % 8);
      ui08 byteVal = bitData[ibyte];

      if (byteVal == 0) {
        continue;
      }
      
      int bitVal = byteVal & (one << ibit);

      if (bitVal != 0) {
        val16 |= (one << bitPos);
      }
      
    } // jj

    _shortData[ii] = val16;

#ifdef DEBUG_PRINT
    if (val16 != 0) {
      cerr << "++++ pixel: " << ii << endl;
      cerr << "val16: " << (int) val16 << ": ";
      for (int jj = 15; jj >= 0; jj--) {
        if (val16 & (one << jj)) {
          cerr << "1";
        } else {
          cerr << "0";
        }
      }
      cerr << endl;
    }
#endif
      
  } // ii

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void Hrit::tokenize(const string &str,
                    const string &spacer,
                    vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

////////////////////////////////////////////////////////
// remove underscores from trailing position of a string

string Hrit::_removeTrailingUnderscores(const string &str)
  
{

  string cleaned;

  int firstUnderscore = str.size();
  for (int ii = str.size() - 1; ii >= 0; ii--) {
    if (str[ii] != '_') {
      break;
    }
    firstUnderscore = ii;
  }

  cleaned.assign(str, 0, firstUnderscore);
  return cleaned;
   
}

void Hrit::printFileInfo(const DISE::CxRITFile &hritFile,
                         ostream &out) 
{
  
  const Util::CDataField &dataField = hritFile.GetDataField();
  ui64 nDataBits = dataField.GetLength();

  out << "--->> File data sizes <<----" << endl;
  out << "  nDataBits: " << nDataBits << endl;
  out << "  nDataBytes: " << (nDataBits / 8) << endl;

  out << "==============================================" << endl;
  out << "  FileTypeCode: " << hritFile.GetFileTypeCode() << endl;
  out << "  DataFieldLength: " << hritFile.GetDataFieldLength() << endl;
  out << "  NB: " << hritFile.GetNB() << endl;
  out << "  NC: " << hritFile.GetNC() << endl;
  out << "  NL: " << hritFile.GetNL() << endl;
  out << "  CompressionFlag: " << hritFile.GetCompressionFlag() << endl;
  out << "  ProjectionName: " << hritFile.GetProjectionName() << endl;
  out << "  CFAC: " << hritFile.GetCFAC() << endl;
  out << "  LFAC: " << hritFile.GetLFAC() << endl;
  out << "  COFF: " << hritFile.GetCOFF() << endl;
  out << "  LOFF: " << hritFile.GetLOFF() << endl;
  out << "  DataDefinitionBlock: " << hritFile.GetDataDefinitionBlock() << endl;

  out << "==============================================" << endl;
  const DISE::CxRITAnnotation &annot = hritFile.GetAnnotation();
  out << "Annotation:" << endl;
  out << "  HRITFlag: " << annot.GetHRITFlag() << endl;
  out << "  CompressedFlag: " << annot.GetCompressedFlag() << endl;
  out << "  EncryptedFlag: " << annot.GetEncryptedFlag() << endl;
  out << "  ProductID1: " << annot.GetProductID1() << endl;
  out << "  ProductID2: " << annot.GetProductID2() << endl;
  out << "  ProductID3: " << annot.GetProductID3() << endl;
  out << "  ProductID4: " << annot.GetProductID4() << endl;
  out << "  Text: " << annot.GetText() << endl;
  out << "  Identifier: " << annot.GetIdentifier() << endl;

  out << "==============================================" << endl;
  const SYSTIME &timeStamp = hritFile.GetTimeStamp();
  out << "TimeStamp: " << endl;
  out << "  Year: " << timeStamp.GetYear() << endl;
  out << "  Month: " << timeStamp.GetMonth() << endl;
  out << "  Day: " << timeStamp.GetDayOfMonth() << endl;
  out << "  Hour: " << timeStamp.GetHour() << endl;
  out << "  Min: " << timeStamp.GetMinuteOfHour() << endl;
  out << "  Sec: " << timeStamp.GetSecondOfMinute() << endl;
  
  out << "==============================================" << endl;
  out << "AncillaryText: " << hritFile.GetAncillaryText() << endl;
  out << "  KeyNumber: " << hritFile.GetKeyNumber() << endl;
  out << "  Seed: " << hritFile.GetSeed() << endl;

  out << "==============================================" << endl;
  const DISE::CSpacecraftID &scraft = hritFile.GetSpacecraftID();
  out << "  spacecraftId: " << scraft.ReadableForm() << endl;

  out << "==============================================" << endl;
  const DISE::CSpectralChannelID &chan = hritFile.GetSpectralChannelID();
  out << "  spectralChannelId: " << chan.GetNameSEVIRI() << endl;

  out << "==============================================" << endl;
  out << "  SegmentSeqNo: " << hritFile.GetSegmentSeqNo() << endl;
  out << "  PlannedStartSegmentNo: " << hritFile.GetPlannedStartSegmentNo() << endl;
  out << "  PlannedEndSegmentNo: " << hritFile.GetPlannedEndSegmentNo() << endl;
  out << "  DataFieldRepresentation: " << hritFile.GetDataFieldRepresentation() << endl;

}

//////////////////////////////////////////////////
// read calibration from a prologue file

int Hrit::_readCalibration(const char *prologue_path,
                           const ui08 *inBuf, int bufLen)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====================================================" << endl;
    cerr << "Handling PROLOGUE file" << endl;
    cerr << "  Looking for calibration data" << endl;
  }
  
  // set data pointer to correct location in input buffer

  const ui08 *data = inBuf + _totalHeaderLength;

  // we need to find the calibration data
  // 
  // we search through the data, looking for recognizable pattern which
  // occurs a known number of bytes before the calibration values
  // 
  // We are looking for the following pattern:
  //   integer between 1 and 3712
  //   integer between 1 and 3712
  //   integer == 1
  //   integer == 3712
  //
  // This pattern occurs in the Image Description Record, which immediately
  // precedes the Radiometric Processing record, which holds the calib
  // data
  
  int pos = 0;
  si32 vals[13];
  memset(vals, 0, sizeof(vals));
  
  for (int ii = 0;
       ii < (int) _dataFieldLengthBytes - (int) sizeof(si32); ii++) {

    // load an int from the current byte position

    memcpy(&vals[0], data + ii, sizeof(si32));
    BE_to_array_32(&vals[0], sizeof(si32));

    // check for pattern
    
    if (vals[0] == 3712 &&
        vals[4] == 1 &&
        vals[8] > 0 && vals[8] < 3713 &&
        vals[12] > 0 && vals[12] < 3713) {

      // found pattern

      if (_params.debug >= Params::DEBUG_VERBOSE_2) {
        cerr << "--->>> Found pattern: "
             << vals[12] << ", " << vals[8] << ", "
             << vals[4] << ", " << vals[0] << endl;
        cerr << "       pos: " << ii << endl;
      }
      
      // store current location and break
      
      pos = ii;
      break;
    }

    // move integer values down array

    memmove(vals + 1, vals, 48);
    
  }

  if (pos == 0) {
    cerr << "WARNING - could not read calibration in prologue file" << endl;
    cerr << "  File: " << prologue_path << endl;
    return -1;
  }

  // move ahead by 122 to the calibration values

  pos += 122;

  // read in scale and offset values

  vector<double> slope, offset;
  
  for (int ii = 0; ii < ChannelSet::nChannels; ii++) {
    
    fl64 dval;
    memcpy(&dval, data + pos, sizeof(fl64));
    BE_to_array_64(&dval, sizeof(fl64));
    slope.push_back(dval);
    pos += 8;

    memcpy(&dval, data + pos, sizeof(fl64));
    BE_to_array_64(&dval, sizeof(fl64));
    offset.push_back(dval);
    pos += 8;

  }

  // check the values to make sure they are reasonable

  for (int ii = 0; ii < ChannelSet::nChannels; ii++) {
    if (slope[ii] <= 0 || slope[ii] > 1 ||
        offset[ii] < -1000 | offset[ii] > 1000) {
      if (_params.debug) {
        cerr << "WARNING - bad values in calibration in prologue file" << endl;
        cerr << "  File: " << prologue_path << endl;
      }
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        for (int ii = 0; ii < ChannelSet::nChannels; ii++) {
          fprintf(stderr, "  Channel, slope, offset: %2d %10g %10g\n",
                  ii, slope[ii], offset[ii]);
        }
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Found calibration:" << endl;
    for (int ii = 0; ii < ChannelSet::nChannels; ii++) {
      fprintf(stderr, "  Channel, slope, offset: %2d %15.8f %15.8f\n",
              ii, slope[ii], offset[ii]);
    }
  }

  // update the calibration

  _channelSet.updateCalibration(slope, offset);

  return 0;

}
