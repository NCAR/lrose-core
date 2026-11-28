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


#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <zlib.h>

#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <rapformats/ac_data.h>
#include <rapformats/ac_route.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/file_io.h>
#include <dataport/bigend.h>

#include "asdiXml2spdb.hh"

const long int MAX_HEADER_SIZE = 10000;
const long int MAX_XML_SIZE = 2000000;

//
// Constructor
//
asdiXml2spdb::asdiXml2spdb(Params *tdrpParams){ 
  //
  // Make a copy of the params and init a few variables.
  //
  _params = tdrpParams;
  _spdbBufferCount = 0;
  _spdbRouteBufferCount = 0;
  _inputFileYear = 0;
  _inputFileMonth = 0;
  _prevSeqNum = 0;
  memset(_asciiBuffer,  0, _asciiBufferLen);
  memset(_asciiBuffer2,  0, _asciiBufferLen);
  _spdbRoute.setPutMode(Spdb::putModeAdd);

  _route = NULL;

}

//
// Destructor
//
asdiXml2spdb::~asdiXml2spdb(){

  flush();
  if(_params->decodeRoute && _route != NULL) {
    delete _route;
  }
}

void asdiXml2spdb::flush()
{
  if (_params->saveSPDB){
    if (_spdb.put( _params->PosnOutUrl,
		   SPDB_AC_DATA_ID,
		   SPDB_AC_DATA_LABEL)){
      fprintf(stderr,"ERROR: Failed to put posn data\n");
    }
    if (_spdbRoute.put( _params->RouteOutUrl,
    			SPDB_AC_ROUTE_ID,
    			SPDB_AC_ROUTE_LABEL)){
      fprintf(stderr,"ERROR: Failed to put data\n");
      return;
    }    
  }

}

void asdiXml2spdb::File(char *FilePath)
{

  //
  // Set up for file reads.
  //
  _fp = fopen(FilePath,"rb");
  if (_fp == NULL){
    cerr << " ERROR: Data source - file or stream - unopenable." << endl;
    return;
  }
  _inputFile = FilePath;

  char buffer[MAX_XML_SIZE];
  
  int i = 0;
  while(TRUE)
  {
    int numRead = fread(&buffer[i], sizeof(unsigned char), 1, _fp);
    if (numRead == 1) {
      // Read until we find "/asdiOutput>" or "/ds:tfmDataService> in the buffer.
      if((i > 11 && buffer[i] == '>' && buffer[i-1] == 't' && buffer[i-2] == 'u' 
	 && buffer[i-3] == 'p' && buffer[i-4] == 't' && buffer[i-5] == 'u' && buffer[i-6] == 'O'
	 && buffer[i-7] == 'i' && buffer[i-8] == 'd' && buffer[i-9] == 's' && buffer[i-10] == 'a'
	 && buffer[i-11] == '/') || 
	 (i > 11 && buffer[i] == '>' && buffer[i-1] == 'e' && buffer[i-2] == 'c' 
	 && buffer[i-3] == 'i' && buffer[i-4] == 'v' && buffer[i-5] == 'r' && buffer[i-6] == 'e'
	 && buffer[i-7] == 'S' && buffer[i-8] == 'a' && buffer[i-9] == 't' && buffer[i-10] == 'a'
	 && buffer[i-11] == 'D')
	  ) {
	buffer[i+1] = char(0);
	_processAsdiXml( buffer );
	i = 0;
      } else {
	if (i < MAX_XML_SIZE-2) {
	  i++;
	} else {
	  cerr << "ERROR: Read problem, Buffer full" << endl;
	  break;
	}
      }
    } else {
      if (feof(_fp))
	if(i > 0) _processAsdiXml( buffer );
	break;
    }
  }

  fclose(_fp);
}

void asdiXml2spdb::Stream()
{
  _inputFile = NULL;

  if (_openSocket())
  {
    cerr << "ERROR: Data source - file or stream - unopenable." << endl;
    return;
  }
  //if(_params->decodeRoute && _params->saveSPDB && _route != NULL && _route->isOK() == false) {
  //fprintf(stderr,"ERROR: Failed to initialize route decoder class.\n");
  //return;
  //}

  const int headerSize = 32;
  char headerBuffer[headerSize];
  xml_header_t header;
  char *buffer;

  do {
    //
    // Read header
    //
    int numRead = _readFromSocket(&headerBuffer[0], headerSize);
    if (numRead == headerSize) {
      _processHeader(headerBuffer, &header);
      if(_params->debug)
	cout << "Got Header: " << header.timestamp << " " << header.data_type << " " << header.sequence_number << " " << header.compressed_size << " " << header.decompressed_size << endl;
    } else {
      return;
    }

    if(header.sequence_number == _prevSeqNum +1)
      _prevSeqNum++;
    else {
      if(header.sequence_number == 0)
	cout << "Server restart detected.." << endl;
      else {
	if(header.sequence_number-_prevSeqNum-1 > 2)
	  cerr << "ERROR: Sequence number missed, missed " << header.sequence_number-_prevSeqNum-1 << " packets!" << endl;
	_prevSeqNum = header.sequence_number;
      }
    }
    if(_prevSeqNum == 100000)
      _prevSeqNum = 0;

    //
    // Read compressed buffer
    //

    int i = 0;
    int msgSize = header.compressed_size / sizeof(char);
    buffer = new char[msgSize];
    while(i < msgSize)
    {
      int numRead = _readFromSocket(&buffer[i], 1);
      if (numRead != 1) {
	delete[] buffer;
	return;
      }
      i++;
    }

    if(i == msgSize)
    {
      if(i > 0)
      {
	if(_params->debug)
	  cout << "Got message, size: " << i << endl;
	int xmlSize = (header.decompressed_size / sizeof(char)) + 1;
	char *xmlBuf = new char[xmlSize];
	int iret = _uncompressXml(buffer, msgSize, xmlBuf, xmlSize);
	if(iret == Z_STREAM_END) {
	  //cout << xmlBuf << endl << endl;
	  _processAsdiXml(xmlBuf);
	} else {
	  
	}
	delete[] xmlBuf;
      }
    } else {
      cerr << "ERROR: Failed to Get message, received: " << i << endl;
      break;
    }
    delete[] buffer;

  } while (true); // Infinite read loop.
 
  _closeSocket();

}

void asdiXml2spdb::_processHeader(char *buff, xml_header_t *header)
{
  char *ptr;
  /* Unpack data stream */
  ptr = buff;
  strncpy(header->timestamp, ptr, 16);
  ptr+= sizeof(header->timestamp);
  memcpy(&header->data_type, ptr, sizeof(header->data_type));
  ptr+= sizeof(header->data_type);
  memcpy(&header->sequence_number, ptr, sizeof(header->sequence_number));
  ptr+= sizeof(header->sequence_number);
  memcpy(&header->compressed_size, ptr, sizeof(header->compressed_size));
  ptr+= sizeof(header->compressed_size);
  memcpy(&header->decompressed_size, ptr, sizeof(header->decompressed_size));
  /* Convert to host format */
  header->data_type = BE_to_si32(header->data_type);
  header->sequence_number = BE_to_si32(header->sequence_number);
  header->compressed_size = BE_to_si32(header->compressed_size);
  header->decompressed_size = BE_to_si32(header->decompressed_size);
}

int asdiXml2spdb::_uncompressXml(char *buff, int buff_size, char *out_buff, int out_buff_size)
{
  z_stream infstream;
  infstream.zalloc = Z_NULL;
  infstream.zfree = Z_NULL;
  infstream.opaque = Z_NULL;
  infstream.avail_in = (uInt)buff_size; // size of input
  infstream.next_in = (Bytef *)buff; // input char array
  infstream.avail_out = (uInt)out_buff_size; // size of output
  infstream.next_out = (Bytef *)out_buff; // output char array

  inflateInit(&infstream);
  int iret = inflate(&infstream, Z_NO_FLUSH);

  if(_params->debug)
    cout << "Uncompressed XML, total size: " << infstream.total_out << endl;
  out_buff[infstream.total_out] = char(0);
  inflateEnd(&infstream);

  return iret;
}

