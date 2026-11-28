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


#ifndef ASDI_H
#define ASDI_H


#include <cstdio>
#include <toolsa/Socket.hh>
#include <Spdb/DsSpdb.hh>
#include <tinyxml/tinystr.h>
#include <tinyxml/tinyxml.h>

#include "Params.hh"
#include "routeDecode.hh"

#define HeartBeatDataType 1
#define XMLDataType 2
#define MACH1 761.2

typedef struct xml_header_t
{
  char timestamp[16];
  int data_type;
  int sequence_number;
  int compressed_size;
  int decompressed_size;
} xml_header_t;

class asdiXml2spdb {
 
public:

  // constructor
  asdiXml2spdb(Params *tdrpParams);
 
  // destructor 
  ~asdiXml2spdb();    
    
  // public methods.
  void File(char *FilePath);
  void Stream();

  // Flush out the buffers to SPDB
  void flush();

protected:
 
private:

  void _processHeader(char *buff, xml_header_t *header);
  int _uncompressXml(char *buff, int buff_size, char *out_buff, int out_buff_size);

  int _openSocket();
  void _closeSocket();
  int _readFromSocket(char *buffer, int numbytes);

  void _processAsdiXml(char *asdiMsg);
  void _contriveDataType(const char *fltID, si32 *dataType, si32 *dataType2);

  void _extractString(char *instring, char *outstring);
  int  _xtoi(const char* xs, unsigned int* result);

  void _save2ascii(date_time_t *fT, char *asdiMsg, int extN);

  bool _saveRouteToSPDB(const si32 data_type, const time_t valid_time,
			const time_t expire_time, const int chunk_len,
			const void *chunk_data, const si32 data_type2);

  void _parseTZ(TiXmlElement* TZ, date_time_t T);
  void _parseFZ(TiXmlElement* FZ, date_time_t T);
  void _parseAF(TiXmlElement* AF, date_time_t T);
  void _parseUZ(TiXmlElement* UZ, date_time_t T);
  void _parseAZ(TiXmlElement* AZ, date_time_t T);
  void _parseDZ(TiXmlElement* DZ, date_time_t T);
  void _parseRZ(TiXmlElement* RZ, date_time_t T);
  void _parseTO(TiXmlElement* TO, date_time_t T);

  void _getPointStr(TiXmlElement *element, char *str);

  int _valueToInt(const TiXmlElement* element);
  float _valueToFloat(const TiXmlElement* element);
  const char* _valueToChar(const TiXmlElement* element);

  //
  // Data.
  //  
  bool _readingFromFile;
  char * _inputFile;

  int _inputFileYear;
  int _inputFileMonth;
  int _inputFileDay;
  int _inputFileHour;

  FILE *_fp;
  Socket _S;
  TiXmlDocument   _doc;
  Params *_params;
  routeDecode *_route;

  const static int _internalStringLen = 64;
  const static int _routeStringLen = 254;

  int _spdbBufferCount;
  DsSpdb _spdb;
  int _spdbRouteBufferCount;
  DsSpdb _spdbRoute;

  const static int _asciiBufferLen = 8192;
  char _asciiBuffer[_asciiBufferLen];
  char _asciiBuffer2[_asciiBufferLen];
  int _prevSeqNum;

};  

#endif
   
