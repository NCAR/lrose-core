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
/////////////////////////////////////////////////////////////////////////
//  ReadGemtronik handles reading the actual gemtronik file.
//
//  Once parsed and/or converted to a format we need the data
//  is then passed off to the 
//
//  $Id: ReadGemtronik.cc,v 1.17 2016/03/07 01:23:00 dixon Exp $
//
////////////////////////////////////////////////////////////////////////

#include <zlib.h>

#include "SweepData.hh"
#include "ReadGemtronik.hh"

const float RAYS_MIN = 0.0;
const float RAYS_MAX = 360.0;
const float RAYS_SCALE = 0.00549316;

const long int MAX_XML_SIZE = 100000;

ReadGemtronik::ReadGemtronik(Params *P)
{
  params = P;
  ncOutput = new NcOutput( params );
  _nBlobs = 0;
  initBlobs();
}

ReadGemtronik::~ReadGemtronik()
{
  clearSweeps();
  delete ncOutput;
}

int ReadGemtronik::readFile(char *fileName, time_t volumeTime, bool uncompress, 
			    ReadGemtronik::VolumeEnum_t fileType)
{

  char *header = NULL;
  int ret = readBlobs(fileName, &header);
  if(ret != 0 || header == NULL) {
    POSTMSG( ERROR, "Error Parsing blobs for file %s\n", fileName);
    return (-1);
  }

  char *tmpFileName;
  if(strlen(params->tmp_dir) > 0) {
    char *pch = strrchr(fileName, '/');
    if(pch == NULL) {
      pch = fileName;
    } else {
      pch = pch+1;
    }
    tmpFileName = new char[strlen(params->tmp_dir)+strlen(pch)+6];
    strcpy(tmpFileName, params->tmp_dir);
    strcat(tmpFileName, "/");
    strcat(tmpFileName, pch);
    strcat(tmpFileName, ".xml");

  } else {
    tmpFileName = new char[strlen(fileName)+5];
    strcpy(tmpFileName, fileName);
    strcat(tmpFileName, ".xml");

  }

  FILE* fpHeader = fopen( tmpFileName, "w" );
  if(fpHeader == NULL)
  { 
    POSTMSG( ERROR, "Error unable to create XML header file %s\n", tmpFileName);
    return (-1);  
  }
  fputs (header, fpHeader);
  fflush (fpHeader);
  int putOK = fputs (header, fpHeader);
  if (putOK < 0 || putOK == EOF)
  {
     POSTMSG( ERROR, "code: %d writting header information to file %s\n %s\n ",
  			putOK, fileName);
     perror ("The following fputs error occurred");
  }
  fclose(fpHeader);
  delete[] header;

  // _doc = new XMLDocument();
  _doc.Clear();
  int loadOkay = _doc.LoadFile( tmpFileName );
  delete[] tmpFileName;
  if(!loadOkay) {
    POSTMSG( ERROR, "code: %d while parsing xml for file %s\n %d %s\n", 
	     loadOkay, fileName, _doc.ErrorId(), _doc.ErrorDesc());
    
    return (-1);
  }
  //_doc.Print();

  TiXmlElement* root = _doc.RootElement();
  const char* type = root->Attribute("type");
  if(strcmp(type, "vol") != 0 && strcmp(type, "azi") != 0) {
    POSTMSG( ERROR, "Unknown file type : %s\n", fileName);
    _doc.Clear();
    return -1;
  }

  TiXmlElement* info = root->FirstChildElement("radarinfo");
  if(info == NULL) {
    POSTMSG( ERROR, "file does not contain radarinfo, %s\n", fileName);
    _doc.Clear();
    return -1;
  }
  info->QueryFloatAttribute("alt", &_alt);
  info->QueryFloatAttribute("lat", &_lat);
  info->QueryFloatAttribute("lon", &_lon);
  _waveLength = valueToFloat(info->FirstChildElement("wavelen"));
  _beamWidth = valueToFloat(info->FirstChildElement("beamwidth"));
  const char*radarid = info->Attribute("id");
  TiXmlElement* name = info->FirstChildElement("name");
  if(radarid == NULL || name == NULL) {
    POSTMSG( ERROR, "file does not contain radarid / radarname, %s\n", fileName);
    _doc.Clear();
    return -1;
  }
  const char* radarName = name->FirstChild()->Value();

  TiXmlElement* scan = root->FirstChildElement("scan");
  if(scan == NULL) {
    POSTMSG( ERROR, "file does not contain scan, %s\n", fileName);
    return -1;
  }

  TiXmlElement* pargroup = scan->FirstChildElement("pargroup");
  if(pargroup == NULL) {
    POSTMSG( ERROR, "file does not contain pargroup, %s\n", fileName);
    return -1;
  }

  const char* refid = pargroup->Attribute("refid");
  if(refid == NULL || strcmp(refid, "sdfbase") != 0) {
    POSTMSG( ERROR, "file does not contain refid sdfbase, %s\n", fileName);
    return -1;
  }

  int numele = valueToInt(pargroup->FirstChildElement("numele"));
  POSTMSG( DEBUG, "Num Elevations:  %d\n", numele); 
  if(numele < 1 || numele > 29) {
    POSTMSG( ERROR, "number of elevations is %d, %s\n", numele, fileName);
    return -1;
  }

  ncOutput->setVolStartTime( volumeTime );
  ncOutput->setBaseTime( volumeTime, 0 );
  ncOutput->setRadarInfo(radarid, _alt, _lat, _lon,
			 params->radarId, radarName);

  if(_sweeps.size() != 0 && _sweeps.size() != numele)
  {
    for(int a = 0; a < _sweeps.size(); a++)
      delete _sweeps[a];
    _sweeps.clear();
  }
  if(_sweeps.size() == 0)
    for(int ele = 0; ele < numele; ele++)
    {
      _sweeps.push_back(new SweepData( params ));
    }

  int numSlice = 0;
  int longpulse = 0;
  float startrange = valueToFloat(pargroup->FirstChildElement("start_range"));
  float rangestep = valueToFloat(pargroup->FirstChildElement("rangestep"));
  int highprf = valueToInt(pargroup->FirstChildElement("highprf"));
  int lowprf = valueToInt(pargroup->FirstChildElement("lowprf"));
  int antspeed = valueToInt(pargroup->FirstChildElement("antspeed"));
  float timesamp = valueToInt(pargroup->FirstChildElement("timesamp"));
  float snr_thresh = valueToFloat(pargroup->FirstChildElement("log"));
  float stagger;
  const char *staggerC = valueToChar(pargroup->FirstChildElement("stagger"));
  const char *pulsewidthC = valueToChar(pargroup->FirstChildElement("pulsewidth"));
  if(staggerC == NULL || strcmp(staggerC, "None") == 0)
    stagger = 0.0;
  else
    if(strcmp(staggerC, "4/3") == 0)
      stagger = 4.0 / 3.0;
  if(pulsewidthC != NULL && strcmp(pulsewidthC, "Long") == 0)
    longpulse = 1;

  TiXmlElement *slice = scan->FirstChildElement("slice");
  while(slice != NULL)
  {

//	      if(strcmp(slice->Name(), "slice") == 0 )
//		          {

    //int sliceNum = slice->IntAttribute("refid");
    int sliceNum;
    if (slice->QueryIntAttribute("refid", &sliceNum) != TIXML_SUCCESS) {
	    POSTMSG( ERROR, "file does not contain slicedata, %s\n", fileName);
    };

    float angle = valueToFloat(slice->FirstChildElement("posangle"));
    POSTMSG( DEBUG, "Slice: %d  %f\n", sliceNum, angle);

    int highprfT = valueToInt(slice->FirstChildElement("highprf"));
    int lowprfT = valueToInt(slice->FirstChildElement("lowprf"));
    const char *staggerT = valueToChar(slice->FirstChildElement("stagger"));
    const char *pulsewidthT = valueToChar(slice->FirstChildElement("pulsewidth"));
    int antspeedT = valueToInt(slice->FirstChildElement("antspeed"));
    int timesampT = valueToInt(slice->FirstChildElement("timesamp"));
    float startrangeT = valueToFloat(slice->FirstChildElement("start_range"));
    float rangestepT = valueToFloat(slice->FirstChildElement("rangestep"));
    float snr_threshT = valueToFloat(slice->FirstChildElement("log"));
    if(highprfT > -990)
	highprf = highprfT;
    if(lowprfT > -990)
	lowprf = lowprfT;
    if(antspeedT > -990)
	antspeed = antspeedT;
    if(timesampT > -990)
	timesamp = timesampT;
    if(startrangeT > -990)
	startrange = startrangeT;
    if(rangestepT > -990)
	rangestep = rangestepT ;
    if(snr_threshT > -990)
	snr_thresh = snr_threshT ;

    if(rangestep > 0 && rangestep <= 1)
	rangestep *= 1000;
    if(startrange < -990)
	startrange = rangestep / 2.0;
    if(pulsewidthT != NULL && strcmp(pulsewidthT, "Long") == 0)
	longpulse = 1;
    if(pulsewidthT != NULL && strcmp(pulsewidthT, "Short") == 0)
	longpulse = 0;
    if(staggerT == NULL || strcmp(staggerT, "None") == 0)
	stagger = 0.0;
    else
	if(strcmp(staggerT, "4/3") == 0)
	  stagger = 4.0 / 3.0;
    if(stagger != 0.0)
    {
	timesamp = timesamp + (timesamp * stagger);
    }

    if(stagger != 0.0 && lowprf > -990)
    {
	float a_prt = ((1.0 / highprf) + (1.0 / lowprf) ) / 2.0;
	highprf = 1.0 / a_prt;
	lowprf = 1.0 / a_prt;
    }

    if(timesamp < -990 && antspeed > -990 && highprf > -990) {
	float degrees = (float)antspeed / highprf;
	float pulse_count = 1.0 / degrees;
	timesamp = pulse_count;
    }

    TiXmlElement* slicedata = slice->FirstChildElement("slicedata");
    if(slicedata == NULL) {
	POSTMSG( ERROR, "file does not contain slicedata, %s\n", fileName);
	memDelete();
	sweepsMemDelete(numele);
	return -1;
    }

    TiXmlElement* rayinfo = slicedata->FirstChildElement("rayinfo");
    TiXmlElement* rawdata = slicedata->FirstChildElement("rawdata");
    if(rayinfo == NULL || rawdata == NULL) {
	POSTMSG( ERROR, "file does not contain rayinfo/rawdata, %s\n", fileName);
	memDelete();
	sweepsMemDelete(numele);
	return -1;
    }

    const char* scanTime = slicedata->Attribute("time");
    const char* scanDate = slicedata->Attribute("date");
    const char* dataTypeStr = rawdata->Attribute("type");
    int rayBlobId;
    int raydepth;
    int nrays;
    int dataBlobId;
    int nbins;
    float datamin;
    float datamax;
    int datadepth;
    if (rayinfo->QueryIntAttribute("blobid", &rayBlobId) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain ray blob id information, %s\n", fileName);
    };

    if (rayinfo->QueryIntAttribute("depth", &raydepth) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain blob id information, %s\n", fileName);
    };
    if (rayinfo->QueryIntAttribute("rays", &nrays) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain nrays information, %s\n", fileName);
    };
    if (rawdata->QueryIntAttribute("blobid", &dataBlobId) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain blob data id information, %s\n", fileName);
    };
    if (rawdata->QueryIntAttribute("bins", &nbins) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain number of bins information, %s\n", fileName);
    };
    if (rawdata->QueryFloatAttribute("min", &datamin) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain data minimum information, %s\n", fileName);
    };
    if (rawdata->QueryFloatAttribute("max", &datamax) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain data maximum information, %s\n", fileName);
    };
    if (rawdata->QueryIntAttribute("depth", &datadepth) != TIXML_SUCCESS) {
	POSTMSG( ERROR, "file does not contain data depth information, %s\n", fileName);
    };

    VolumeEnum_t dataType = VOLUMEUNKNOWN;
    if(strcmp(dataTypeStr, "dBZ") == 0) {
      dataType = REFLECTIVITY;
    } else if(dataTypeStr[0] == 'V') {
      dataType = VELOCITY;
    } else if(dataTypeStr[0] == 'W') {
      dataType = SPECTRUMWIDTH;
    }

    _sweeps[sliceNum]->setInfo(sliceNum, angle, scanDate, scanTime, nrays, _beamWidth, _waveLength);

    if(raydepth != 16 || datadepth != 8) {
	POSTMSG( ERROR, "raydepth (%d) != 16 or datadepth (%d) != 8, %s\n",
		 raydepth, datadepth, fileName);
        memDelete();
	_doc.Clear();
        sweepsMemDelete(numele);

	return -1;
     }

     float datascale = getScaling(datamin, datamax, datadepth);
     unsigned long int rayDataSize;
      
     unsigned char* rayData = getBlob(rayBlobId, &rayDataSize, (long)nrays * (raydepth / 8));
     float *rays = convertRayData(rayData, rayDataSize, nrays);

     unsigned long int dataSize;
     unsigned char* data = getBlob(dataBlobId, &dataSize, (long)nrays * (long)nbins * (datadepth / 8));

     _sweeps[sliceNum]->setData(dataType, data, dataSize, rays, nrays, nbins, datamin, datascale,
				startrange, rangestep, scanDate, scanTime, timesamp, stagger, 
				highprf, snr_thresh, longpulse);
     delete[] rays;

     slice = slice->NextSiblingElement();
  }

  memDelete();
  if(_sweeps[0]->complete())
  {
    if(params->combineSweeps)
    {
      if(longpulse == 1)
      {
	for(int a = 0; a < _prevSweeps.size(); a++)
	  delete _prevSweeps[a];
	_prevSweeps.clear();
	_prevSweeps = _sweeps;
	_sweeps.clear();
      } else {
	for(int a = 0; a < _prevSweeps.size(); a++)
	  for(int b = 0; b < _sweeps.size(); b++)
	    if(_prevSweeps[a]->getLongPulse() == 1 && _prevSweeps[a]->getElevation() == _sweeps[b]->getElevation())
	    {
	      _sweeps[b]->mergeDbz(_prevSweeps[a]);
	      POSTMSG( DEBUG, "Merging long pulse elevation %f\n", _prevSweeps[a]->getElevation()); 
	      delete _prevSweeps[a];
	      _prevSweeps.erase(_prevSweeps.begin()+a);
	    }
	finishVolume();
      }
    } else {
      finishVolume();
    }
  }

  if(_sweeps.size() != 0 && _sweeps.size() != numele)
  {
    for(int a = 0; a < _sweeps.size(); a++)
      delete _sweeps[a];
    _sweeps.clear();
  }
  if(_sweeps.size() == 0)
    for(int ele = 0; ele < numele; ele++)
    {
      _sweeps.push_back(new SweepData( params ));
    }

  return 0;
}