////////////////////////////////////////////////////////////////
//
// Processs a message.
//
void asdiXml2spdb::_processAsdiXml(char *asdiMsg)
{

  // See if we wish to save the raw data
  // to an ASCII file.
  //
  if (_params->saveRawASCII){
    _save2ascii(NULL, asdiMsg, 1);
  }

  //
  // Return if we are not saving SPDB or parsed ascii.
  //
  if (!_params->saveSPDB && !_params->saveParsedASCII) {
    return;
  }

  int preamble_size = 0;
  bool tfmData = false;
  char preamble[MAX_HEADER_SIZE];
  string tmpFileName;
  int tmpNum = 0;
  char charTmpNum[15];
  preamble[0] = char(0);

  char *start = strstr( asdiMsg, "<?xml");
  if(start == NULL)
    return;
  char *end = strchr( start, '>');

  if(start == NULL|| end == NULL) {
    cerr << "ERROR: XML message missing starting preamble, cannot process. " << endl;
    return;
  } 
  strncpy(preamble, start, end-start+1);
  preamble[end-start+1] = char(0);
  preamble_size = end-start;

  start = strstr( asdiMsg, "<asdiOutput");
  if(start == NULL) {
    start = strstr( asdiMsg, "<ds:tfmDataService");
    tfmData = true;
  }
  end = strchr( start, '>');
  if(start == NULL || end == NULL) {
    cerr << "ERROR: XML message missing starting preamble, cannot process. " << endl;
    return;
  } 
  strncat(preamble, start, end-start+1);
  preamble[preamble_size+(end-start)+1] = char(0);
  preamble_size += end-start;

  char *xml = new char[MAX_XML_SIZE];
  xml[0] = char(0);

  if(tfmData) {
    start = strstr(asdiMsg, "<fdm:fltdMessage");
  } else {
    start = strstr(asdiMsg, "<asdiMessage");
  }
  while(start != NULL)
  {
    if(tfmData) {
      end = strstr(start, "</fdm:fltdMessage>");
    } else {
      end = strstr(start, "</asdiMessage>");
    }
    if(end == NULL) {
      cerr << "ERROR: XML message missing closing </asdiMessage> or </fdm:fltdMessage>, cannot process. " << endl;
    } else {
      if(tfmData) {
	strncpy(xml, start, end-start+18);
	xml[end-start+18] = char(0);
      } else {
	strncpy(xml, start, end-start+14);
	xml[end-start+14] = char(0);
      }

      if(_inputFile) {
	tmpFileName = _inputFile;
      } else {
	tmpFileName = _params->tmpFileName;
      }
      sprintf(charTmpNum, ".%d.tmp", tmpNum);
      tmpFileName += charTmpNum;

      FILE* fpTmpXml = fopen(tmpFileName.c_str() , "w" );
      if(fpTmpXml == NULL) { 
	cerr << "Error unable to create XML tmp file " << tmpFileName << endl;
	delete[] xml;
	return;  
      }

      fputs (xml, fpTmpXml);
      fflush (fpTmpXml);
      fclose(fpTmpXml);


      _doc.Clear();
      int loadOkay = _doc.LoadFile( tmpFileName.c_str() );


      if(!loadOkay) {
	cerr << "Error unable to read XML tmp file " << tmpFileName << endl;
	remove( tmpFileName.c_str() );
	delete[] xml;
	return;  
      }
      remove( tmpFileName.c_str() );

      TiXmlElement* root = _doc.RootElement();
      const char* facility = root->Attribute("sourceFacility");
      const char* time = root->Attribute("sourceTimeStamp");

      if(!time || !facility) {
	cerr << "Error missing asdiMessage tag information" << endl;
	delete[] xml;
	return;
      }

      date_time_t T;
      int A_year, A_month, A_day, A_hour, A_min, A_sec;
      if (6 != sscanf(time, "%4d-%2d-%2dT%2d:%2d:%2dZ",
		      &A_year, &A_month, &A_day, &A_hour, &A_min, &A_sec)) {
	cerr << "Error unknown time format or missing time: " << time << endl;
	delete[] xml;
	return;
      }

      T.year = A_year;
      T.month = A_month;
      T.day = A_day; 
      T.hour = A_hour; 
      T.min = A_min; 
      T.sec = A_sec;
      uconvert_to_utime( &T );

      //
      // Only Parse the messages we want.
      //
      TiXmlElement* TZ;
      if(tfmData)
	TZ = root->FirstChildElement("fdm:trackInformation");
      else
	TZ = root->FirstChildElement("trackInformation");
      if(TZ != NULL){
	_parseTZ(TZ, T);
      } else {
	TiXmlElement *FZ, *AF, *UZ, *DZ, *AZ, *TO, *RZ;
	if(tfmData) {
	  FZ = root->FirstChildElement("fdm:flightPlanInformation");
	  AF = root->FirstChildElement("fdm:flightPlanAmendmentInformation");
	  UZ = root->FirstChildElement("fdm:boundaryCrossingUpdate");
	  DZ = root->FirstChildElement("fdm:departureInformation");
	  AZ = root->FirstChildElement("fdm:arrivalInformation");
	  TO = root->FirstChildElement("fdm:oceanicReport");
	  RZ = root->FirstChildElement("fdm:flightPlanCancellation");
	} else {
	  FZ = root->FirstChildElement("flightPlanInformation");
	  AF = root->FirstChildElement("flightPlanAmendmentInformation");
	  UZ = root->FirstChildElement("boundaryCrossingUpdate");
	  DZ = root->FirstChildElement("departureInformation");
	  AZ = root->FirstChildElement("arrivalInformation");
	  TO = root->FirstChildElement("oceanicReport");
	  RZ = root->FirstChildElement("flightPlanCancellation");
	}

	//TiXmlElement* BZ = root->FirstChildElement("beaconCodeInformation");	
	//TiXmlElement* RT = root->FirstChildElement("flightManagementInformation");

	if(TO != NULL){
	  _parseTO(TO, T);
	} else if(DZ != NULL) {
	  _parseDZ(DZ, T);
	} else if(AZ != NULL) {
	  _parseAZ(AZ, T);
	} else if(FZ != NULL) {
	  _parseFZ(FZ, T);
	} else if(UZ != NULL) {
	  _parseUZ(UZ, T);
	} else if(RZ != NULL) {
	  _parseRZ(RZ, T);
	} else if(AF != NULL) {
	  _parseAF(AF, T);
	}

      }


    }
    if(tfmData) {
      start = strstr(start+1, "<fdm:fltdMessage");
    } else {
      start = strstr(start+1, "<asdiMessage");
    }
    tmpNum++;
    if(tmpNum < 0)
      tmpNum = 0;
  }

  delete[] xml;

  return;
}

//
// TZ - Track Message
// Provides a position update.
//
void asdiXml2spdb::_parseTZ(TiXmlElement* TZ, date_time_t T)
{
  
  TiXmlElement *aircraftIdEle = TZ->FirstChildElement("nxcm:aircraftId");
  if(!aircraftIdEle) {
     TiXmlElement *qualAirIdEle = TZ->FirstChildElement("nxcm:qualifiedAircraftId");
     if(!qualAirIdEle)
       return;
     aircraftIdEle = qualAirIdEle->FirstChildElement("nxce:aircraftId");
     if(!aircraftIdEle) 
       return;
  }
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  TiXmlElement *reportedAltEle = TZ->FirstChildElement("nxcm:reportedAltitude");
  if(!reportedAltEle)
    return;
  TiXmlElement *assignedAltEle = reportedAltEle->FirstChildElement("nxce:assignedAltitude");
  if(!assignedAltEle)
    return;
  TiXmlElement *simpleAltitudeEle = assignedAltEle->FirstChildElement("nxce:simpleAltitude");
  if(!simpleAltitudeEle)
    return;
  const char* altStr = simpleAltitudeEle->FirstChild()->Value();
  float altitude = _valueToFloat(simpleAltitudeEle);

  TiXmlElement *speedEle = TZ->FirstChildElement("nxcm:speed");
  if(!speedEle)
    return;
  double speed = _valueToFloat(speedEle);


  TiXmlElement *positionEle = TZ->FirstChildElement("nxcm:position");
  if(!positionEle)
    return;
  TiXmlElement *latitudeEle = positionEle->FirstChildElement("nxce:latitude");
  if(!latitudeEle)
    return;
  TiXmlElement *latitudeDMSEle = latitudeEle->FirstChildElement("nxce:latitudeDMS");
  if(!latitudeDMSEle)
    return;
  TiXmlElement *longitudeEle = positionEle->FirstChildElement("nxce:longitude");
  if(!longitudeEle)
    return;
  TiXmlElement *longitudeDMSEle = longitudeEle->FirstChildElement("nxce:longitudeDMS");
  if(!longitudeDMSEle)
    return;

  int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
  latitudeDMSEle->QueryIntAttribute("degrees", &latDeg);
  latitudeDMSEle->QueryIntAttribute("minutes", &latMin);
  if(latitudeDMSEle->QueryIntAttribute("seconds", &latSec) != TIXML_SUCCESS)
    latSec = 30;
  const char* latDirection = latitudeDMSEle->Attribute("direction");

  longitudeDMSEle->QueryIntAttribute("degrees", &lonDeg);
  longitudeDMSEle->QueryIntAttribute("minutes", &lonMin);
  if(longitudeDMSEle->QueryIntAttribute("seconds", &lonSec) != TIXML_SUCCESS)
    lonSec = 30;
  const char* lonDirection = longitudeDMSEle->Attribute("direction");

  double lat, lon;
  lat = latDeg + (latMin + (latSec / 60.0)) / 60.0;
  lon = lonDeg + (lonMin + (lonSec / 60.0)) / 60.0;
  if (latDirection[0] == 'S') lat = -lat;
  if (lonDirection[0] == 'W') lon = -lon;


  //
  // Return if we are applying a region and this is outside
  // of it.
  //
  if (_params->applyRegion){
    if (
	(lat > _params->region.latMax) ||
	(lat < _params->region.latMin) ||
	(lon > _params->region.lonMax) ||
	(lon < _params->region.lonMin)
	){
      return;
    }
  }
  
  ac_data_t A;
  memset(&A, 0, sizeof(A));

  //
  // Parse altitude 
  // If it ends in a 'T' then it is an altitude the aircraft has been
  //  cleared to, but is not at yet. 
  // If it starts with 'OTP/' as in OTP/350 then the aircraft is above 350. 
  // If it has a B then it is a lower and upper altitude, save the average.
  // If it ends in a 'C' then it is correct, but is not what the aircraft is
  // supposed to be at.
  // If it has none of these then it is the assigned altitude.
  //
  if(altStr[strlen(altStr)-1] == 'C') {
    A.alt_type = ALT_TRANSPONDER;
  } else if(altStr[strlen(altStr)-1] == 'T') {
    A.alt_type = ALT_INTERIM;
  } else if(strlen(altStr) == 3 || strlen(altStr) == 2) {
    A.alt_type = ALT_NORMAL;
  } 
  if(altitude < 0.0 || altitude > 1000)
    return;


  A.lat = lat; 
  A.lon = lon;
  A.ground_speed = speed;
  sprintf(A.callsign,"%s", fltID);

  if (altitude < 0.0){
    A.alt = AC_DATA_UNKNOWN_VALUE;
  } else {
    //
    // Convert from flight level (hundreds of feet) to Km.
    //
    A.alt = altitude*100.0*0.3048/1000.0;
  }

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  if (_params->saveParsedASCII) {
    char alt_type[5] = "U";
    if(A.alt_type == ALT_NORMAL)
      strcpy(alt_type, "N");
    else if(A.alt_type == ALT_TRANSPONDER)
      strcpy(alt_type, "T");
    else if(A.alt_type == ALT_INTERIM)
      strcpy(alt_type, "I");
    else if(A.alt_type == ALT_VFR_ON_TOP)
      strcpy(alt_type, "T");
    else if(A.alt_type == ALT_AVERAGE)
      strcpy(alt_type, "A");

    char buffer[256];
    sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' %s %.5f %.5f %.3d %.5d %s", 
	    T.year,T.month,T.day,T.hour,T.min,T.sec,A.callsign,A.lat,A.lon,
	    (int)A.ground_speed,(int)(altitude*100),alt_type);
    _save2ascii(&T, buffer, 2);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB) {
    return;
  }

  ac_data_to_BE( &A );


  _spdb.addPutChunk( dataType,
		     T.unix_time,
		     T.unix_time + _params->expiry,
		     sizeof(ac_data_t), &A,
		     dataType2 );
  //
  // Output data every N points.
  //
  _spdbBufferCount++;
  if (_spdbBufferCount == _params->nPosnWrite){
    _spdbBufferCount = 0;
    if (_spdb.put( _params->PosnOutUrl,
		   SPDB_AC_DATA_ID,
		   SPDB_AC_DATA_LABEL)){
      _spdb.clearPutChunks();
      fprintf(stderr,"ERROR: Failed to put posn data\n");
      return;
    }
    _spdb.clearPutChunks();
  }

  return;

}

