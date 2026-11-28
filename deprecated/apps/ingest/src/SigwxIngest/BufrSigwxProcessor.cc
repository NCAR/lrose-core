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
#include "BufrSigwxProcessor.hh"
#include "BufrCloud2Spdb.hh"
#include "BufrStorm2Spdb.hh"
using namespace std;

// Constructor

BufrSigwxProcessor::BufrSigwxProcessor(char *buffer, string outUrl, bool debug):
  _sigwxBuf(buffer),
  _outUrl(outUrl),
  _debug(debug)
  
{
  cerr << "found a message " << endl;
}

// destructor

BufrSigwxProcessor::~BufrSigwxProcessor()

{
 
}

int BufrSigwxProcessor::process()
{
  BUFR_Info_t bInfo;

  BUFR_Val_t bv;

  PMU_auto_register("BufrSigwxProcessor::decodeBufrMsg(): Decoding SIGWX");

  //
  // Initialize mel_bufr structures
  //
  if ( BUFR_Info_Init(&bInfo))
  {
    cerr << "BufrSigwxProcessor::process(): BUFR_Info_Init failure.\n";
    
    return 1;
  }
 
  //
  // Initialize file decoding 
  //
  if( BUFR_Init( &bInfo,_sigwxBuf, DECODING ) )
  {
    cerr << "BufrSigwxProcessor::process() decoding init failure.\n";
    
    BUFR_perror( (char*)"BufrSigwxProcessor::process():" );
    
    return 1;
  }

  BufrVariableDecoder::Status_t ret;

  while(true)
  {
    
    BufrVariableDecoder::Status_t ret;

    //
    // Search for start of message
    //
    ret = _findHeaderStart(bv);
    
    if( ret == BufrVariableDecoder::BUFR_DECODER_EOM  || 
	ret == BufrVariableDecoder::BUFR_DECODER_EOF)
    {
      cerr << "No message header found. Empty message." << endl;
     
      break;
    }
    
    //
    // All SIGWX should start with originating center, generation time,
    // valid time, flight level boundaries for data. We record only 
    // the relevant data.
    //
    DateTime genTime, fcstTime;
    
    int altLowerBound, altUpperBound;

    //
    // If header is not successfully parsed than move on to the next one
    // 
    if ( _parseMsgHeader(bv, genTime, fcstTime, altLowerBound, altUpperBound))
      continue;

    //
    // Determine if we have Cloud, Storm, or other report
    // only Cloud and Storm are decoded.
    //
    char fxyStr[8];

    ret = _bufrDecoder.getFxyString(fxyStr, bv);
    
    if( ret == BufrVariableDecoder::BUFR_DECODER_EOM  || 
	ret == BufrVariableDecoder::BUFR_DECODER_EOF)
    {
      cerr << "Empty message." << endl;
     
      break;
    }
    else if ( strcmp(fxyStr,"0-08-011") == 0)
    {
      int metCode;
      
      _bufrDecoder.getValue(metCode, (char*)"0-08-011",bv);

      //
      // Decode Meteorlogical Feature
      //
      switch (metCode)
      {
      case 12:
	{
	  cerr << "Processing Cloud (Met feature type " << metCode << ")" << endl;
	  BufrCloud2Spdb bufrCloud(bv, genTime, fcstTime, altLowerBound, 
				   altUpperBound, _outUrl, _debug);
	  ret = bufrCloud.process();
	  cerr << "cloud processing ret " << ret  << endl;
	  break;
	}
      case 14:
	{
	  cerr << "Processing Storm (Met feature type " << metCode << ")" << endl;
	  BufrStorm2Spdb bufrStorm(bv, genTime, fcstTime, altLowerBound, 
				   altUpperBound, _outUrl,_debug);
	  cerr << "storm processing ret " << ret  << endl;
	  break;
	}
      default:
	{
	  cerr << "Meteorological feature " << metCode << " not being decoded"<< endl;
	  break;
	}
      }
    }
    else if  ( strcmp(fxyStr,"0-08-005") == 0)
    {
      int metAttribute;
      
      _bufrDecoder.getValue(metAttribute, (char *)fxyStr, bv);

       if (metAttribute == 1)
       {
	 //
	 // Decode storm center
	 //
	 cerr << "Processing Storm Met Attribute (type " << metAttribute << ")" << endl;
	 BufrStorm2Spdb bufrStorm(bv, genTime, fcstTime, altLowerBound, 
				  altUpperBound,_outUrl,_debug);
	 ret = bufrStorm.process();
	 cerr << "Storm processing ret " << ret  << endl;
       }
       else
       {
	 cerr << "Met Attribute type " << metAttribute 
	      << " Not == storm so not processed" << endl;
       }
    }
    else
    {
      cerr << "Unexpected Variable string " <<  fxyStr  
	   << " Not decoding rest of message " << endl;

      continue;
    }

    if ( ret == BufrVariableDecoder::BUFR_DECODER_EOF)
    {
      cerr << "BUFR EOF reached. Exiting." << endl;
      break;
    }
  } // end while

  BUFR_Destroy(1);
}