void ReadGemtronik::finishVolume()
{
  for(int slice = 0; slice < _sweeps.size(); slice++)
  {
    _sweeps[slice]->finishSweep();
    if( ncOutput->writeFile( _sweeps[slice] ) != 0 ) {
      POSTMSG( ERROR, "Failed to write out NetCdf file '%s'\n", ncOutput->getFileName() );
    }

  }

}

void ReadGemtronik::clearSweeps()
{
  for(int a = 0; a < _prevSweeps.size(); a++)
    delete _prevSweeps[a];
  _prevSweeps.clear();
  for(int a = 0; a < _sweeps.size(); a++)
    delete _sweeps[a];
  _sweeps.clear();
}

unsigned char* ReadGemtronik::getBlob(int id, unsigned long int* size, long int elementsSize)
{
  unsigned long int bSize = _blobSize[id];
  int bComp = _blobComp[id];
  unsigned char* bData = _blobData[id];

  if(bData == NULL) {
    printf( "Error requested BLOB does not exist\n" );
    return NULL;
  }

  if(bComp == 1)
  {
    unsigned char* uData;
    unsigned long int expectedSize = (bData[0] << 24) | (bData[1] << 16) | (bData[2] <<  8) | (bData[3]);
    if(elementsSize > expectedSize)
      expectedSize = elementsSize;
    unsigned long int actualSize = expectedSize;

    if ((uData = (unsigned char*)calloc(expectedSize, sizeof(unsigned char))) == NULL) {
      printf( "Error uncompressing BLOB, Out of Memory\n" );
      return NULL;
    }
    int res = uncompress( uData, &actualSize, bData + 4, bSize - 4 );

    if(res != Z_OK)
    {
      free(uData);

      if(res == Z_MEM_ERROR)
	printf( "Z_MEM_ERROR: Not enough memory\n" );
      if(res == Z_DATA_ERROR)
	printf( "Z_DATA_ERROR: Input data is corrupted\n" );
      if(res == Z_BUF_ERROR) {
	printf( "Z_BUF_ERROR: Uncompressed size is larger than expected\n" );
	return getBlob(id, size, elementsSize*2);
      }
      return NULL;
    }

    free(bData);
    _blobData[id] = uData;
    _blobComp[id] = 0;
    _blobSize[id] = actualSize;
  }

  *size = _blobSize[id];
  return _blobData[id];
}