//
// FZ - Flight Plan Information Message 
// Provides flight plan data for eligible flight plans.
//
void asdiXml2spdb::_parseFZ(TiXmlElement* FZ, date_time_t T)
{
  TiXmlElement *aircraftIdEle = FZ->FirstChildElement("nxcm:aircraftId");
  if(!aircraftIdEle) {
     TiXmlElement *qualAirIdEle = FZ->FirstChildElement("nxcm:qualifiedAircraftId");
     if(!qualAirIdEle)
       return;
     aircraftIdEle = qualAirIdEle->FirstChildElement("nxce:aircraftId");
     if(!aircraftIdEle) 
       return;
  }
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  TiXmlElement *aircraftTypeEle = FZ->FirstChildElement("nxcm:flightAircraftSpecs");
  if(!aircraftTypeEle)
    return;
  const char* typeStrC = aircraftTypeEle->FirstChild()->Value();
  const char* typeQualifierC = aircraftTypeEle->Attribute("equipmentQualifier");

  TiXmlElement *speedEle = FZ->FirstChildElement("nxcm:speed");
  if(!speedEle)
    return;
  TiXmlElement *trueSpeedEle = speedEle->FirstChildElement("nxce:filedTrueAirSpeed");
  int airSpeed;
  if(!trueSpeedEle) {
    TiXmlElement *machEle = speedEle->FirstChildElement("nxce:mach");
    airSpeed = _valueToInt(machEle);
    airSpeed =  ((float)airSpeed / 100) * MACH1;
  } else {
    airSpeed = _valueToInt(trueSpeedEle);
  }

  TiXmlElement *altitudeEle = FZ->FirstChildElement("nxcm:altitude");
  if(!altitudeEle)
    return;
  TiXmlElement *altEle = altitudeEle->FirstChildElement("nxce:requestedAltitude");
  if(!altEle) {
    altEle = altitudeEle->FirstChildElement("nxce:assignedAltitude");
    if(!altEle)
      return;
  }
  TiXmlElement *simpleAltitudeEle = altEle->FirstChildElement("nxce:simpleAltitude");
  if(!simpleAltitudeEle)
    return;
  float altitude = _valueToFloat(simpleAltitudeEle);
  // Convert from flight level (hundreds of feet) to Km.
  altitude = altitude*100.0*0.3048/1000.0;

  char fixStr[_internalStringLen];

  TiXmlElement *coordPointEle = FZ->FirstChildElement("nxcm:coordinationPoint");
  _getPointStr(coordPointEle, fixStr);

  TiXmlElement *coordTimeEle = FZ->FirstChildElement("nxcm:coordinationTime");
  if(!coordTimeEle)
    return;
  const char* fixTimeC = coordTimeEle->FirstChild()->Value();

  TiXmlElement *routeEle = FZ->FirstChildElement("nxcm:routeOfFlight");
  if(!routeEle)
    return;
  const char* routeStrC = routeEle->Attribute("legacyFormat");

  char callsign[12];
  char routeStr[_routeStringLen];
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];
  char typeStr[_internalStringLen];
  int arrTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;

  fixStr[0] = char(0);
  destStr[0] = char(0);
  deptStr[0] = char(0);


  strncpy(callsign, fltID, 12);
  strncpy(routeStr, routeStrC, _routeStringLen-1);
  strncpy(typeStr, typeStrC, _internalStringLen-1);

  int routeSize;
  ac_route_posn_t *routeArray = NULL;
  if(_params->decodeRoute) {
    //
    // Decode the route string
    //
    if(_route == NULL) 
      _route = new routeDecode(_params);
    _route->decodeRoute(routeStr);
    
    routeArray = _route->getRouteArray(&routeSize, &arrTime,
				       destStr, deptStr,_internalStringLen);
    if(routeArray == NULL)
      return;
    if(routeSize < 1 || routeSize > 1000) {
      fprintf(stderr,"WARNING: getRouteArray returned bad routeSize: %d \n", routeSize);
      return;
    }
    
    //
    // Return if we are applying a region and the route 
    // has no point in it.
    //
    if (_params->applyRegion) {
      bool entered = false;
      for(int a = 0; a < routeSize; a++) {
	if ((routeArray[a].lat < _params->region.latMax) &&
	    (routeArray[a].lat > _params->region.latMin) &&
	    (routeArray[a].lon < _params->region.lonMax) &&
	    (routeArray[a].lon > _params->region.lonMin)) {
	  entered = true;
	}
      }
      if(entered == false)
	return;
    }
  }
  

  if (_params->saveParsedASCII) {

    if(strlen(callsign) > 8)
      return;

    char buffer[_routeStringLen+256];
    if(arrTime != AC_ROUTE_MISSING_INT && _params->decodeRoute)
    {
      
      date_time_t arrT = T;
      arrT.hour = arrTime / 100;
      arrT.min = arrTime % 100;
      arrT.sec = 0;
      if(arrT.hour > 23 || arrT.min > 60)
	return;
      uconvert_to_utime( &arrT );
      if(T.unix_time > arrT.unix_time) {
	arrT.unix_time += 24*60*60;
	uconvert_from_utime( &arrT );
      }

      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' F %s %s %s %s \\N \\N '%d/%02d/%02d %02d:%02d:00' E '%s'", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, 
	      arrT.year,arrT.month,arrT.day,arrT.hour,arrT.min, routeStr);
    } else {
      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' F %s %s %s %s \\N \\N \\N \\N '%s'", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, routeStr );
    }
    _save2ascii(&T, buffer, 3);
  }


  // Return if we are not saving SPDB.
  if (!_params->saveSPDB || !_params->decodeRoute) {
    return;
  }

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_FlightPlan, ALT_NORMAL, routeSize,
				   arrTime, deptTime, altitude, airSpeed,
				   fixTime, fixStr, typeStr, destStr,
				   deptStr, callsign, routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(callsign, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  delete [] routeArray;
  free(ac_route);

  return;
}

