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

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPt.hh>
#include "BufrStorm2Spdb.hh"
using namespace std;

const int BufrStorm2Spdb::VAR_NOT_SET = -1;
const int BufrStorm2Spdb::BUFRSTORM2SPDB_MISSING = -9999;

BufrStorm2Spdb::BufrStorm2Spdb(BUFR_Val_t &bv, DateTime &genTime, 
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
  _clearStormVars();

  _outUrl = outUrl + "/storm";

  _spdb.addUrl(_outUrl);

  _spdb.setPutMode(Spdb::putModeAddUnique);

  _spdb.clearPutChunks();
}

BufrStorm2Spdb::~BufrStorm2Spdb()
{
 
}
void BufrStorm2Spdb::_clearStormVars()
{
 
  _code = VAR_NOT_SET;

  _featureGeom = VAR_NOT_SET;

  _stormCenterLat = VAR_NOT_SET;

  _stormCenterLon = VAR_NOT_SET;

  _synopticFeature = VAR_NOT_SET;

  _stormName[0] = '\0';  
}

BufrVariableDecoder::Status_t  BufrStorm2Spdb::process()
{
  //
  // Meteorlogical attribute significance-- need STORM CENTRE == 1 
  // 0  Automatic                                                              
  // 1  STORM CENTRE                                                            
  // 2  STORM EDGE  OR OUTER LIMIT                                              
  // 3  MAXIMUM WIND 
  //
  BufrVariableDecoder::Status_t ret;

  ret = _bufrDecoder.getValue(_code, (char *)"0-08-005", _bv);
  
  int count = 0;

  while ( _code == 1 && ret == BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    if (_debug)
    { 
      cerr << "Decoding storm " <<  count << endl;
    }

    //
    // Check feature geometry 
    //    
    _bufrDecoder.getVar(_featureGeom, (char *)"0-08-007", _bv);
    
    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrStorm2Spdb(): WARNING: feature geometry fxy var not found" << endl;
      
      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _featureGeom = BUFRSTORM2SPDB_MISSING;
    }
    
    //
    // Get storm name
    //
    _bufrDecoder.getVar( _stormName, (char *)"0-01-026", _bv);
    
    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Storm name fxy var not found" << endl;
      
      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      sprintf(_stormName, "MISSING");
    }

    //
    // Get storm latitude
    //
    _bufrDecoder.getVar(_stormCenterLat, (char *)"0-05-002", _bv);

    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Storm latitude fxy var not found" << endl;
      
      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _stormCenterLat = BUFRSTORM2SPDB_MISSING;
    }
    
    //
    // Get storm longitude
    //
    _bufrDecoder.getVar(_stormCenterLon, (char *)"0-06-002", _bv);

    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Storm longitude fxy var not found" << endl;
      
      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    else if(_bv.missing_flag == 0)
    {
      _stormCenterLon = BUFRSTORM2SPDB_MISSING;
    }
 
    //
    // Get synoptic feature
    // 
    _bufrDecoder.getVar(_synopticFeature, (char *)"0-19-001", _bv);
        
    if( ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
    {
      cerr << "BufrCloud2Spdb(): WARNING: Storm longitude fxy var not found" << endl;
      
      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
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

    ret = _bufrDecoder.getVar( metFeature, (char *) "0-08-005",_bv);
    
    if (ret !=  BufrVariableDecoder:: BUFR_DECODER_SUCCESS || _bv.missing_flag != 0)
    {
      cerr << "WARNING:variable 0-08-005 (met feature) should be the "
	   << "missing/cancel value. metFeature =  " << metFeature << endl;

      cerr << " fxy found was: " << _bufrDecoder.getFxyString(_bv) << endl;
    }
    
    if(_debug)
    {
      _printStormVars();
    }

    //
    // Create an spdb GenPt if possible
    //
    _createGenPt();

    //
    // Next storm
    //
    count++;

    _clearStormVars();

    cerr << "\n\n" << endl;

    ret = _bufrDecoder.getVar( _code, (char *) "0-08-005",_bv);

  }

  if(_haveData)
  {
    _writeSpdb();
  }
  
  return ret;
}

int BufrStorm2Spdb::_createGenPt()
{
  if (!_varsSet())
  {
    if (_debug)
    {
      cerr << "BufrStorm2Spdb::_createGenPt(): WARNING! Storm vars not all set. "
	   << "No GenPt created" << endl;
    }
    return 1;
  }
  
  GenPt storm;

  storm.setName("SIGWX Storm");

  storm.setLat(_stormCenterLat);

  storm.setLon(_stormCenterLon);

  storm.setTime(_fcstTime.utime());
  
  storm.setId(0);

  storm.setNLevels(1);

  storm.addFieldInfo("synopticFeature", "none");
  
  storm.addVal(_synopticFeature);

  int errorCheck = storm.assemble();
   
  if (errorCheck)
    {
      cerr << "BufrStorm2Spdb::_createGenPt: ERROR GenPt failed to assemble: "
           <<  storm.getErrStr() << endl;

      return 1;
    }
  else if(_debug)
    {
      cerr << "BufrCloud2Spdb::_createGenPt(): Adding storm to buffer." 
	   << endl;
    }    
   
  //
  // Add storm to data to be written
  //
  _spdb.addPutChunk(storm.getId(),
		    storm.getTime(),
		    storm.getTime() + 1,
		    storm.getBufLen(),
		    storm.getBufPtr()); 
  
  _haveData = true;
   
  return 0;
}
 
int BufrStorm2Spdb::_writeSpdb()
{
  //
  // Write data
  //
  if (_spdb.put(SPDB_GENERIC_POINT_ID, SPDB_GENERIC_POINT_LABEL) != 0)
  {
    cerr << "BufrStorm2Spdb::_writeSpdb(): Spdb put failed to " << _outUrl.c_str() << endl;

    return 1;
  }
  else
  {
    if(_debug)
    {
      cerr << "BufrStorm2Spdb::_writeSpdb(): Writing to " << _outUrl.c_str() << endl;
    }
  }

  //
  // Reset spdb related members
  //
  _haveData = false;

  _spdb.clearPutChunks();

  return 0;
}
bool  BufrStorm2Spdb::_varsSet()
{
  if( _stormCenterLat != VAR_NOT_SET &&  _stormCenterLon != VAR_NOT_SET)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void BufrStorm2Spdb::_printStormVars()
{ 
  
  //
  // Meteorlogical attribute significance-- need STORM CENTRE == 1 
  // 0  Automatic                                                              
  // 1  STORM CENTRE                                                            
  // 2  STORM EDGE  OR OUTER LIMIT                                              
  // 3  MAXIMUM WIND 
  //

  cerr << "code(0 Automatic, 1 STORM CENTRE, 2 STORM EDGE OR OUTER LIMIT, "
       << "3 MAXIMUM WIND): " << _code << endl;

  cerr << "feature geometry (0 = point, 1 = Line, 2 = area, 3 = volume): " 
       <<  _featureGeom << endl;

  cerr << "storm center latitude: " << _stormCenterLat << endl;

  cerr << "storm center longitude: " << _stormCenterLon << endl;

  cerr << "synoptic feature(0 DEPRESSION , 1 TROP DEPRESS, 2 TROPIC STORM, "
       << "3 SEVERE STORM, 4 TYPHOON, 10 DUST/SANDSTORM): " 
       << _synopticFeature << endl;
}
