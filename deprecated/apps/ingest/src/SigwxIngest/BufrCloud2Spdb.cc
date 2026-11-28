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

#include <map>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>
#include <rapformats/GenPoly.hh>
#include <Spdb/Spdb_typedefs.hh>
#include "BufrCloud2Spdb.hh"
#include "BufrVariableDecoder.hh"
using namespace std;


const int BufrCloud2Spdb::VAR_NOT_SET = -1;
const int BufrCloud2Spdb::BUFRCLOUD2SPDB_MISSING = -9999;

BufrCloud2Spdb::BufrCloud2Spdb(BUFR_Val_t &bv, DateTime &genTime, 
			       DateTime &fcstTime,int &altLowerBound, 
			       int &altUpperBound, string outUrl, bool debug):
  _bv(bv),
  _genTime(genTime),
  _fcstTime(fcstTime),
  _altLowerBound(altLowerBound),
  _altUpperBound(altUpperBound),
  _haveData(false),
  _debug(debug)
{
  _clearCloudVars();

  _outUrl = outUrl + "/cloud";

  _spdb.addUrl(_outUrl);

  _spdb.setPutMode(Spdb::putModeAddUnique);

  _spdb.clearPutChunks();
}

BufrCloud2Spdb::~BufrCloud2Spdb()
{
 
}

void BufrCloud2Spdb::_clearCloudVars()
{
  _cloudBoundary.clear();
   
  _metCode = VAR_NOT_SET;

  _featureGeom = VAR_NOT_SET;

  _cloudBase = VAR_NOT_SET;

  _cloudTop = VAR_NOT_SET;

  _distCode = VAR_NOT_SET;

  _cloudType = VAR_NOT_SET;  
}

BufrVariableDecoder::Status_t BufrCloud2Spdb::process()
{

  BufrVariableDecoder::Status_t ret;

  ret = _bufrDecoder.getValue(_metCode, (char *)"0-08-011", _bv);
  
  int count = 0;

  while ( _metCode == 12 && ret == BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    if (_debug)
    {
      cerr << "Decoding cloud " <<  count << endl;
    }

    //
    // Check feature geometry 
    // 0 = point, 1 = Line, 2 = area, 3= volume
    // 
    ret = _bufrDecoder.getVar(_featureGeom, (char *)"0-08-007", _bv);
    
    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: feature geometry fxy var not found" << endl;
      
      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _featureGeom = BUFRCLOUD2SPDB_MISSING;
    }

    //
    // Get cloud base
    //
    _bufrDecoder.getVar(_cloudBase, (char *)"0-07-002", _bv);
      
    if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Cloud base fxy not found" << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _cloudBase = BUFRCLOUD2SPDB_MISSING;
    }

    //
    // Get cloud top
    //
    _bufrDecoder.getVar(_cloudTop, (char *)"0-07-002", _bv);
     
    if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Cloud top fxy not found" << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _cloudTop = BUFRCLOUD2SPDB_MISSING;
    }

    //
    // Get all lat lon pairs defining the cloud
    //
    char *fxyStr;
    
    float lat, lon;
    
    _bufrDecoder.getVar(lat, (char *)"0-05-002", _bv);
    
    fxyStr = _bufrDecoder.getFxyString(_bv);
    
    while ( strcmp(fxyStr, "0-05-002" ) == 0 )
    {
      _bufrDecoder.getVar(lon, (char *)"0-06-002", _bv);
    
      pair <float, float> coordinates;

      coordinates.first = lat;

      coordinates.second = lon;
  
      _cloudBoundary.push_back(coordinates);
      
      _bufrDecoder.getVar(lat, (char *)"0-05-002", _bv);
      
      fxyStr = _bufrDecoder.getFxyString(_bv);
    } 
    
    //
    // Cloud distribution  
    //   
    _bufrDecoder.getValue(_distCode, (char *)"0-20-008", _bv);
    
    if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Cloud distribution code fxy not found" << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _distCode = BUFRCLOUD2SPDB_MISSING;
    }
    
    //
    // Get cloud type
    //
    int cloudType;
    
    ret = _bufrDecoder.getVar(_cloudType, (char *)"0-20-012", _bv);
    
    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb():WARNING: Cloud type code fxy not found" << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    if(_bv.missing_flag == 0)
    {
      _cloudType = BUFRCLOUD2SPDB_MISSING;
    }
    
    //
    // These are missing vals/cancel values)
    //
    int dim;

    ret = _bufrDecoder.getVar( dim, (char *) "0-08-007",_bv);

    if (ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS || _bv.missing_flag != 0)
    {
      cerr << "WARNING:variable 0-08-007 (dim) should be missing " << dim << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }

    int metFeature;

    ret = _bufrDecoder.getVar( metFeature, (char *) "0-08-011",_bv);
    
    if (ret !=  BufrVariableDecoder:: BUFR_DECODER_SUCCESS || _bv.missing_flag != 0)
    {
      cerr << "WARNING:variable 0-08-011 (met feature) should be the "
	   << "missing/cancel value. metFeature =  " << metFeature << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
   
    if(_debug)
    {
      _printCloudVars();
    }

    //
    // Create an spdb polygon if possible
    //
    _createGenPoly();

    //
    // Next cloud
    //
    count++;

    _clearCloudVars();

    cerr << "\n\n" << endl;

    //
    // Get met code
    //
    ret = _bufrDecoder.getVar( _metCode, (char *) "0-08-011",_bv);  
  }

  if(_haveData)
  {
    _writeSpdb();
  }

  return ret;
}