void asdiXml2spdb::_parseAF(TiXmlElement* AF, date_time_t T)
{
  TiXmlElement *qualAircraftIdEle = AF->FirstChildElement("nxcm:qualifiedAircraftId");
  if(!qualAircraftIdEle)
    return;
  TiXmlElement *aircraftIdEle = qualAircraftIdEle->FirstChildElement("nxce:aircraftId");
  if(!aircraftIdEle)
    return;
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL) {
      return;
    }
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  char callsign[12];
  char routeStr[_routeStringLen];
  char typeStr[_internalStringLen];
  char fixStr[_internalStringLen];
  char timeStr[_internalStringLen];
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  float altitude = AC_ROUTE_MISSING_FLOAT;
  ac_data_alt_type_t alt_type = ALT_NORMAL;

  int arrTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];

  destStr[0] = char(0);
  deptStr[0] = char(0);
  routeStr[0] = char(0);
  typeStr[0] = char(0);
  fixStr[0] = char(0);
  timeStr[0] = char(0);


  strncpy(callsign, fltID, 12);

  TiXmlElement *departurePointEle = qualAircraftIdEle->FirstChildElement("nxce:departurePoint");
  _getPointStr(departurePointEle, deptStr);

  TiXmlElement *arrivalPointEle = qualAircraftIdEle->FirstChildElement("nxce:arrivalPoint");
  _getPointStr(arrivalPointEle, destStr);

  TiXmlElement *amendmentEle = AF->FirstChildElement("nxcm:amendmentData");
  if(!amendmentEle)
    return;

  TiXmlElement *routeEle = amendmentEle->FirstChildElement("nxcm:newRouteOfFlight");
  if(routeEle) {
    const char* routeStrC = routeEle->Attribute("legacyFormat");
    strncpy(routeStr, routeStrC, _routeStringLen);
  }

  TiXmlElement *aircraftTypeEle = amendmentEle->FirstChildElement("nxcm:newFlightAircraftSpecs");
  if(aircraftTypeEle) {
    const char* typeStrC = aircraftTypeEle->FirstChild()->Value();
    strncpy(typeStr, typeStrC, _internalStringLen-1);
  }

  TiXmlElement *speedEle = amendmentEle->FirstChildElement("nxcm:newSpeed");
  if(speedEle) {
    TiXmlElement *trueSpeedEle = speedEle->FirstChildElement("nxce:filedTrueAirSpeed");
    if(!trueSpeedEle) {
      TiXmlElement *machEle = speedEle->FirstChildElement("nxce:mach");
      airSpeed = _valueToInt(machEle);
      airSpeed =  ((float)airSpeed / 100) * MACH1;
    } else {
      airSpeed = _valueToInt(trueSpeedEle);
    }
  }

  TiXmlElement *altitudeEle = amendmentEle->FirstChildElement("nxcm:newAltitude");
  if(altitudeEle) {
    TiXmlElement *altEle = altitudeEle->FirstChildElement("nxce:requestedAltitude");
    if(!altEle) {
      altEle = altitudeEle->FirstChildElement("nxce:assignedAltitude");
      if(!altEle)
	return;
    }
    TiXmlElement *simpleAltitudeEle = altEle->FirstChildElement("nxce:simpleAltitude");
    if(!simpleAltitudeEle)
      return;
    altitude = _valueToFloat(simpleAltitudeEle);
    // Convert from flight level (hundreds of feet) to Km.
    altitude = altitude*100.0*0.3048/1000.0;
  }

  TiXmlElement *coordPointEle = amendmentEle->FirstChildElement("nxcm:newCoordinationPoint");
  _getPointStr(coordPointEle, fixStr);

  TiXmlElement *coordTimeEle = amendmentEle->FirstChildElement("nxcm:newCoordinationTime");
  if(coordTimeEle) {
    const char* fixTimeC = coordTimeEle->FirstChild()->Value();
    strncpy(timeStr, fixTimeC, _internalStringLen);
  }

  //
  // Does it have an updated route?
  //
  int routeSize = 0;
  ac_route_posn_t *routeArray = NULL;
  if(routeEle &&_params->decodeRoute) {
    //
    // Decode the route
    if(_route == NULL) 
      _route = new routeDecode(_params);
    _route->decodeRoute(routeStr);

    routeArray = _route->getRouteArray(&routeSize, &arrTime, destStr,
				       deptStr, _internalStringLen);

    if(routeArray == NULL)
      routeSize = 0;
    else if(routeSize < 1 || routeSize > 1000) {
      fprintf(stderr,"WARNING: getRouteArray returned bad routeSize: %d \n", routeSize);
      routeSize = 0;
    } else {
      // Return if we are applying a region and the route 
      // has no point in it.
      if (_params->applyRegion) {
	bool entered = false;
	for(int a = 0; a < routeSize; a++) {
	  if ((routeArray[a].lat < _params->region.latMax) &&
	      (routeArray[a].lat > _params->region.latMin) &&
	      (routeArray[a].lon < _params->region.lonMax) &&
	      (routeArray[a].lon > _params->region.lonMin)) {
	    entered = true;
	  }
	}
	if(entered == false)
	  return;
      }
    }
  }

  if (_params->saveParsedASCII) {

    if(strlen(typeStr) == 0)
      strcpy(typeStr, "\\N");

    char buffer[_routeStringLen+256];
    if(arrTime != AC_ROUTE_MISSING_INT)
    {
      
      date_time_t arrT = T;
      arrT.hour = arrTime / 100;
      arrT.min = arrTime % 100;
      arrT.sec = 0;
      if(arrT.hour > 23 || arrT.min > 60)
	return;
      uconvert_to_utime( &arrT );
      if(T.unix_time > arrT.unix_time) {
	arrT.unix_time += 24*60*60;
	uconvert_from_utime( &arrT );
      }

      if(!routeEle) {
	strcpy(routeStr, "\\N");
      }

      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' R %s %s %s %s \\N \\N '%d/%02d/%02d %02d:%02d:00' E '%s'", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, 
	      arrT.year,arrT.month,arrT.day,arrT.hour,arrT.min, routeStr);
    } else {
      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' R %s %s %s %s \\N \\N \\N \\N '%s'", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, routeStr );
    }
    _save2ascii(&T, buffer, 3);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB) {
    return;
  }
  if( routeSize == 0) {
    return;
  }

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Revised, alt_type, routeSize,
				   arrTime, deptTime, altitude, airSpeed,
				   fixTime, fixStr, typeStr, destStr,
				   deptStr, callsign, routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  delete [] routeArray;
  free(ac_route);

  return;
}

//
// UZ - ARTCC boundary crossing Message 
// Sent to provide current flight plan information on active
// eligible flights that enter an ARTCC.
//
void asdiXml2spdb::_parseUZ(TiXmlElement* UZ, date_time_t T)
{
  TiXmlElement *aircraftIdEle = UZ->FirstChildElement("nxcm:aircraftId");
  if(!aircraftIdEle) {
     TiXmlElement *qualAirIdEle = UZ->FirstChildElement("nxcm:qualifiedAircraftId");
     if(!qualAirIdEle)
       return;
     aircraftIdEle = qualAirIdEle->FirstChildElement("nxce:aircraftId");
     if(!aircraftIdEle) 
       return;
  }
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  TiXmlElement *aircraftTypeEle = UZ->FirstChildElement("nxcm:flightAircraftSpecs");
  if(!aircraftTypeEle)
    return;
  const char* typeStrC = aircraftTypeEle->FirstChild()->Value();
  const char* typeQualifierC = aircraftTypeEle->Attribute("equipmentQualifier");

  TiXmlElement *speedEle = UZ->FirstChildElement("nxcm:speed");
  if(!speedEle)
    return;
  TiXmlElement *trueSpeedEle = speedEle->FirstChildElement("nxce:filedTrueAirSpeed");
  int airSpeed;
  if(!trueSpeedEle) {
    TiXmlElement *machEle = speedEle->FirstChildElement("nxce:mach");
    airSpeed = _valueToInt(machEle);
    airSpeed =  ((float)airSpeed / 100) * MACH1;
  } else {
    airSpeed = _valueToInt(trueSpeedEle);
  }

  TiXmlElement *altitudeEle = UZ->FirstChildElement("nxcm:reportedAltitude");
  if(!altitudeEle)
    return;
  TiXmlElement *altEle = altitudeEle->FirstChildElement("nxce:requestedAltitude");
  if(!altEle) {
    altEle = altitudeEle->FirstChildElement("nxce:assignedAltitude");
    if(!altEle)
      return;
  }
  TiXmlElement *simpleAltitudeEle = altEle->FirstChildElement("nxce:simpleAltitude");
  if(!simpleAltitudeEle)
    return;
  float altitude = _valueToFloat(simpleAltitudeEle);
  // Convert from flight level (hundreds of feet) to Km.
  altitude = altitude*100.0*0.3048/1000.0;

  TiXmlElement *routeEle = UZ->FirstChildElement("nxcm:routeOfFlight");
  if(!routeEle)
    return;
  const char* routeStrC = routeEle->Attribute("legacyFormat");

  double lat, lon;
  char boundryStr[_internalStringLen];
  TiXmlElement *positionEle = UZ->FirstChildElement("nxcm:boundaryPosition");
  if(!positionEle)
    return;
  TiXmlElement *latitudeEle = positionEle->FirstChildElement("nxce:latitude");
  if(latitudeEle)
  {
    TiXmlElement *latitudeDMSEle = latitudeEle->FirstChildElement("nxce:latitudeDMS");
    if(!latitudeDMSEle)
      return;
    TiXmlElement *longitudeEle = positionEle->FirstChildElement("nxce:longitude");
    if(!longitudeEle)
      return;
    TiXmlElement *longitudeDMSEle = longitudeEle->FirstChildElement("nxce:longitudeDMS");
    if(!longitudeDMSEle)
      return;

    int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
    latitudeDMSEle->QueryIntAttribute("degrees", &latDeg);
    latitudeDMSEle->QueryIntAttribute("minutes", &latMin);
    if(latitudeDMSEle->QueryIntAttribute("seconds", &latSec) != TIXML_SUCCESS)
      latSec = 30;
    const char* latDirection = latitudeDMSEle->Attribute("direction");
    
    longitudeDMSEle->QueryIntAttribute("degrees", &lonDeg);
    longitudeDMSEle->QueryIntAttribute("minutes", &lonMin);
    if(longitudeDMSEle->QueryIntAttribute("seconds", &lonSec) != TIXML_SUCCESS)
      lonSec = 30;
    const char* lonDirection = longitudeDMSEle->Attribute("direction");
    
    lat = latDeg + (latMin + (latSec / 60.0)) / 60.0;
    lon = lonDeg + (lonMin + (lonSec / 60.0)) / 60.0;
    if (latDirection[0] == 'S') lat = -lat;
    if (lonDirection[0] == 'W') lon = -lon;

    sprintf(boundryStr,"%2d%2d%c/%3d%2d%c",
	    latDeg, latMin, latDirection[0],
	    lonDeg, lonMin, lonDirection[0]);
    
  } else {
    lat = -99.99;
    lon = -999.99;
    sprintf(boundryStr,"%2f", lat);
  }

  char routeStr[_routeStringLen];
  char typeStr[_internalStringLen];
  char callsign[12];
  
  strncpy(callsign, fltID, 12);
  strncpy(routeStr, routeStrC, _routeStringLen);
  strncpy(typeStr, typeStrC, _internalStringLen);


  int deptTime = AC_ROUTE_MISSING_INT;
  int arrTime = AC_ROUTE_MISSING_INT;
  int boundryTime = AC_ROUTE_MISSING_INT;
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];

  destStr[0] = char(0);
  deptStr[0] = char(0);

  int routeSize;
  ac_route_posn_t *routeArray = NULL;
  if(_params->decodeRoute) {
    //
    // Decode the route string
    //
    if(_route == NULL) 
      _route = new routeDecode(_params);
    _route->decodeRoute(routeStr);
    
    routeArray = _route->getRouteArray(&routeSize, &arrTime, destStr,
				       deptStr, _internalStringLen);
    if(routeArray == NULL)
      return;
    if(routeSize < 1 || routeSize > 1000) {
      fprintf(stderr,"WARNING: getRouteArray returned bad routeSize: %d \n", routeSize);
      return;
    }
    
    
    //
    // Return if we are applying a region and the route 
    // has no point in it.
    //
    if (_params->applyRegion) {
      bool entered = false;
      for(int a = 0; a < routeSize; a++) {
	if ((routeArray[a].lat < _params->region.latMax) &&
	    (routeArray[a].lat > _params->region.latMin) &&
	    (routeArray[a].lon < _params->region.lonMax) &&
	    (routeArray[a].lon > _params->region.lonMin)) {
	  entered = true;
	}
      }
      if(entered == false)
	return;
    }
  }

  if (_params->saveParsedASCII) {

    char buffer[_routeStringLen+256];
    if(arrTime != AC_ROUTE_MISSING_INT && _params->decodeRoute)
    {
      
      date_time_t arrT = T;
      arrT.hour = arrTime / 100;
      arrT.min = arrTime % 100;
      arrT.sec = 0;
      if(arrT.hour > 23 || arrT.min > 60)
	return;
      uconvert_to_utime( &arrT );
      if(T.unix_time > arrT.unix_time) {
	arrT.unix_time += 24*60*60;
	uconvert_from_utime( &arrT );
      }

      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' U %s %s %s %s \\N \\N '%d/%02d/%02d %02d:%02d:00' E '%s'", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, 
	      arrT.year,arrT.month,arrT.day,arrT.hour,arrT.min, routeStr);
    } else {
      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' U %s %s %s %s \\N \\N \\N \\N '%s'", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, routeStr );
    }
    _save2ascii(&T, buffer, 3);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB || !_params->decodeRoute) {
    return;
  }

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_BoundryCross, ALT_NORMAL, routeSize,
				   arrTime, deptTime, altitude, airSpeed,
				   boundryTime, boundryStr, typeStr, destStr,
				   deptStr, callsign, routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  delete [] routeArray;
  free(ac_route);

  return;
}