float* ReadGemtronik::convertRayData(unsigned char* rayData, unsigned long int rayDataSize, int numRays)
{
  int raydepth = 16;
  float rayscale = RAYS_SCALE;

  if(rayDataSize < numRays * (raydepth / 8))
  {
    printf( "Uncompressed ray data size is larger than expected\n" );
    return NULL;
  }

  float *rays = new float[numRays];

  for(int a = 0; a < numRays; a++)
  {
    unsigned int intval = ((int)rayData[a*2] << 8) + (int)rayData[(a*2)+1];
    float floatval = (float)intval * rayscale;
    rays[a] = floatval;
  }
  float start = rays[0];
  float end = rays[numRays-1];
  float fix = ((end+360 - start) / (numRays-1)) / 2.0;

  for(int a = 0; a < numRays; a++)
    rays[a] += fix;

  return rays;
}

float ReadGemtronik::getScaling(float datamin, float datamax, int datadepth)
{
  long int size = (2 << datadepth-1) - 2;
  float scale = (datamax- datamin) / size;
  return scale;
}

void ReadGemtronik::memDelete()
{
  for(int bId = 0; bId < _nBlobs; bId++)
  {
    if(_blobData[bId] != NULL)
      free( _blobData[bId] );
    _blobData[bId] = NULL; 
  }
  _nBlobs = 0;
  _doc.Clear();
}

