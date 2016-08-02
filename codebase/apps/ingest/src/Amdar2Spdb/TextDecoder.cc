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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: TextDecoder.cc,v 1.14 2016/03/07 01:22:59 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	TextDecoder
//
// Author:	G. M. cunning
//
// Date:	Mon Mar 19 11:00 2012
//
// Description: Decoder subclass that handles text messages following 
//		FM 42 -XI Ext. See WMO AMDAR reference manual.
//
//

// C++ include files
#include <cstdlib>

// System/RAP include files
#include <toolsa/DateTime.hh>

// Local include files
#include "TextDecoder.hh"

using namespace std;

// define any constants
const string TextDecoder::_className = "TextDecoder";
const string TextDecoder::SECTION_3_ID = "333";
const string TextDecoder::MISSING = "///";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

TextDecoder::TextDecoder() : 
  Decoder()
{
  _setID();

}

TextDecoder::TextDecoder(const Params *params) :
  Decoder(params)
{
  _setID();

}

TextDecoder::TextDecoder(const TextDecoder &from) :
  Decoder(from)
{
  _copy(from);
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
TextDecoder::~TextDecoder()
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// checkFormat
//
Decoder::msg_type_t 
TextDecoder::checkFormat(const string &hdr, const string &msg)
{
  if(hdr.find(_wmoId) != string::npos && msg.find(_typeId) != string::npos) {
    return MSG_TYPE_ASCII;
  }

  return MSG_TYPE_UNKNOWN;
} 

/////////////////////////////////////////////////////////////////////////
// process
//
int 
TextDecoder::process(const string& amdar_str, vector <Amdar*>& amdars)
{
  const string methodName = _className + string( "::process" );

  string eol = "\n";
  string bulletinTerm = "=";

  // read section 1
  size_t begin = amdar_str.find(eol) + 1;

  if (begin == string::npos) {
    if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
      cerr << "WARNING: " << methodName << " -- no bulletins in message." << endl;
    }
    return 1;
  }

  size_t end = 0;
  size_t messageSize = amdar_str.size();

  // read sections 2 and 3. there may be multiple 2 & 3 pairs
  while (begin <= messageSize && end != string::npos) {
    end = amdar_str.find(bulletinTerm, begin);

    // replace new line from bulletin with space
    string bulletin = string(amdar_str, begin, end-begin);

    for(size_t i = 0; i < bulletin.size(); i++) {
      if(isgraph(bulletin[i]) == false) {
	bulletin = bulletin.replace(i, 1, " ");
      }
    }
	
    if(bulletin.size() > MIN_BULLETIN_LEN) {
      
      if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
	cerr << "DEBUG: " << methodName << "bulletin: " << bulletin << endl;
      }
      
      // parse the bulletin and fill out Amdar::ascii_std_amdar_bulletin_t
      Amdar::ascii_std_amdar_bulletin_t asciiAmdar;
      _parse(bulletin, asciiAmdar);
      
      amdars.push_back(new Amdar(asciiAmdar));
    }

    // there can be one or more new lines before next bulletin
    begin = amdar_str.find_first_not_of(eol, end+1);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////
// operator=
//
TextDecoder& 
TextDecoder::operator=(const TextDecoder& from)
{
  if(this == &from) {
    return *this;
  }

  _copy(from);

  return *this;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// _parse
//
void 
TextDecoder::_parse(const string& bulletin, 
		    Amdar::ascii_std_amdar_bulletin_t& ascii_amdar)
{
  const string methodName = _className + string( "::_parse" );

  // break bulletin into tokens
  vector<string> tokens;
  size_t begin = bulletin.find_first_not_of(DELIM);
  size_t end = 0;
  bool section3Present = false;
  ascii_amdar.hasSection3 = false;
  while(end != string::npos) {
    end = bulletin.find(DELIM, begin);

    //    if((begin == string::npos) || (end ==  string::npos)) {
    //      cerr << "Error: " << methodName << endl;
    //      cerr << "   bad bulletin: " << bulletin << endl;
    //     }

    string token = string(bulletin, begin, end-begin);
    begin = bulletin.find_first_not_of(DELIM, end+1);

    // check for section 3
    if(token == SECTION_3_ID) {
      section3Present = true;
      ascii_amdar.hasSection3 = true;
      continue;
    }

    tokens.push_back(token);
  } 

 // fill out the structure
  ascii_amdar.text = bulletin;
  
  // phase of flight -- use function to normalize to expected names
  ascii_amdar.phaseFlight = _setPhaseOfFlight(tokens[0]);

  // A/C id
  ascii_amdar.acIdent = tokens[1];

  // latitude
  if(tokens[2].find('S') != string::npos) {
    ascii_amdar.latitude = -1.0*strtod(tokens[2].substr(0,4).c_str(), NULL)/100.0;
  }
  else if(tokens[2].find('N') != string::npos) {
    ascii_amdar.latitude = strtod(tokens[2].substr(0,4).c_str(), NULL)/100.0;
  }
  else {
    ascii_amdar.latitude = Amdar::MISSING_VALUE;
  }

  // longitude
  if(tokens[3].find('W') != string::npos) {
    ascii_amdar.longitude = -1.0*strtod(tokens[3].substr(0,5).c_str(), NULL)/100.0;
  }
  else if(tokens[3].find('E') != string::npos) {
    ascii_amdar.longitude = strtod(tokens[3].substr(0,5).c_str(), NULL)/100.0;
  }
  else {
    ascii_amdar.longitude = Amdar::MISSING_VALUE;
  }

  // bulletin time
  ascii_amdar.issueTime = _setTime(tokens[4]);

  // pressure altitude
  // remove F
  if(tokens[5].find(MISSING) == string::npos) {
    ascii_amdar.pressAltitude = strtod(tokens[5].substr(1,3).c_str(), NULL)*100.0;	  
  }
  else {
    ascii_amdar.pressAltitude  = Amdar::MISSING_VALUE;
  }

  // static air temperature
  if(tokens[6].find("MS") != string::npos) {
    if(tokens[6].find(MISSING) == string::npos) {
      ascii_amdar.temperature = -1.0*strtod(tokens[6].substr(2,3).c_str(), NULL)/10.0;
    }
    else {
      ascii_amdar.temperature = Amdar::MISSING_VALUE;
    }
  }
  else if(tokens[6].find("PS") != string::npos) {
    if(tokens[6].find(MISSING) == string::npos) {
      ascii_amdar.temperature = strtod(tokens[6].substr(2,3).c_str(), NULL)/10.0;
    }
    else {
      ascii_amdar.temperature  = Amdar::MISSING_VALUE;
    }
  }
  else {
    ascii_amdar.temperature = Amdar::MISSING_VALUE;
  }

  // check for dew point or humidity
  size_t idx = 7;

  bool badWinds = false;
  // a missing token might appear in place of RH/dewpoint or winds are missing
  if(tokens[idx].find(MISSING) != string::npos) {
    ascii_amdar.dewPoint = Amdar::MISSING_VALUE;
    ascii_amdar.relativeHumidity = Amdar::MISSING_VALUE;
    if(tokens[idx].size() > 3) {
       badWinds = true;
    }
    idx++;
  }
  else if(tokens[idx].find('/') != string::npos) {
    ascii_amdar.dewPoint = Amdar::MISSING_VALUE;
    ascii_amdar.relativeHumidity = Amdar::MISSING_VALUE;
  }
  else {
    // check if the token is dewpoint or humidity
    if(tokens[idx].find('S') != string::npos) {
      // static air temperature
      if(tokens[idx].find("MS") != string::npos) {
	if(tokens[idx].find(MISSING) == string::npos) {
	  ascii_amdar.dewPoint = -1.0*strtod(tokens[7].substr(2,3).c_str(), NULL)/10.0;
	}
	else {
	  ascii_amdar.dewPoint = Amdar::MISSING_VALUE;
	}
      }
      else if(tokens[idx].find("PS") != string::npos) {
	if(tokens[idx].find(MISSING) == string::npos) {
	  ascii_amdar.dewPoint = strtod(tokens[7].substr(2,3).c_str(), NULL)/10.0;
	}
	else {
	  ascii_amdar.dewPoint = Amdar::MISSING_VALUE;
	}
      }
      else {
	ascii_amdar.dewPoint = Amdar::MISSING_VALUE;
      }
      ascii_amdar.relativeHumidity = Amdar::MISSING_VALUE;
    }
    else {
	if(tokens[idx].find(MISSING) == string::npos) {
	  ascii_amdar.relativeHumidity = strtod(tokens[7].c_str(), NULL);
	}
      ascii_amdar.dewPoint = Amdar::MISSING_VALUE;
    }
    idx ++;
  }
  
  // winds
  if(badWinds == false) {
    if(tokens[idx].substr(0,3).find(MISSING) == string::npos) {
      ascii_amdar.windDirection = strtod(tokens[idx].substr(0,3).c_str(), NULL);
    }
    else {
      ascii_amdar.windDirection = Amdar::MISSING_VALUE;
    }
    if(tokens[idx].substr(4,3).find(MISSING) == string::npos) {
      ascii_amdar.windSpeed = strtod(tokens[idx].substr(4,3).c_str(), NULL);
    }
    else {
      ascii_amdar.windSpeed = Amdar::MISSING_VALUE;
    }
    idx++;
  }
  // turbulence
  if(tokens[idx].find(MISSING) == string::npos) {
    ascii_amdar.turbulenceCode = strtol(tokens[idx].substr(2,1).c_str(), NULL, 10);
  } 
  else {
    ascii_amdar.turbulenceCode = static_cast<int>(Amdar::MISSING_VALUE);
  }
  idx++;
    
  // navigation system
  if(tokens[idx].find(MISSING) == string::npos) {
    ascii_amdar.navSys = strtol(tokens[idx].substr(1,1).c_str(), NULL, 10);
    ascii_amdar.typeNavSys = strtol(tokens[idx].substr(2,1).c_str(), NULL, 10);
    ascii_amdar.tempPrec = strtol(tokens[idx].substr(3,1).c_str(), NULL, 10);
  } 
  else {
    ascii_amdar.navSys = Amdar::MISSING_VALUE;
  } 
  idx++;

  // take care of section 3, if present
  if(section3Present == true) {
    if(tokens[idx].find(MISSING) == string::npos) {
      ascii_amdar.sec3presAlt = strtod(tokens[idx].substr(1,3).c_str(), NULL)*100.0;
    }
    else {
      ascii_amdar.sec3presAlt = Amdar::MISSING_VALUE;
    }
    idx++;
    if(tokens[idx].find(MISSING) == string::npos) {
      ascii_amdar.verticalGust = strtod(tokens[idx].substr(2,3).c_str(), NULL)/10.0;
    }
    else {
      ascii_amdar.verticalGust = Amdar::MISSING_VALUE;
    }
  }

}

/////////////////////////////////////////////////////////////////////////
// _setPhaseOfFlight
//
string 
TextDecoder::_setPhaseOfFlight(const string& text)
{
  return text;
}

/////////////////////////////////////////////////////////////////////////
// _setTime
//
time_t 
TextDecoder::_setTime(const string& btime)
{
  // compare the hhmmss time to _fileTime and create difference

  // turn btime into number of seconds since start of day
  int day = strtol(btime.substr(0,2).c_str(), NULL, 10);
  int hour = strtol(btime.substr(2,2).c_str(), NULL, 10);
  int minute = strtol(btime.substr(4,2).c_str(), NULL, 10);
  int bTimeSecs = 24*3600*day + 3600*hour + 60*minute;

  // turn time part (not date) into number of seconds since start of day
  DateTime ftime(_fileTime);
  day = ftime.getDay();
  hour = ftime.getHour();
  minute = ftime.getMin();
  int fTimeSecs = 24*3600*day + 3600*hour + 60*minute;
  
  int deltaSec = fTimeSecs - bTimeSecs;

  return _fileTime - deltaSec;
    
}

/////////////////////////////////////////////////////////////////////////
// _setID
//
void 
TextDecoder::_setID()
{
  _wmoId = "UD";
  _typeId = "AMDAR";
}


/////////////////////////////////////////////////////////////////////////
// _copy
//
void 
TextDecoder::_copy(const TextDecoder& from)
{

}