// AZ - Arrival Message 
// Arrival data for all eligible arriving flights.
//
void asdiXml2spdb::_parseAZ(TiXmlElement* AZ, date_time_t T)
{
  TiXmlElement *qualAircraftIdEle = AZ->FirstChildElement("nxcm:qualifiedAircraftId");
  if(!qualAircraftIdEle)
    return;
  TiXmlElement *aircraftIdEle = qualAircraftIdEle->FirstChildElement("nxce:aircraftId");
  if(!aircraftIdEle)
    return;
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  char callsign[12];
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];
  char deptTimeStr[_internalStringLen];
  char arrTimeStr[_internalStringLen];
  char arrTimeEst[2];

  strncpy(callsign, fltID, 12);

  TiXmlElement *departurePointEle = qualAircraftIdEle->FirstChildElement("nxce:departurePoint");
  _getPointStr(departurePointEle, deptStr);

  TiXmlElement *arrivalPointEle = qualAircraftIdEle->FirstChildElement("nxce:arrivalPoint");
  _getPointStr(arrivalPointEle, destStr);

  TiXmlElement *destTimeEle = AZ->FirstChildElement("nxcm:timeOfArrival");
  if(destTimeEle) {
    const char* destTimeStrC = destTimeEle->FirstChild()->Value();
    strncpy(arrTimeStr, destTimeStrC, _internalStringLen);
    const char* estimatedStrC = destTimeEle->Attribute("estimated");
    if(strstr(estimatedStrC, "true") == NULL)
      arrTimeEst[0] = 'A';
    else
      arrTimeEst[0] = 'E';
  }

  float altitude = AC_ROUTE_MISSING_FLOAT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int arrTime = AC_ROUTE_MISSING_INT;

  if (_params->saveParsedASCII) {

    date_time_t arrT;
    int A_year, A_month, A_day, A_hour, A_min, A_sec;
    if (6 != sscanf(arrTimeStr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
		    &A_year, &A_month, &A_day, &A_hour, &A_min, &A_sec)) {
      arrTimeStr[0] = char(0);
    }
    arrT.year = A_year;
    arrT.month = A_month;
    arrT.day = A_day; 
    arrT.hour = A_hour; 
    arrT.min = A_min; 
    arrT.sec = A_sec;
    uconvert_to_utime( &arrT );

    char buffer[256];
    sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' A %s \\N %s %s \\N \\N '%d/%02d/%02d %02d:%02d:%02d' %c", 
	    T.year,T.month,T.day,T.hour,T.min,T.sec,
	    callsign, deptStr, destStr, 
	    arrT.year,arrT.month,arrT.day,arrT.hour,arrT.min,arrT.sec,arrTimeEst[0]);
    _save2ascii(&T, buffer, 3);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB) {
    return;
  }

  char fixStr[_internalStringLen];
  char typeStr[_internalStringLen];
  fixStr[0] = char(0);
  typeStr[0] = char(0);
  ac_route_posn_t *routeArray = NULL;

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Arrival, ALT_NORMAL, 0,
				   arrTime, deptTime, altitude, 
				   airSpeed, fixTime,
				   fixStr, typeStr, destStr, deptStr, callsign, 
				   routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  return;
}

//
// DZ - Departure Message 
// Transmitted for all eligible initially activated flight plans when
//  the activation message is not from an adjacent NAS.
//
void asdiXml2spdb::_parseDZ(TiXmlElement* DZ, date_time_t T)
{
  TiXmlElement *qualAircraftIdEle = DZ->FirstChildElement("nxcm:qualifiedAircraftId");
  if(!qualAircraftIdEle)
    return;
  TiXmlElement *aircraftIdEle = qualAircraftIdEle->FirstChildElement("nxce:aircraftId");
  if(!aircraftIdEle)
    return;
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  char callsign[12];
  char typeStr[_internalStringLen];
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];
  char deptTimeStr[_internalStringLen];
  char deptTimeEst[2];
  char arrTimeStr[_internalStringLen];
  char arrTimeEst[2];

  strncpy(callsign, fltID, 12);

  TiXmlElement *departurePointEle = qualAircraftIdEle->FirstChildElement("nxce:departurePoint");
  _getPointStr(departurePointEle, deptStr);

  TiXmlElement *arrivalPointEle = qualAircraftIdEle->FirstChildElement("nxce:arrivalPoint");
  _getPointStr(arrivalPointEle, destStr);

  TiXmlElement *deptTimeEle = DZ->FirstChildElement("nxcm:timeOfDeparture");
  if(deptTimeEle) {
    const char* deptTimeStrC = deptTimeEle->FirstChild()->Value();
    strncpy(deptTimeStr, deptTimeStrC, _internalStringLen-1);
    const char* estimatedStrC = deptTimeEle->Attribute("estimated");
    if(strstr(estimatedStrC, "true") == NULL)
      deptTimeEst[0] = 'D';
    else
      deptTimeEst[0] = 'E';
  }

  TiXmlElement *destTimeEle = DZ->FirstChildElement("nxcm:timeOfArrival");
  if(destTimeEle) {
    const char* destTimeStrC = destTimeEle->FirstChild()->Value();
    strncpy(arrTimeStr, destTimeStrC, _internalStringLen);
    const char* estimatedStrC = destTimeEle->Attribute("estimated");
    if(strstr(estimatedStrC, "true") == NULL)
      arrTimeEst[0] = 'A';
    else
      arrTimeEst[0] = 'E';
  }

  TiXmlElement *aircraftTypeEle = DZ->FirstChildElement("nxcm:flightAircraftSpecs");
  if(!aircraftTypeEle)
    return;
  const char* typeStrC = aircraftTypeEle->FirstChild()->Value();
  const char* typeQualifierC = aircraftTypeEle->Attribute("equipmentQualifier");

  strncpy(typeStr, typeStrC, _internalStringLen);

  float altitude = AC_ROUTE_MISSING_FLOAT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int arrTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;

  if (_params->saveParsedASCII) {

    date_time_t deptT;
    int A_year, A_month, A_day, A_hour, A_min, A_sec;
    if (6 != sscanf(deptTimeStr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
		    &A_year, &A_month, &A_day, &A_hour, &A_min, &A_sec)) {
      cerr << "Error unknown time format or missing time: " << deptTimeStr << endl;
      return;
    }
    deptT.year = A_year;
    deptT.month = A_month;
    deptT.day = A_day; 
    deptT.hour = A_hour; 
    deptT.min = A_min; 
    deptT.sec = A_sec;
    uconvert_to_utime( &deptT );

    date_time_t arrT;
    if (6 != sscanf(arrTimeStr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
		    &A_year, &A_month, &A_day, &A_hour, &A_min, &A_sec)) {
      arrTimeStr[0] = char(0);
    }
    arrT.year = A_year;
    arrT.month = A_month;
    arrT.day = A_day; 
    arrT.hour = A_hour; 
    arrT.min = A_min; 
    arrT.sec = A_sec;
    uconvert_to_utime( &arrT );

    char buffer[256];
    
    if(strlen(arrTimeStr) > 0)
    {

      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' D %s %s %s %s '%d/%02d/%02d %02d:%02d:00' %c '%d/%02d/%02d %02d:%02d:%02d' %c", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, 
	      deptT.year,deptT.month,deptT.day,deptT.hour,deptT.min, deptTimeEst[0],
	      arrT.year,arrT.month,arrT.day,arrT.hour,arrT.min,arrT.sec, arrTimeEst[0]);
    } else {
      sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' D %s %s %s %s '%d/%02d/%02d %02d:%02d:%02d' %c \\N \\N", 
	      T.year,T.month,T.day,T.hour,T.min,T.sec,
	      callsign, typeStr, deptStr, destStr, 
	      deptT.year,deptT.month,deptT.day,deptT.hour,deptT.min,deptT.sec, deptTimeEst[0]);
    }
    _save2ascii(&T, buffer, 3);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB) {
    return;
  }

  char fixStr[_internalStringLen];
  fixStr[0] = char(0);
  ac_route_posn_t *routeArray = NULL;

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Departure, ALT_NORMAL, 0,
				   arrTime, deptTime, altitude, 
				   airSpeed, fixTime, 
				   fixStr, typeStr, destStr, deptStr, callsign, 
				   routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  return;
}