int BufrCloud2Spdb::_createGenPoly()
{
  if (!_varsSet())
  {
    if (_debug)
    {
      cerr << "BufrCloud2Spdb::_createGenPoly(): WARNING! Cloud vars not all set. "
	   << "No GenPoly created" << endl;
    }
    return 1;
  }
  
  GenPoly cloud;
  
  cloud.setName("SIGWX Cloud");

  cloud.setNLevels(1);

  cloud.setTime(_fcstTime.utime());
  
  int lastIndex = _cloudBoundary.size() -1; 

  if( _cloudBoundary[0].first ==  _cloudBoundary[lastIndex].first &&
      _cloudBoundary[0].second ==  _cloudBoundary[lastIndex].second)
  {
    cloud.setClosedFlag(true);

    lastIndex = lastIndex-1;
  }

  for (int i = 0; i < lastIndex; i++)
  { 
     GenPoly::vertex_t cloudPoint;

     cloudPoint.lat = _cloudBoundary[i].first;

     cloudPoint.lon = _cloudBoundary[i].second;

     cloud.addVertex(cloudPoint);
  }

  cloud.addFieldInfo("CloudBase", "meters");

  cloud.addFieldInfo("CloudTop", "meters");

  cloud.addFieldInfo("CloudDistribution", "none");

  cloud.addFieldInfo("CloudType", "none");

  cloud.addVal(_cloudBase);

  cloud.addVal(_cloudTop);

  cloud.addVal(_distCode);

  cloud.addVal(_cloudType);

  bool success = cloud.assemble();

  if ( !success)
  {
    cerr << "BufrCloud2Spdb::_createGenPoly(): ERROR: GenPoly failed to "
	 << "assemble: " << cloud.getErrStr() << endl;
    return 1;
  }
  else
  {
    if(_debug)
    {
      cerr << "BufrCloud2Spdb::_createGenPoly(): Adding cloud to buffer " 
	   << endl;
    }    
  }

  //
  // Add cloud to data buffer 
  //
  _spdb.addPutChunk(cloud.getId(),
		    cloud.getTime(),
		    cloud.getExpireTime(),
		    cloud.getBufLen(),
		    cloud.getBufPtr());

  _haveData = true;
   
  return 0;
}

int BufrCloud2Spdb::_writeSpdb()
{
  //
  // Write data
  //
  if (_spdb.put(SPDB_GENERIC_POLYLINE_ID, SPDB_GENERIC_POLYLINE_LABEL) != 0)
  {
    cerr << "BufrCloud2Spdb::_writeSpdb: Spdb put failed to " 
	 << _outUrl.c_str() << endl;

    return 1;
  }
  else
  {
    if(_debug)
    {
      cerr << "BufrCloud2Spdb::_writeSpdb(): Writing to " << _outUrl.c_str() 
	   << endl;
    }
  }

  //
  // Reset spdb related members
  //
  _haveData = false;

  _spdb.clearPutChunks();

  return 0;
}

bool  BufrCloud2Spdb::_varsSet()
{
  if ( _cloudBoundary.size() >0 )
  {   
    return true;
  }
  else
  {
    if(_debug)
    {
      cerr << "BufrCloud2Spdb::_varsSet(): Not all cloud variables set. " << endl;
    }
    return false;
  }
       
}

void BufrCloud2Spdb::_printCloudVars()
{ 
  cerr << "met code: " << endl;

  cerr << "feature geometry (0 = point, 1 = Line, 2 = area, 3 = volume): " 
       <<  _featureGeom << endl;

  cerr << "cloud base" << _cloudBase << endl;
  
  cerr << "cloud top: " << _cloudTop << endl;
  
  for (int i = 0; i < _cloudBoundary.size(); i++)
  {
    cerr << "lat : " << _cloudBoundary[i].first
	 << " lon : " << _cloudBoundary[i].second << endl;
  }
  
  cerr << "distribution Code " << _distCode << endl;
  
  cerr << "cloud type " << _cloudType << endl;
}