void ReadGemtronik::sweepsMemDelete(const int numele)
{
  if(_sweeps.size() != 0 && _sweeps.size() != numele)
  {
    for(int a = 0; a < _sweeps.size(); a++)
      delete _sweeps[a];
    _sweeps.clear();
  }
}

int ReadGemtronik::valueToInt(const TiXmlElement* element)
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

float ReadGemtronik::valueToFloat(const TiXmlElement* element)
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

const char* ReadGemtronik::valueToChar(const TiXmlElement* element)
{
  if(element == NULL || element->FirstChild() == NULL)
    return NULL;
  const char *value = element->FirstChild()->Value();
  if(value == NULL)
    return NULL;
  return value;
}

void ReadGemtronik::initBlobs()
{
  for(int bId = 0; bId < 30; bId++)
  {
    _blobData[bId] = NULL;
  }
}

int ReadGemtronik::readBlobs(char *fileName, char **header)
{
  *header == NULL;
  int headersize = 0;

  _nBlobs = 0;

  struct stat st;
  stat(fileName, &st);
  int size = st.st_size;

  if (size == 0)
  {
     printf ("ERROR: found file size of zero, missing volume from %s.\n", fileName);
     return (-1);
  }

  FILE* fpIn = fopen( fileName, "rb" );
  if( ! fpIn )
  {
    printf( "Cannot open input file: %s", fileName );
    return( -1 );
  }

  for(int bId = 0; bId < 30; bId++)
  {
    if(_blobData[bId] != NULL)
      free( _blobData[bId] );
    _blobData[bId] = NULL;
  }
  _nBlobs = 0;

  *header = new char[MAX_XML_SIZE];
  *header[0] = char(0);
  bool stopcopy = false;
  char line[ 1025 ];
  while( fgets( line, 1024, fpIn ) != 0 )
  {

    if(strstr( line, "<!-- END XML -->" ) != NULL)
      stopcopy = true;

    if(strstr( line, "<BLOB" ) != NULL)
    {
      char* id = strstr(line, "blobid" );
      char* size = strstr(line, "size" );
      char* com = strstr(line, "compression" );
      if(id == NULL || size == NULL || com == NULL) {
	printf( "Error processing BLOB Attributes\n" );
	fclose( fpIn );
	return(-1);
      }
      id = strchr(id, '\"')+1;
      size = strchr(size, '\"')+1;
      com = strchr(com, '\"')+1;

      int bId = atoi(id);
      unsigned long int bSize = atol(size);
      int bComp;
      if(com[0] == 'q')
	bComp = 1;
      else if(com[0] == 'n')
      	bComp = 0;
      else
	bComp = -1;

      if(bId < 0 || bId >= 30 || bSize < 0 || bComp == -1) {
	printf( "Error processing BLOB Attributes\n" );
	fclose( fpIn );
	return(-1);
      }

      unsigned char* bData;
      if ((bData = (unsigned char*)calloc(bSize, sizeof(unsigned char))) == NULL) {
	printf( "Error reading BLOB, Out of Memory\n" );
	fclose( fpIn );
	return(-1);
      }
      
      int res = fread( bData, 1, bSize, fpIn );
      if( res != bSize || bSize < 4 || 
	  ( bSize == 4 && ( bData[0] != 0 || bData[1] != 0 || bData[2] != 0 || bData[3] != 0)) )
      {
	if(bSize <= 4)
	  printf( "Cannot read BLOB data size too small, size = %ld\n", bSize);
	else {
	  int foundEOF = feof( fpIn );
	  int foundERROR = ferror( fpIn );
	  printf( "Cannot read BLOB data EOF = %d, ERROR = %d\n", foundEOF, foundERROR );
	}
	free(bData);
	return(-1);
      }

      _blobData[bId] = bData;
      _blobSize[bId] = bSize;
      _blobComp[bId] = bComp;
      if(bId > _nBlobs)
	_nBlobs = bId;
    } else 
      {
	if(!stopcopy) {
	  headersize += strlen(line);
	  if(headersize > MAX_XML_SIZE) {
	    printf( "XML Header larger than MAX_XML_SIZE\n");
	    return -1;
	  }
	  strcat(*header, line);
	}
      }
  }
  fclose( fpIn );

  return 0;
}