//
// RZ - Cancellation Message 
// Cancelation of flight for all eligible flight plans.
//
void asdiXml2spdb::_parseRZ(TiXmlElement* RZ, date_time_t T)
{
  TiXmlElement *qualAircraftIdEle = RZ->FirstChildElement("nxcm:qualifiedAircraftId");
  if(!qualAircraftIdEle)
    return;
  TiXmlElement *aircraftIdEle = qualAircraftIdEle->FirstChildElement("nxce:aircraftId");
  if(!aircraftIdEle)
    return;
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  char callsign[12];
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];

  strncpy(callsign, fltID, 12);

  TiXmlElement *departurePointEle = qualAircraftIdEle->FirstChildElement("nxce:departurePoint");
  _getPointStr(departurePointEle, deptStr);

  TiXmlElement *arrivalPointEle = qualAircraftIdEle->FirstChildElement("nxce:arrivalPoint");
  _getPointStr(arrivalPointEle, destStr);

  if (_params->saveParsedASCII) {
    char callsign[12];
    sprintf(callsign,"%s", fltID);

    char buffer[256];
    sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' C %s \\N %s %s \\N \\N \\N \\N", 
	    T.year,T.month,T.day,T.hour,T.min,T.sec,
	    callsign, deptStr, destStr);
    _save2ascii(&T, buffer, 3);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB) {
    return;
  }

  float altitude = AC_ROUTE_MISSING_FLOAT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int arrTime = AC_ROUTE_MISSING_INT;

  char fixStr[_internalStringLen];
  char typeStr[_internalStringLen];
  fixStr[0] = char(0);
  typeStr[0] = char(0);
  ac_route_posn_t *routeArray = NULL;

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Cancelled, ALT_NORMAL, 0,
				   arrTime, deptTime, altitude, 
				   airSpeed, fixTime, 
				   fixStr, typeStr, destStr, deptStr, callsign, 
				   routeArray, &ac_route_size);
  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(callsign, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  return;
}

// TO - Oceanic Position
// Provides sparse position reports while outside radar tracking via ARINC.
//
void asdiXml2spdb::_parseTO(TiXmlElement* TO, date_time_t T)
{
  TiXmlElement *qualAircraftIdEle = TO->FirstChildElement("nxcm:qualifiedAircraftId");
  if(!qualAircraftIdEle)
    return;
  TiXmlElement *aircraftIdEle = qualAircraftIdEle->FirstChildElement("nxce:aircraftId");
  if(!aircraftIdEle)
    return;
  const char* fltID = aircraftIdEle->FirstChild()->Value();

  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }
  if(_params->flightTypes != Params::ALL_TYPES) {
    if(_params->flightTypes == Params::COMMERCIAL_ONLY) {
      if(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57))
	return;
    } else 
      if(_params->flightTypes == Params::GA_ONLY) {
	if(!(fltID[0] == 'N' && fltID[1] >= char(48) && fltID[1] <= char(57)))
	  return;
      }
  }

  char callsign[12];
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];

  strncpy(callsign, fltID, 12);

  TiXmlElement *departurePointEle = qualAircraftIdEle->FirstChildElement("nxce:departurePoint");
  _getPointStr(departurePointEle, deptStr);

  TiXmlElement *arrivalPointEle = qualAircraftIdEle->FirstChildElement("nxce:arrivalPoint");
  _getPointStr(arrivalPointEle, destStr);

  TiXmlElement *speedEle = TO->FirstChildElement("nxcm:speed");
  if(!speedEle)
    return;
  double speed = _valueToFloat(speedEle);


  TiXmlElement *repPositionEle = TO->FirstChildElement("nxcm:reportedPositionData");
  if(!repPositionEle)
    return;
  TiXmlElement *positionEle = repPositionEle->FirstChildElement("nxce:position");
  if(!positionEle)
    return;
  TiXmlElement *latitudeEle = positionEle->FirstChildElement("nxce:latitude");
  if(!latitudeEle)
    return;
  TiXmlElement *latitudeDMSEle = latitudeEle->FirstChildElement("nxce:latitudeDMS");
  if(!latitudeDMSEle)
    return;
  TiXmlElement *longitudeEle = positionEle->FirstChildElement("nxce:longitude");
  if(!longitudeEle)
    return;
  TiXmlElement *longitudeDMSEle = longitudeEle->FirstChildElement("nxce:longitudeDMS");
  if(!longitudeDMSEle)
    return;

  int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
  latitudeDMSEle->QueryIntAttribute("degrees", &latDeg);
  latitudeDMSEle->QueryIntAttribute("minutes", &latMin);
  if(latitudeDMSEle->QueryIntAttribute("seconds", &latSec) != TIXML_SUCCESS)
    latSec = 00;
  const char* latDirection = latitudeDMSEle->Attribute("direction");

  longitudeDMSEle->QueryIntAttribute("degrees", &lonDeg);
  longitudeDMSEle->QueryIntAttribute("minutes", &lonMin);
  if(longitudeDMSEle->QueryIntAttribute("seconds", &lonSec) != TIXML_SUCCESS)
    lonSec = 00;
  const char* lonDirection = longitudeDMSEle->Attribute("direction");

  double lat, lon;
  lat = latDeg + (latMin + (latSec / 60.0)) / 60.0;
  lon = lonDeg + (lonMin + (lonSec / 60.0)) / 60.0;
  if (latDirection[0] == 'S') lat = -lat;
  if (lonDirection[0] == 'W') lon = -lon;

  char locStr[_internalStringLen];
  sprintf(locStr,"%02d%02d%c/%03d%02d%c",
	  latDeg, latMin, latDirection[0],
	  lonDeg, lonMin, lonDirection[0]);

  TiXmlElement *altitudeEle = repPositionEle->FirstChildElement("nxce:altitude");
  if(!altitudeEle)
    return;
  float altitude = _valueToFloat(altitudeEle);

  TiXmlElement *timeEle = repPositionEle->FirstChildElement("nxce:time");
  if(!timeEle)
    return;

  ac_data_t A;
  memset(&A, 0, sizeof(A));

  // Set client data type to letter O to 
  // distinguish Oceanic position.
  A.client_data_type = char(79);
  A.lat = lat; 
  A.lon = lon;
  A.ground_speed = speed;
  strncpy(A.callsign, fltID, AC_DATA_CALLSIGN_LEN);

  A.alt = altitude;
  A.alt_type = ALT_NORMAL;
  // Convert from flight level (hundreds of feet) to Km.
  A.alt = A.alt*100.0*0.3048/1000.0;

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Create a route based on the lat/lons available
  //
  int routeSize = 3;
  ac_route_posn_t routeArray[5];
  
  if(_params->decodeRoute)
  {
    TiXmlElement *plan1PositionEle = NULL;
    TiXmlElement *plan2PositionEle = NULL;

    TiXmlNode *plan1PositionNode = TO->IterateChildren("nxcm:plannedPositionData", NULL);
    if(plan1PositionNode) {
      plan1PositionEle = plan1PositionNode->ToElement();
      TiXmlNode *plan2PositionNode = TO->IterateChildren("nxcm:plannedPositionData", plan1PositionNode);
      if(plan2PositionNode)
	plan2PositionEle = plan2PositionNode->ToElement();
    }

    if(plan1PositionEle != NULL) {
      routeSize = 4;
      
      TiXmlElement *position1Ele = plan1PositionEle->FirstChildElement("nxce:position");
      if(!position1Ele)
	return;
      TiXmlElement *latitude1Ele = position1Ele->FirstChildElement("nxce:latitude");
      if(!latitude1Ele)
	return;
      TiXmlElement *latitudeDMS1Ele = latitude1Ele->FirstChildElement("nxce:latitudeDMS");
      if(!latitudeDMS1Ele)
	return;
      TiXmlElement *longitude1Ele = position1Ele->FirstChildElement("nxce:longitude");
      if(!longitude1Ele)
	return;
      TiXmlElement *longitudeDMS1Ele = longitude1Ele->FirstChildElement("nxce:longitudeDMS");
      if(!longitudeDMS1Ele)
	return;
      
      int lat1Deg, lon1Deg, lat1Min, lon1Min, lat1Sec, lon1Sec;
      latitudeDMS1Ele->QueryIntAttribute("degrees", &lat1Deg);
      latitudeDMS1Ele->QueryIntAttribute("minutes", &lat1Min);
      if(latitudeDMS1Ele->QueryIntAttribute("seconds", &lat1Sec) != TIXML_SUCCESS)
	lat1Sec = 00;
      const char* lat1Direction = latitudeDMS1Ele->Attribute("direction");
      
      longitudeDMS1Ele->QueryIntAttribute("degrees", &lon1Deg);
      longitudeDMS1Ele->QueryIntAttribute("minutes", &lon1Min);
      if(longitudeDMS1Ele->QueryIntAttribute("seconds", &lon1Sec) != TIXML_SUCCESS)
	lon1Sec = 00;
      const char* lon1Direction = longitudeDMS1Ele->Attribute("direction");
      
      routeArray[2].lat = lat1Deg + (lat1Min + (lat1Sec / 60.0)) / 60.0;
      routeArray[2].lon = lon1Deg + (lon1Min + (lon1Sec / 60.0)) / 60.0;
      if (lat1Direction[0] == 'S') routeArray[2].lat = -routeArray[2].lat;
      if (lon1Direction[0] == 'W') routeArray[2].lon = -routeArray[2].lon;
      
      char locStr1[_internalStringLen];
      sprintf(locStr1,"%02d%02d%c/%03d%02d%c",
	      lat1Deg, lat1Min, lat1Direction[0],
	      lon1Deg, lon1Min, lon1Direction[0]);
      strncpy(routeArray[2].name, locStr1, AC_ROUTE_N_POSNAME);
    
      if(plan2PositionEle != NULL) {
	routeSize = 5;
	
	TiXmlElement *position2Ele = plan2PositionEle->FirstChildElement("nxce:position");
	if(!position2Ele)
	  return;
	TiXmlElement *latitude2Ele = position2Ele->FirstChildElement("nxce:latitude");
	if(!latitude2Ele)
	  return;
	TiXmlElement *latitudeDMS2Ele = latitude2Ele->FirstChildElement("nxce:latitudeDMS");
	if(!latitudeDMS2Ele)
	  return;
	TiXmlElement *longitude2Ele = position2Ele->FirstChildElement("nxce:longitude");
	if(!longitude2Ele)
	  return;
	TiXmlElement *longitudeDMS2Ele = longitude2Ele->FirstChildElement("nxce:longitudeDMS");
	if(!longitudeDMS2Ele)
	  return;
	
	int lat2Deg, lon2Deg, lat2Min, lon2Min, lat2Sec, lon2Sec;
	latitudeDMS2Ele->QueryIntAttribute("degrees", &lat2Deg);
	latitudeDMS2Ele->QueryIntAttribute("minutes", &lat2Min);
	if(latitudeDMS2Ele->QueryIntAttribute("seconds", &lat2Sec) != TIXML_SUCCESS)
	  lat2Sec = 00;
	const char* lat2Direction = latitudeDMS2Ele->Attribute("direction");
	
	longitudeDMS2Ele->QueryIntAttribute("degrees", &lon2Deg);
	longitudeDMS2Ele->QueryIntAttribute("minutes", &lon2Min);
	if(longitudeDMS2Ele->QueryIntAttribute("seconds", &lon2Sec) != TIXML_SUCCESS)
	  lon2Sec = 00;
	const char* lon2Direction = longitudeDMS2Ele->Attribute("direction");
	
	routeArray[3].lat = lat2Deg + (lat2Min + (lat2Sec / 60.0)) / 60.0;
	routeArray[3].lon = lon2Deg + (lon2Min + (lon2Sec / 60.0)) / 60.0;
	if (lat2Direction[0] == 'S') routeArray[3].lat = -routeArray[3].lat;
	if (lon2Direction[0] == 'W') routeArray[3].lon = -routeArray[3].lon;
	
	char locStr2[_internalStringLen];
	sprintf(locStr2,"%02d%02d%c/%03d%02d%c",
		lat2Deg, lat2Min, lat2Direction[0],
		lon2Deg, lon2Min, lon2Direction[0]);
	strncpy(routeArray[3].name, locStr2, AC_ROUTE_N_POSNAME);

      }
    }
  }

  strncpy(A.origin, deptStr, AC_DATA_AIRPORT_LEN);
  strncpy(A.destination, destStr, AC_DATA_AIRPORT_LEN);

  if(_params->decodeRoute)
  {
    if(_route == NULL) 
      _route = new routeDecode(_params);

    //
    // Set the first and last points on the route
    // to the Departure and destination airports.
    //
    routeArray[0].lat = AC_ROUTE_MISSING_INT;
    routeArray[0].lon = AC_ROUTE_MISSING_INT;
    routeArray[routeSize-1].lat = AC_ROUTE_MISSING_INT;
    routeArray[routeSize-1].lon = AC_ROUTE_MISSING_INT;
    
    strncpy(routeArray[0].name, deptStr, AC_ROUTE_N_POSNAME);
    _route->lookupAirport(deptStr, &(routeArray[0].lat), &(routeArray[0].lon) );
    strncpy(routeArray[routeSize-1].name, destStr, AC_ROUTE_N_POSNAME);
    _route->lookupAirport(destStr, &(routeArray[routeSize-1].lat),
			  &(routeArray[routeSize-1].lon) );
    
    //
    // Set the second point on the route
    // to the current position.
    //
    routeArray[1].lat = A.lat;
    routeArray[1].lon = A.lon;
    strncpy(routeArray[1].name, locStr, AC_ROUTE_N_POSNAME);
  }

  if (_params->saveParsedASCII) {
    char alt_type[5] = "O";

    if(deptStr[0] == '-')
      strcpy(deptStr, "\\N");
    if(destStr[0] == '-')
      strcpy(destStr, "\\N");

    char buffer[256];
    sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' %s %.5f %.5f %.3d %.5d %s", 
	    T.year,T.month,T.day,T.hour,T.min,T.sec,A.callsign,A.lat,A.lon,
	    (int)A.ground_speed,(int)(altitude*100),alt_type);
    _save2ascii(&T, buffer, 2);

    sprintf(buffer,"'%d/%02d/%02d %02d:%02d:%02d' O %s \\N %s %s \\N \\N \\N \\N", 
	    T.year,T.month,T.day,T.hour,T.min,T.sec,
	    A.callsign, deptStr, destStr);
    _save2ascii(&T, buffer, 3);
  }

  // Return if we are not saving SPDB.
  if (!_params->saveSPDB) {
    return;
  }

  int deptTime = AC_ROUTE_MISSING_INT;
  int arrTime = AC_ROUTE_MISSING_INT;
  int boundryTime = AC_ROUTE_MISSING_INT;
  char typeStr[_internalStringLen];
  typeStr[0] = char(0);
  
  if(_params->decodeRoute)
  {
    //
    // Turn the route array into a ac_route for spdb
    //
    int ac_route_size;
    void *ac_route = create_ac_route(AC_ROUTE_Oceanic, ALT_NORMAL, routeSize,
				     arrTime, deptTime, A.alt, (int)A.ground_speed,
				     boundryTime, locStr, typeStr, destStr,
				     deptStr, callsign, routeArray, &ac_route_size);
    
    //
    // Save out to Spdb
    _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		     ac_route_size, ac_route, dataType2);
  }

  ac_data_to_BE( &A );

  _spdb.addPutChunk( dataType,
		     T.unix_time,
		     T.unix_time + _params->expiry,
		     sizeof(ac_data_t), &A,
		     dataType2 );

  //
  // Output data every N points.
  //
  _spdbBufferCount++;
  if (_spdbBufferCount == _params->nPosnWrite){
    _spdbBufferCount = 0;
    if (_spdb.put( _params->PosnOutUrl,
		   SPDB_AC_DATA_ID,
		   SPDB_AC_DATA_LABEL)){
      fprintf(stderr,"ERROR: Failed to put posn data\n");
      return;
    }
  }

  return;
}