int BufrSigwxProcessor::_parseMsgHeader(BUFR_Val_t &bv, DateTime &genTime, 
					DateTime &fcstTime,int &altLowerBound, 
					int &altUpperBound)
{

  //
  // Identification of originating/generating centre
  //
  int centerCode;

  BufrVariableDecoder::Status_t ret = _bufrDecoder.getValue(centerCode, (char *)"0-01-031",   bv);
  
  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): ERROR: Did not find orginating center" << endl;
    return 1; 
  }

  
  //
  // Time significance (should be 'analysed' == 12)
  //
  ret = _bufrDecoder.checkVar((char *)"0-08-021",   bv);

  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): ERROR: Did not find time significance var (analysed)" << endl;

    return 1; 
  }


  //
  // Get generation time
  //
  int iret = _getDateTime(bv, genTime);

  if (iret)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): Failure to get generation time " << endl;
    
    return 1;
  }

  //
  // Time significance (should be 'forecast' == 4)
  //
  ret = _bufrDecoder.checkVar((char *)"0-08-021",   bv);
  
  
  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): ERROR: Did not find time significance var (analysed)" << endl;
   
    return 1; 
  }

  //
  // Get get forecast time
  //
  iret = _getDateTime(bv, fcstTime);
   
  if (iret)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): Failure to get forecast time " << endl;
   
    return 1;
  }


  //
  // Get flight level boundaries (meters)
  //
  ret = _bufrDecoder.getVar(altLowerBound, (char *)"0-07-002",   bv);
  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): Failure to alt lower bound " << endl;

    return 1; 
  }


  ret = _bufrDecoder.getVar(altUpperBound, (char *)"0-07-002",   bv);
 
  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    cerr << "BufrSigwxProcessor::_parseMsgHeader(): Failure to alt upper bound " << endl;

    return 1; 
  }
  
  if(_debug)
  {
    cerr << "genTime: " << genTime.dtime() << endl;

    cerr << "fcstTime: " << fcstTime.dtime() << endl;
    
    cerr << "flight level m (lower bound): " << altLowerBound << endl;
    
    cerr << "flight level m (upper bound): " << altUpperBound << endl;
  }

  return 0;
}

int BufrSigwxProcessor::_getDateTime( BUFR_Val_t &bv, DateTime &dateTime)
{
  BufrVariableDecoder::Status_t ret;

  int year;
 
  ret =_bufrDecoder.getVar(year, (char *)"0-04-001",   bv);

  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    return 1; 
  } 

  int month;

  ret = _bufrDecoder.getVar(month, (char *)"0-04-002",   bv);
  
  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    return 1; 
  }


  int day;

  ret = _bufrDecoder.getVar(day, (char *)"0-04-003",   bv);

  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    return 1; 
  }


  int hour;

  ret = _bufrDecoder.getVar(hour, (char *)"0-04-004",   bv);

  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    return 1; 
  }

  int minute;

  ret = _bufrDecoder.getVar(minute, (char *)"0-04-005",   bv);
  
  if(ret !=  BufrVariableDecoder::BUFR_DECODER_SUCCESS)
  {
    return 1; 
  }

  dateTime.set( year, month, day, hour, minute, 0 );

  if(_debug)
  {
    cerr << "Date/Time: " << dateTime.dtime() << endl;
  }

  return 0;
}

BufrVariableDecoder::Status_t BufrSigwxProcessor::_findHeaderStart(BUFR_Val_t &bv)
{
  BufrVariableDecoder::Status_t ret;

  char fxyStr[30];

  bool gotStart;

  do 
  {
    ret = _bufrDecoder.getFxyString(fxyStr, bv);
  }
  while( ret == BufrVariableDecoder::BUFR_DECODER_SUCCESS &&
	 ret != BufrVariableDecoder::BUFR_DECODER_EOM &&
	 ret != BufrVariableDecoder::BUFR_DECODER_EOF &&
	 strcmp(fxyStr,"0-01-031") != 0  );

  return ret;
}
