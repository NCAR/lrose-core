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
//////////////////////////////////////////////////////////////////////////////
//
//  ReadNexrad handles reading the actual Gemtronik file.
//
//  Once parsed and/or converted to a format we need the data
//  is then passed off to the 
//
//  $Id: ReadGemtronik.hh,v 1.4 2016/03/07 01:23:00 dixon Exp $
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _READ_GEMTRONIK_HH
#define _READ_GEMTRONIK_HH

#include <string>
#include <cstdio>

#include <tinyxml/tinystr.h>
#include <tinyxml/tinyxml.h>

#include "Params.hh"
#include "Gemtronik2Netcdf.hh"
#include "NcOutput.hh"

//
// Forward class declarations
class SweepData;

class ReadGemtronik
{
public:
   ReadGemtronik(Params *P);
  ~ReadGemtronik();

  typedef enum {
    REFLECTIVITY,
    SPECTRUMWIDTH,
    VELOCITY,
    VOLUMEUNKNOWN
  } VolumeEnum_t;


  int readFile(char *fileName, time_t volumeTime, bool uncompress, 
	       VolumeEnum_t fileType);

  void finishVolume();

  void clearSweeps();

private:

  int readBlobs(char *fileName, char **header);
  void initBlobs();

  unsigned char* getBlob(int id, unsigned long int* size, long int elementsSize);

  float* convertRayData(unsigned char* rayData, unsigned long int rayDataSize, int numRays);

  float getScaling(float datamin, float datamax, int datadepth);
  void memDelete();
  void sweepsMemDelete(const int numele);

  int valueToInt(const TiXmlElement* element);
  float valueToFloat(const TiXmlElement* element);
  const char* valueToChar(const TiXmlElement* element);

  Params   *params;
  
  NcOutput  *ncOutput;
  TiXmlDocument   _doc;

  float _lat;
  float _lon;
  float _alt;
  float _waveLength;
  float _beamWidth;

  vector< SweepData* > _sweeps;
  vector< SweepData* > _prevSweeps;

  int _nBlobs;
  unsigned char* _blobData[30];
  unsigned long int _blobSize[30];
  int _blobComp[30];

};

#endif