void asdiXml2spdb::_getPointStr(TiXmlElement *element, char *str)
{
  str[0] = char(0);
  if(!element)
    return;

  TiXmlElement *fixEle = element->FirstChildElement("nxce:fix");
  if(fixEle)
    _getPointStr(fixEle, str);

  TiXmlElement *namedFixEle = element->FirstChildElement("nxce:namedFix");
  TiXmlElement *airportFixEle = element->FirstChildElement("nxce:airport");
  TiXmlElement *fixRadialEle = element->FirstChildElement("nxce:fixRadialDistance");
  TiXmlElement *latlonEle = element->FirstChildElement("nxce:latLong");

  if(namedFixEle) {
    const char* fixStrC = namedFixEle->FirstChild()->Value();
    strncpy(str, fixStrC, _internalStringLen-1);
  }
  if(airportFixEle) {
    const char* fixStrC = airportFixEle->FirstChild()->Value();
    strncpy(str, fixStrC, _internalStringLen);
  }
  if(fixRadialEle) {
    const char* fixStrC = fixRadialEle->FirstChild()->Value();
    strncpy(str, fixStrC, _internalStringLen);
  }
  if(latlonEle) {

    TiXmlElement *latitudeEle = latlonEle->FirstChildElement("nxce:latitude");
    if(!latitudeEle)
      return;
    TiXmlElement *latitudeDMSEle = latitudeEle->FirstChildElement("nxce:latitudeDMS");
    if(!latitudeDMSEle)
      return;
    TiXmlElement *longitudeEle = latlonEle->FirstChildElement("nxce:longitude");
    if(!longitudeEle)
      return;
    TiXmlElement *longitudeDMSEle = longitudeEle->FirstChildElement("nxce:longitudeDMS");
    if(!longitudeDMSEle)
      return;
    
    int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
    latitudeDMSEle->QueryIntAttribute("degrees", &latDeg);
    latitudeDMSEle->QueryIntAttribute("minutes", &latMin);
    if(latitudeDMSEle->QueryIntAttribute("seconds", &latSec) != TIXML_SUCCESS)
      latSec = 00;
    const char* latDirection = latitudeDMSEle->Attribute("direction");
    
    longitudeDMSEle->QueryIntAttribute("degrees", &lonDeg);
    longitudeDMSEle->QueryIntAttribute("minutes", &lonMin);
    if(longitudeDMSEle->QueryIntAttribute("seconds", &lonSec) != TIXML_SUCCESS)
      lonSec = 00;
    const char* lonDirection = longitudeDMSEle->Attribute("direction");
    
    sprintf(str,"%02d%02d%c/%03d%02d%c",
	    latDeg, latMin, latDirection[0],
	    lonDeg, lonMin, lonDirection[0]);
  }

}

/*
 *  Saves a ac_route to Spdb
 *  Returns true on success
 */

bool asdiXml2spdb::_saveRouteToSPDB(const si32 data_type, const time_t valid_time,
				 const time_t expire_time, const int chunk_len,
				 const void *chunk_data, const si32 data_type2)
{
  //
  // Add the route to the buffer
  _spdbRoute.addPutChunk(data_type, valid_time, expire_time, 
			 chunk_len, chunk_data, data_type2 );
  //
  // Output the buffer every N routes.
  //
  _spdbRouteBufferCount++;
  if (_spdbRouteBufferCount == _params->nRouteWrite){
    _spdbRouteBufferCount = 0;
    if (_spdbRoute.put( _params->RouteOutUrl, SPDB_AC_ROUTE_ID,
			SPDB_AC_ROUTE_LABEL)){
      _spdbRoute.clearPutChunks();
      fprintf(stderr,"ERROR: Failed to put route data\n");
      return false;
    }
    _spdbRoute.clearPutChunks();
  }
  return true;
}

/* 
 * Creates a two dataTypes based on flightID
 * first dataType is the last four characters
 * second dataType is the first four characters of the flightID character array.
 * Examples:
 *   "UAL224":     dataType="L224",  dataType2="UAL2"
 *   "DAL224":     dataType="L224",  dataType2="DAL2"
 *   "UAL59":      dataType="AL59",  dataType2="UAL5"
 *   "N423":       dataType="N423",  dataType2="N423"
 */
void asdiXml2spdb::_contriveDataType(const char *fltID, si32 *dataType, si32 *dataType2)
{
  
  if(strlen(fltID) < 4)
    *dataType = Spdb::hash4CharsToInt32(fltID);
  else
    *dataType = Spdb::hash4CharsToInt32( fltID + strlen(fltID) - 4);

  *dataType2 = Spdb::hash4CharsToInt32(fltID);

}


// Source: DevPinoy.org, Slightly modified
// Converts a hexadecimal string to integer
int asdiXml2spdb::_xtoi(const char* xs, unsigned int* result)
{
 size_t szlen = strlen(xs);
 int i, xv, fact;

 if (szlen > 0)
 {
  if (szlen < 4) return 2; // exit

  // Begin conversion here
  *result = 0;
  fact = 1;

  // Run until no more character to convert
  for(i=3; i>=0 ;i--)
  {
   if (isxdigit(*(xs+i)))
   {
    if (*(xs+i)>=97)
    {
     xv = ( *(xs+i) - 97) + 10;
    }
    else if ( *(xs+i) >= 65)
    {
     xv = (*(xs+i) - 65) + 10;
    }
    else
    {
     xv = *(xs+i) - 48;
    }
    *result += (xv * fact);
    fact *= 16;
   }
   else
   {
    // Conversion was abnormally terminated
    // by non hexadecimal digit, hence
    // returning only the converted with
    // an error value 4 (illegal hex character)
    return 4;
   }
  }
 } else {
   // Nothing to convert
   return 1;
 }
 return 0;
}

void asdiXml2spdb::_save2ascii(date_time_t *fT, char *asdiMsg, int extN)
{

  date_time_t T;
  if(fT == NULL) 
  {
    T.unix_time = time(NULL);
    uconvert_from_utime(&T);
  } else {
    T.year = fT->year;
    T.month = fT->month;
    T.day = fT->day; 
    T.hour = fT->hour; 
    T.min = fT->min;
    T.sec = fT->sec;
  }
    
  //
  // Make the output directory.
  //
  char outdirname[MAX_PATH_LEN];
  sprintf(outdirname,"%s/%d%02d%02d",
	  _params->OutASCIIdir,
	  T.year, T.month, T.day);
  
  if (ta_makedir_recurse(outdirname)){
    fprintf(stderr,"ERROR: Failed to make directory %s\n",
	    outdirname);
    return;
  }
    
  char ext[7] = "xml";
  if(extN == 2) 
    strcpy(ext, "ascii");
  else if(extN == 3)
    strcpy(ext, "ascii2");
  
  //
  // Open the output file append.
  //
  char outfilename[MAX_PATH_LEN];
  sprintf(outfilename,"%s/%d%02d%02d_%02d.%s", outdirname, T.year, T.month, T.day, T.hour, ext);
  FILE *fp;
  fp = fopen(outfilename, "a");
  if (fp == NULL){
    fprintf(stderr,"ERROR: Failed to create file %s\n",
	    outfilename);
    return;
  }
  
  //
  // Write the message and return.
  //
  fprintf(fp,"%s\n", asdiMsg);
  fclose(fp);

  return;
}

/////////////////////////////////////
//
// Small method to extract a space-delimited substring
//
void asdiXml2spdb::_extractString(char *instring, char *outstring)
{

  while(instring[0] == ' ') {
    instring++;
  }
  int j=0;
  do {
    if (instring[j] == ' ')
      outstring[j] = char(0);
    else
      outstring[j]=instring[j];
    j++;
  } while (
	   (outstring[j-1] != char(0)) &&
	   (j < _internalStringLen-1)
	   );

  if (j == _internalStringLen-1)
    outstring[j] = char(0);

  return;
}

int asdiXml2spdb::_valueToInt(const TiXmlElement* element)
{
  if(element == NULL || element->FirstChild() == NULL)
    return -999;
  const char *value = element->FirstChild()->Value();
  if(value == NULL)
    return -999;
  if(value[0] >= '-' || (value[0] >= '0' && value[0] <= '9'))
  {
    return atoi(value);
  }
  return -999;
}

float asdiXml2spdb::_valueToFloat(const TiXmlElement* element)
{
  if(element == NULL || element->FirstChild() == NULL)
    return -999.99;
  const char *value = element->FirstChild()->Value();
  if(value == NULL)
    return -999.99;
  if(value[0] >= '-' || (value[0] >= '0' && value[0] <= '9'))
  {
    return atof(value);
  }
  return -999.99;
}

const char* asdiXml2spdb::_valueToChar(const TiXmlElement* element)
{
  if(element == NULL || element->FirstChild() == NULL)
    return NULL;
  const char *value = element->FirstChild()->Value();
  if(value == NULL)
    return NULL;
  return value;
}

////////////////////////////////////////////////////////////////
//
// Open stream.
//
int asdiXml2spdb::_openSocket()
{
  int retVal = 0;
  char registrationMsg[50];

  strcpy (registrationMsg, "ID=asdi_ucar1,PASSWORD=B4YusP5t#");
  retVal = _S.open(_params->hostname, _params->port);

  if(_params->debug)
    cout << "Opening Socket.." << endl;

  if (retVal != 0)
  {
    fprintf(stderr,"ERROR: Attempt to open port %d at %s returned %d\n",
	    _params->port, _params->hostname, retVal);
    
    fprintf(stderr,"Error string : %s\n",_S.getErrString().c_str());
    
    switch(_S.getErrNum())
    {
      
    case Socket::UNKNOWN_HOST :
      fprintf(stderr,"ERROR: Unknown host.\n");
      break;
      
    case Socket::SOCKET_FAILED :
      fprintf(stderr,"ERROR: Could not set up socked (maxed out decriptors?).\n");
      break;
      
    case Socket::CONNECT_FAILED :
      fprintf(stderr,"ERROR: Socket Connect failed.\n");
      break;
      
    case Socket::TIMED_OUT :
      fprintf(stderr,"ERROR: Socket Timed out..\n");
      break;
      
    case Socket::UNEXPECTED :
      fprintf(stderr,"ERROR: Unexpected socket error.\n");
      break;
      
    default :
      fprintf(stderr,"ERROR: Unknown socket error.\n");
      break;
      
    }


  } else {
    if(_params->debug)
      cout << "Sending registration message ..." << endl;
    retVal = _S.writeBuffer(registrationMsg, 33);

    if (retVal != 0) {
      fprintf(stderr,"ERROR: Failed to send registration message, returned %d\n", retVal);
    }

  }
  if(retVal != 0)
    _S.close();
  return retVal;
}

////////////////////////////////////////////////////////////////
//
// Close socket.
//
void asdiXml2spdb::_closeSocket()
{
  _S.close();
  _S.freeData();
}

///////////////////////////////////////////////////
//
// Read N bytes from the input socket.
// Size of buffer to read to not checked.
// Returns number of bytes read.
//
int asdiXml2spdb::_readFromSocket(char *buffer, int numbytes)
{

  int retVal;
  if (_S.readSelectPmu(60))
  {

    switch (_S.getErrNum())
    {
      
    case Socket::TIMED_OUT :
      fprintf(stderr,"ERROR: Read select timed out.\n");
      exit(-1);
      break;
      
    case Socket::SELECT_FAILED :
      fprintf(stderr,"ERROR: Read select failed.\n");
      exit(-1);
      break;
      
    case Socket::UNEXPECTED :
      fprintf(stderr,"ERROR: Read select - unexpected error.\n");
      exit(-1);
      break;
      
    default :
      fprintf(stderr,"ERROR: Unkown error with read select.\n");
      exit(-1);
      break;
      
    }
  }

  if (_S.readBufferHb(buffer, numbytes, numbytes,
		      (Socket::heartbeat_t)PMU_auto_register, 3600) != 0 )
  {
    
    switch (_S.getErrNum())
    {
      
    case Socket::TIMED_OUT :
      fprintf(stderr,"ERROR: Read buffer timed out.\n");
      exit(-1);
      break;
      
    case Socket::BAD_BYTE_COUNT :
      fprintf(stderr,"ERROR: Read buffer gave bad byte count.\n");
      fprintf(stderr, _S.getErrString().c_str() );
      fprintf(stderr,"\n");
      _S.close();
      return(-1);
      break;
      
    default :
      fprintf(stderr,"ERROR: Unkown error with read buffer.\n");
      exit(-1);
      break;
    }
    
    return -1;
  }

  retVal = _S.getNumBytes();
  
  return retVal;
}
