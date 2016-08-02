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
// Utils.cc
//
// Taf2Spdb object -- utilities functions
//
// Deirdre Garvey, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Spring 2003
//
///////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/globals.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/DateTime.hh>
#include "Taf2Spdb.hh"

/////////////////////////////////////////////////
// Look for imbedded delimiter
// Return the number found.

int Taf2Spdb::_isImbedDelimiter(size_t &startIdx,
                                const vector <string> &toks,
                                const string &delimiter)

{
  // set defaults

  int nDelim=0;
  if (startIdx < 0) {
    return false;
  }

  // look for imbedded delimiter

  for (size_t ii = startIdx; ii < toks.size(); ii++) {

    if ((toks[ii] != delimiter) && (toks[ii].find(delimiter, 0) != string::npos)) {
      int nfound=_findStringNTimes(toks[ii], delimiter);
      nDelim=nDelim+nfound;
    }
  }

  return nDelim;
   
}

////////////////////////////////////////////////
// If spacer1 occurs multiple times in the input vector, replace it with 
// spacer2. Then retokenize on spacer3. Use this with care!
//
// Return true if do the replace, false otherwise

bool Taf2Spdb::_replaceSpacerRetokenize(const string &spacer1,
                                        const string &spacer2,
                                        const string &spacer3,
                                        size_t &startTok,
                                        vector<string> &intoks,
                                        vector<string> &toks)
{
  // set defaults

  bool foundSpacer1=false;

  // look for spacer

  int nspacer1=0;
  for (size_t ii = startTok; ii < intoks.size(); ii++) {

    string useTok=intoks[ii];

    // Look for the input spacer1 in the string

    size_t pos = useTok.find(spacer1);
    if (pos != string::npos) {
      nspacer1++;
      foundSpacer1=true;
      break;
    }
  }

  // did not find the spacer. exit.

  if (!foundSpacer1) {
    return false;
  }

  // are spacer1 and spacer2 the same length?

  size_t len_spacer1=spacer1.length();
  size_t len_spacer2=spacer2.length();
   
  // Do the replace

  string str="";
  for (size_t ii = startTok; ii < intoks.size(); ii++) {

    string useTok=intoks[ii];
     
    // Replace all occurences of spacer1 with spacer2

    bool foundSpacer1=false;
    bool done=false;
    size_t start=0;
    size_t pos=0;
    string tmpUseTok="";
    string remaining="";

    while (!done) {
      start= useTok.find(spacer1, pos);
      if (start == string::npos) {
        done = true;
      } else {
        foundSpacer1=true;
        if (len_spacer1 == len_spacer2) {
          useTok.replace(start, len_spacer1, spacer2);
          pos=pos+len_spacer1;
        } else {
          size_t len=start-pos;
          tmpUseTok += useTok.substr(pos, len);
          tmpUseTok += spacer2;
          pos=start+len_spacer1;
          remaining = useTok.substr(pos, string::npos);

        }
      }
    }

    // Rebuild the string so can retokenize

    if (!foundSpacer1) {
      str += useTok;
    } else {
      if (len_spacer1 == len_spacer2) {
        str += useTok;
      } else {
        str += tmpUseTok;
        str += remaining;
      }
    }
    str += " ";
  } // ii

  // tokenize based on spacer3

  _tokenize(str, spacer3, toks);

  return true;
}
  
  
////////////////////////////////////////////////////////////////
// compute issue time from the file time and a time token string

int Taf2Spdb::_computeIssueTime(const string &timeTok,
                                time_t &issueTime)
  
{

  if (timeTok.size() != 6) {
    return -1;
  }

  if (!_allDigits(timeTok)) {
    return -1;
  }

  int day, hour, min;
  if (sscanf(timeTok.c_str(), "%2d%2d%2d", &day, &hour, &min) != 3) {
    return -1;
  }

  if (day < 1 || day > 31) {
    return -1;
  }
  if (hour < 0 || hour > 24) {
    return -1;
  }
  if (min < 0 || min > 59) {
    return -1;
  }

  DateTime rtime(_fileTime);
  rtime.setDay(day);
  rtime.setHour(hour);
  rtime.setMin(min);
  rtime.setSec(0);
  issueTime = rtime.utime();
  _correctTimeForMonthChange(issueTime);

  return 0;

}
  
////////////////////////////////////////////////
// find the start and end times
//

int Taf2Spdb::_findStartEndTimes(time_t &startTime,
                                 time_t &endTime)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setStartEndTimes()" << endl;
  }
  
  // set the start and end times - take the first set which decodes
  // set the defaults to the file time
  
  // look for "VALID .... "
  
  for (int ii = _startTokNum + 1; ii < (int) _toks.size() - 1; ii++) {

    if (_toks[ii] == "VALID") {

      // VALID start_time/end_time
      
      if (_computeStartEndTimes(_toks[ii+1], startTime, endTime) == 0) {
        _used[ii] = true;
        _used[ii+1] = true;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Valid times: found start and end times: "
               << _toks[ii] << " "
               << _toks[ii+1] << endl;
        }
        return 0;
      }

      // VALID UNTIL end_time?
      
      if (ii < (int) _toks.size() - 2) {
        if (_toks[ii+1] == "UNTIL" &&
            _isTimeTok(_toks[ii+2], endTime)) {
          // use file time for start time
          startTime = _fileTime;
          if (endTime < startTime) {
            endTime += 86400;
          }
          _used[ii] = true;
          _used[ii+1] = true;
          _used[ii+2] = true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "Valid times: found end time: "
                 << _toks[ii] << " "
                 << _toks[ii+1] << " " 
                 << _toks[ii+2] << endl;
          }
          return 0;
        }
      }

      // VALID start_time end time
        
      if (ii < (int) _toks.size() - 2) {
        if (_isTimeTok(_toks[ii+1], startTime) &&
            _isTimeTok(_toks[ii+2], endTime)) {
          _used[ii] = true;
          _used[ii+1] = true;
          _used[ii+2] = true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "Valid times: found start and end times: "
                 << _toks[ii] << " "
                 << _toks[ii+1] << " " 
                 << _toks[ii+2] << endl;
          }
          return 0;
        }
      }

      // VALID start_time - end time
        
      if (ii < (int) _toks.size() - 3) {
        if (_isTimeTok(_toks[ii+1], startTime) &&
            _isTimeTok(_toks[ii+3], endTime)) {
          _used[ii] = true;
          _used[ii+1] = true;
          _used[ii+2] = true;
          _used[ii+3] = true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "Valid times: found start and end times: "
                 << _toks[ii] << " "
                 << _toks[ii+1] << " " 
                 << _toks[ii+2] << " " 
                 << _toks[ii+3] << endl;
          }
          return 0;
        }
      }

    } // if (_toks[ii] == "VALID")

  } // ii

  return -1;

}

//////////////////////////////////////////////////////////
// compute start and end times from the file time and a
// time token string

int Taf2Spdb::_computeStartEndTimes(const string &timeTok,
                                    time_t &startTime,
                                    time_t &endTime)
  
{

  if ((timeTok.size() < 13) || (timeTok.size() > 15)) {
    return -1;
  }

  if (_allAlpha(timeTok)) {
    return -1;
  }

  int sday, shour, smin;
  int eday, ehour, emin;
  bool found=false;

  if (sscanf(timeTok.c_str(), "%2d%2d%2d/%2d%2d%2d",
	     &sday, &shour, &smin, &eday, &ehour, &emin) == 6) {
    found=true;
  }

  if (!found) {
    if (sscanf(timeTok.c_str(), "%2d%2d%2d-%2d%2d%2d",
	       &sday, &shour, &smin, &eday, &ehour, &emin) == 6) {
      found=true;
    }
  }

  if (!found) {
    char zStr[2];
    if (sscanf(timeTok.c_str(), "%2d%2d%2d%1s/%2d%2d%2d",
	       &sday, &shour, &smin, zStr, 
	       &eday, &ehour, &emin) == 7) {
      found=true;
    }
  }

  if (!found) {
    char zStr[2];
    if (sscanf(timeTok.c_str(), "%2d%2d%2d/%2d%2d%2d%1s",
	       &sday, &shour, &smin, 
	       &eday, &ehour, &emin, zStr) == 7) {
      found=true;
    }
  }

  if (!found) {
    char zStr1[2], zStr2[2];
    if (sscanf(timeTok.c_str(), "%2d%2d%2d%1s/%2d%2d%2d%1s",
	       &sday, &shour, &smin, zStr1, 
	       &eday, &ehour, &emin, zStr2) == 8) {
      found=true;
    }
  }

  if (!found) {
    if (sscanf(timeTok.c_str(), "%2d%2d%2d/%2d%2d",
	       &sday, &shour, &smin, &ehour, &emin) == 5) {
      eday=sday;
      found=true;
    }
  }
  if (!found) {
    return -1;
  }

  if (sday < 1 || sday > 31) {
    return -1;
  }
  if (shour < 0 || shour > 24) {
    return -1;
  }
  if (smin < 0 || smin > 59) {
    return -1;
  }

  if (eday < 1 || eday > 31) {
    return -1;
  }
  if (ehour < 0 || ehour > 24) {
    return -1;
  }
  if (emin < 0 || emin > 59) {
    return -1;
  }

  DateTime stime(_fileTime);
  stime.setDay(sday);
  stime.setHour(shour);
  stime.setMin(smin);
  stime.setSec(0);
  startTime = stime.utime();
  _correctTimeForMonthChange(startTime);

  DateTime etime(_fileTime);
  etime.setDay(eday);
  etime.setHour(ehour);
  etime.setMin(emin);
  etime.setSec(0);
  endTime = etime.utime();
  _correctTimeForMonthChange(endTime);

  return 0;

}
  
//////////////////////////////////////////////////////////
// adjust time to correct for error which occur when the
// month and/or year is different from the file time

void Taf2Spdb::_correctTimeForMonthChange(time_t &reportTime)
  
{

  // if within half month we are OK
  
  double diff = (double) _fileTime - (double) reportTime;

  if (fabs(diff) > _halfMonthSecs) {

    // adjust year as required
    
    if (_fileTime - reportTime > _halfYearSecs) {
      // report time year is behind
      DateTime rtime(reportTime);
      rtime.setYear(rtime.getYear() + 1);
      reportTime = rtime.utime();
    } else if (reportTime - _fileTime > _halfYearSecs) {
      // report time year is ahead
      DateTime rtime(reportTime);
      rtime.setYear(rtime.getYear() - 1);
      reportTime = rtime.utime();
    }
    
    // adjust month as required
    
    if (_fileTime - reportTime > _halfMonthSecs) {
      // report time month is behind
      DateTime rtime(reportTime);
      rtime.setMonth(rtime.getMonth() + 1);
      reportTime = rtime.utime();
    } else if (reportTime - _fileTime > _halfMonthSecs) {
      // report time month is ahead
      DateTime rtime(reportTime);
      rtime.setMonth(rtime.getMonth() - 1);
      reportTime = rtime.utime();
    }

  }

}

///////////////////////////////////////////////////////////////
// correct time to within reasonable range compared to file time

void Taf2Spdb::_correctTimeForFileTime(time_t &computedTime)
  
{

  double diff = (double) computedTime - (double) _fileTime;

  if (fabs(diff) > 86400) {
    // report time should not differ from file time by more
    // than 1 day
    if (_params.debug) {
      cerr << "WARNING: computed time differs from file time by secs: "
           << (int) diff << endl;
      cerr << "  computed time: " << DateTime::strn(computedTime) << endl;
      cerr << "  file time: " << DateTime::strn(_fileTime) << endl;
      cerr << "  Setting computed time to file time" << endl;
    }
    computedTime = _fileTime;
  }

}

///////////////////////////////////////////////////
// find a FROM in the input tokens, this is used for a start point
// for line points
//
// Return true if a from found, false otherwise

bool Taf2Spdb::_findFrom(const size_t &startIdx,
                         const size_t &endIdx,
                         const vector <string> &toks,
                         size_t &fromIdx)
{
  // set defaults

  fromIdx=startIdx;

  // find from

  bool found = false;
  for (size_t ii = startIdx; ii < endIdx; ii++) {
    
    if ((toks[ii] == "FROM") || (toks[ii] == "FM")) {
      fromIdx=ii;
      found=true;
      break;
    }
  } // ii

  return found;
}

///////////////////////////////////////////////////
// Is this a location indicator or MWO that occurs after
// the valid time in the ICAO standard. The expected
// format is: VALID nnnnnn/nnnnnn XXXX- where XXXX is
// the location indicator/MWO.
// May want to skip this station when finding points
//
// Return true if found the location indicator/MWO, false otherwise

bool Taf2Spdb::_isLocationIndicator(const size_t &startIdx,
                                    const size_t &endIdx,
                                    const vector <string> &toks,
                                    size_t &idx)
{
  // set defaults

  bool found=false;
  idx=startIdx;

  // Look for the specific pattern of VALID plus a time

  size_t validIdx=0;
  size_t timeIdx=0;
  bool foundValid=false;
  bool foundTime=false;
  for (size_t ii=startIdx; ii<endIdx; ii++) {

    if (toks[ii] == "VALID") {
      foundValid=true;
      validIdx=ii;
    }

    time_t isTime;
    if (_computeStartEndTimes(toks[ii], isTime, isTime) == 0) {
      foundTime=true;
      timeIdx=ii;
      break;
    }
  }
  
  // Did not find the time string

  if (!foundTime) {
    return found;
  }

  // Should we require the string VALID before the time? Not sure
  
  // Return false if there are no characters after the time

  if (timeIdx+1 >= toks.size()) {
    return found;
  }

  // Now look for the location indicator/MWO (after valid time, ICAO standard has 
  // 4-char station followed by dash or equals sign).

  bool possibleStation=false;
  string tok=toks[timeIdx+1];

  // e.g., FCBB- or FCBB=

  if (((_lastString(tok, "-")) || (_lastString(tok, "="))) && (tok.size() == 5)) {
    possibleStation=true;
  }

  // e.g., FCBB - or FCBB =

  if ((!possibleStation) && (timeIdx+2 < endIdx)) {
    if (((_lastString(toks[timeIdx+2], "-")) || 
	 (_lastString(toks[timeIdx+2], "="))) && 
	(tok.size() == 4) && (_allAlpha(tok))) {
      possibleStation=true;
    }
  }
  
  if (possibleStation) {
    found=true;
    idx=timeIdx+1;
  }
  
  return found;
}


///////////////////////////////////////////////////
// find a match in the input tokens
//
// Return true if a width found, false otherwise

bool Taf2Spdb::_findMatchInToks(const string &tok,
                                const size_t &startIdx,
                                const size_t &endIdx,
                                const vector <string> &toks,
                                size_t &idxInToks)
{

  // set defaults

  idxInToks=0;

  // find match between tok and toks[]. Return the index

  bool found = false;
  for (size_t ii = startIdx; ii < endIdx; ii++) {
    if (toks[ii].find(tok, 0) != string::npos) {
      found=true;
      idxInToks=ii;
      break;
    }
  }

  return found;
}


///////////////////////////////////////////////////
// check if all characters in the token are alpha

bool Taf2Spdb::_allAlpha(const string &tok)

{
  for (size_t ii = 0; ii < tok.size(); ii++) {
    if (!isalpha(tok[ii])) {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////
// check if all characters in the token are digits

bool Taf2Spdb::_allDigits(const string &tok)

{
  for (size_t ii = 0; ii < tok.size(); ii++) {
    if (!isdigit(tok[ii])) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////
// check if there are some digits in the token

bool Taf2Spdb::_hasDigit(const string &tok)

{
  for (size_t ii = 0; ii < tok.size(); ii++) {
    if (isdigit(tok[ii])) {
      return true;
    }
  }
  return false;
}


////////////////////////////////////////////////
// check if the station is bogus. 
// 
// Return true if bogus, false if not

bool Taf2Spdb::_isStationBogus(const string &tok)

{
  for (int ii = 0; ii < _params.bogus_stations_n; ii++) {
    if (tok == _params._bogus_stations[ii]) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////
// Is this a NIL (empty) SIGMET?
//
// Return true if a NIL (empty) SIGMET, false otherwise
//

bool Taf2Spdb::_isNil(const size_t startIdx,
                      const vector <string> toks)

{
  for (size_t ii = startIdx; ii < toks.size(); ii++) {
    string tok=toks[ii];
    _stripTermChars(tok, tok);
    
    if (tok == "NIL" ||
	tok == "NILSIG" ||
	tok == "SIGMET...NONE") {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////
// check if the token is a weather token
//
// Return true if a weather token, false if not

bool Taf2Spdb::_isWxTok(const string &tok)

{
  // look through wx types

  for (int ii = 0; ii < _params.wx_translator_n; ii++) {

    // get and tokenize the weather string

    string msgWxStr = _params._wx_translator[ii].message_text;
    vector<string> wxToks;
    _tokenize(msgWxStr, " \n\t\r", wxToks);

    // loop through the tokens in the wx string to try to find a
    // match with the input token

    for (size_t jj = 0; jj < wxToks.size(); jj++) {
      if (wxToks[jj] == tok) {
	return true;
      }
    } // jj
  } // ii

  return false;
}

////////////////////////////////////////////////////
// check if the token is a possible ID token
//
// Return true if a possible ID token, false if not.
// Return the stripped ID token in strippedTok.
//

bool Taf2Spdb::_isIdTok(const string &tok,
                        string &strippedTok)
  
{

  // work on local copy in case tok and stripped tok are the same object

  bool found=false;
  string local = tok;

  if (tok.size() <= 4 && _hasDigit(tok)) {
    // Remove any trailing . or =
    _stripTermChars(tok, local);
    found=true;
  }

  strippedTok = local;
  return found;

}

////////////////////////////////////////////////
// check if the token is a time string. This does not
// work for the ICAO standard valid times which do not
// have a Z in them.
// 
// Return true if a time, false if not. Also return in the
// desired DateTime format

bool Taf2Spdb::_isTimeTok(const string &tok, time_t &isTime)

{

  // is it all alpha? not a time

  if (_allAlpha(tok)) {
    return false;
  }

  // does it have digits? if not, not a time

  if (!_hasDigit(tok)) {
    return false;
  }
  
  // Does it end in a Z or UTC? If so, remove the Z or UTC and all else 
  // should be digits

  string tmptok;
  if (_lastString(tok, "Z")) {
    _stripTrailingSpacer("Z", tok, tmptok);
  } else if (_lastString (tok, "UTC")) {
    _chopSpacer2End("UTC", tok, tmptok);
  } else {
    tmptok=tok;
  }

  if (!_allDigits(tmptok)) {
    return false;
  } 

  // is the string 4 or 6 chars long

  if ((tmptok.size() != 4) && (tmptok.size() != 6)) {
    return false;
  }

  // Now parse different formats

  int day = 1, hour, min;
  bool hasDay=false;

  // DDHHMM

  if (tmptok.size() == 6) {
    if (sscanf(tmptok.c_str(), "%2d%2d%2d", &day, &hour, &min) != 3) {
      return false;
    }
    hasDay=true;
  }

  // HHMM

  if (tmptok.size() == 4) {
    if (sscanf(tmptok.c_str(), "%2d%2d", &hour, &min) != 2) {
      return false;
    }
  }

  if (day < 1 || day > 31) {
    return false;
  }
  if (hour < 0 || hour > 24) {
    return false;
  }
  if (min < 0 || min > 59) {
    return false;
  }

  // Reformat the time

  DateTime newTime(_fileTime);
  if (hasDay) {
    newTime.setDay(day);
  }
  newTime.setHour(hour);
  newTime.setMin(min);
  newTime.setSec(0);
  isTime = newTime.utime();
  _correctTimeForMonthChange(isTime);

  return true;
}


////////////////////////////////////////////////////
// convert a speed to km/hr.
//
// Return true if can do the conversion, false if not
//

bool Taf2Spdb::_speed2Kph(const string &speed, double &speed_kph)

{
  // set default

  speed_kph = -1;

  // return if not a speed token

  if (_isSpeedTok(speed) != 1) {
    return false;
  }

  // Setup the units array

  vector <string> unitStrings;
  unitStrings.clear();
  
  unitStrings.push_back("KPH");
  unitStrings.push_back("KMH");
  unitStrings.push_back("KTS");
  unitStrings.push_back("KT");


  // Split the digits from the units.  String is assumed to be
  // of the form xxxUNITSTRING

  for (size_t ii=0; ii<unitStrings.size(); ii++) {
    size_t unitPos=speed.find(unitStrings[ii], 0);

    if (unitPos != string::npos) {

      // found the unit string now need the digits which is the
      // remaining string after the units are stripped off

      size_t lenUnitString=unitStrings.size();
      string digitString=speed;
      digitString.erase(unitPos, lenUnitString);

      // bail if this is not all digits
  
      if (!_allDigits(digitString)) {
	return false;
      }

      // Convert the string to integer

      const char *start = digitString.c_str();
      int speedInt;
      if (sscanf(start, "%d", &speedInt) != 1) {
	return false;
      }
    
      // If this is kph, do not need to convert

      if ((unitStrings[ii] == "KPH") || (unitStrings[ii] == "KMH")) {
	speed_kph=(double)speedInt;
      }

      // If this is knots, do conversion
      
      if (unitStrings[ii].find("KT", 0) != string::npos) {
	double speed_ms=(double)speedInt * KNOTS_TO_MS;
	speed_kph=speed_ms * MPERSEC_TO_KMPERHOUR;
      }

      // break since we did find the unit string

      break;
    }
  }

  if (speed_kph != -1) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////
// check if the token contains a speed
//
// Return 1 if contains a speed and units, 0 if no units,
// 2 if contains units and no speed
//

int Taf2Spdb::_isSpeedTok(const string &tok)

{
  if (_allDigits(tok)) {
    return 0;
  }

  bool found=false;
  if ((tok.find("KMH", 0) != string::npos) || 
      (tok.find("KT", 0) != string::npos) ||
      (tok.find("KM/H", 0) != string::npos)) {
    found=true;
  }

  if (found && (_hasDigit(tok))) {
    return 1;
  }

  if (found) {
    return 2;
  }

  return 0;
}

////////////////////////////////////////////////////
// check if the token contains a N/S/E/W direction
//
// Return true if contains a direction, false if not
//

////////////////////////////////////////////////////
// check if the token(s) contain a movement
//
// Return true if contains a movement, false if not
//

bool Taf2Spdb::_isMovementTok(const vector <string> &toks, 
                              const size_t &moveIdx,
                              vector <size_t> &moveToks,
                              string &moveType,
                              string &moveDir,
                              string &moveSpeed)

{

  // Set defaults

  moveType="UNKNOWN";
  moveDir="UNKNOWN";
  moveSpeed="UNKNOWN";
  moveToks.clear();

  string func_name="_isMovementTok";
  bool found=false;
  bool speed2toks=false;
  string tok=toks[moveIdx];
  size_t dirTok=moveIdx+1;
  size_t speedTok=moveIdx+2;

  if (tok == "MOV" || 
      tok == "MOVING" || 
      tok == "MOVG") {

    moveType="MOV";

    if ((dirTok < toks.size()) /* && (_isDirTok(toks[dirTok]))*/) {
      moveDir=toks[dirTok];
      found = true;
    } else {
      found=false;
    }

    // Special case for US. Get movement speed as dddssKT

    if ((!found) && ((toks[dirTok].find("KT", 0) != string::npos) ||
		     (toks[dirTok] == "FROM"))) {
      string tmpMoveDir=toks[dirTok];
      size_t tmpDirIdx=dirTok;
      if ((tmpMoveDir == "FROM") && (speedTok < toks.size())) {
	tmpMoveDir=toks[speedTok];
	tmpDirIdx=speedTok;
      }

      if (tmpMoveDir.find("KT", 0) != string::npos) {
	      
	// Strip trailing . if there is one

	_stripTrailingSpacer(".", tmpMoveDir, tmpMoveDir);

	// Is this the right number of character?

	if (tmpMoveDir.length() == 7) {
	  moveDir=tmpMoveDir.substr(0, 3);
	  moveSpeed=tmpMoveDir.substr(3, tmpMoveDir.length());

	  found=true;

	  // Set the return array of indices used and return

	  moveToks.push_back(moveIdx);
	  moveToks.push_back(tmpDirIdx);
	  return found;
	}
      }
    }
  }

  // Return if we have not found a direction

  if (!found) {
    return found;
  }

  // speed token may occur as 1 token, e.g., 10KT 
  // or 2 tokens, e.g., 10 KT

  // First test the 1 token case - this should be speed and units

  if (speedTok < toks.size() && (_isSpeedTok(toks[speedTok]) == 1)) {
    moveSpeed=toks[speedTok];
  }

  // Next test the 2 token case. Assemble the speed as a single token from the
  // 2 tokens
    
  if ((moveSpeed == "UNKNOWN") && (speedTok+1 < toks.size())) {
    if ((_isSpeedTok(toks[speedTok+1]) == 2) && (_allDigits(toks[speedTok]))) {
      moveSpeed=toks[speedTok] + toks[speedTok+1];
      speed2toks=true;
    }
  }
  
  // Is the speed a composite string? e.g., 15/25 or 15-25. We need to find
  // the max or higher speed. First remove the units.

  string speeds;
  string units;
  vector <string> speedsToks;
  bool compositeSpeed=false;
  size_t unitPos=moveSpeed.find("KM", 0);
  if (unitPos == string::npos) {
    unitPos=moveSpeed.find("KT", 0);
  }
  if (unitPos != string::npos) {
    speeds=moveSpeed.substr(0, unitPos);
    units=moveSpeed.substr(unitPos, string::npos);
    if (speeds.find("/", 0) != string::npos) {
      _tokenize(speeds, "/", speedsToks);
      compositeSpeed=true;
    }
    else if (speeds.find("-", 0) != string::npos) {
      _tokenize(speeds, "-", speedsToks);
      compositeSpeed=true;
    }
  }
  else {
    found=false;
  }

  if (compositeSpeed) {
    int maxSpeed=0;
    size_t idx=9999;

    for (size_t ii=0; ii<speedsToks.size(); ii++) {
      if (_allDigits(speedsToks[ii])) {
	int ispeed=-1;
	int len=speedsToks[ii].length();
	bool found=false;

	if (len == 3) {
	  if (sscanf(speedsToks[ii].c_str(), "%3d", &ispeed) == 1) {
	    found=true;
	  }
	} else if (len == 2) {
	  if (sscanf(speedsToks[ii].c_str(), "%2d", &ispeed) == 1) {
	    found=true;
	  }
	} else if (len == 1) {
	  if (sscanf(speedsToks[ii].c_str(), "%1d", &ispeed) == 1) {
	    found=true;
	  }
	}
	if (found && ispeed > maxSpeed) {
	  maxSpeed=ispeed;
	  idx=ii;
	}
      } //endif _allDigits()
    } //endfor

    if (maxSpeed > 0) {
      moveSpeed=speedsToks[idx] + units;
    }
    else {
      found=false;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "isMovementTok: maxSpeed: " << maxSpeed << ", units: "
           << units << ", original moveSpeed: " << moveSpeed << endl;
    }

  }

  // Set the return array of indices used

  if (found) {
    moveToks.push_back(moveIdx);
    moveToks.push_back(dirTok);      
    if (moveSpeed != "UNKNOWN") {
      moveToks.push_back(speedTok);
      if (speed2toks) {
	moveToks.push_back(speedTok+1);
      }
    }
  }

  return found;

}

/////////////////////////////////////////////////////
// Find the ID and qualifier
//
// Return true if found, false otherwise
// 
//
bool Taf2Spdb::_findIdAndQualifier(const size_t &startIdx,
                                   const vector <string> &toks,
                                   string &id,
                                   string &qualifier,
                                   bool &foundId,
                                   bool &foundQualifier,
                                   vector <size_t> &usedIdxs)

{
  // Set defaults
  
  bool found=false;
  string func_name="_findIdAndQualifier";

  foundId=false;
  foundQualifier=false;
  id="NULL";
  qualifier="NULL";
  usedIdxs.clear();

  size_t tokNum1=startIdx;
  size_t tokNum2=tokNum1+1;

  if (tokNum1 >= toks.size() || tokNum2 >= toks.size()) {
    cerr << func_name << ": toks array too small to search" << endl;
    return found;
  }
   
  string tok1 = toks[tokNum1];
  string tok2 = toks[tokNum2];

  // Try the first token

  string idTok = tok1;
  if (_isIdTok(idTok, idTok)) {
    foundId=true;
    id=idTok;
    usedIdxs.push_back(tokNum1);

    //    cerr << func_name << ": found id in tok1: " << id << endl;

  } else {

    // Try the second token. The first must be a qualifier
    // if the second is an ID.

    string qualifierTok = idTok;
    
    // Continue to see if there is a ID in the second position

    string idTok2 = tok2;
    if (_isIdTok(idTok2, idTok2)) {

      foundId=true;
      foundQualifier=true;

      id=idTok2;
      qualifier=qualifierTok;

      usedIdxs.push_back(tokNum1);
      usedIdxs.push_back(tokNum2);

      _mergeIdAndQualifier(id, qualifier);
      
    } else {
      // cannot find ID
      return found;
    }
  }

  // set the final return

  if (foundId) {
    found=true;
  }

  return found;

}

/////////////////////////////////////////////////////
// Merge the ID and qualifier if appropriate
// returns 0 on success, -1 on failure
// On success, id is merged.
// On failure, id is unchanged.

int Taf2Spdb::_mergeIdAndQualifier(string &id,
                                   const string &qualifier)

{

  string mergedId = id;
  if (qualifier == "ALPHA") {
    mergedId = "A" + id;
  } else if (qualifier == "BRAVO") {
    mergedId = "B" + id;
  } else if (qualifier == "CHARLIE") {
    mergedId = "C" + id;
  } else if (qualifier == "DELTA") {
    mergedId = "D" + id;
  } else if (qualifier == "ECHO") {
    mergedId = "E" + id;
  } else if (qualifier == "FOXTROT") {
    mergedId = "F" + id;
  } else if (qualifier == "GOLF") {
    mergedId = "G" + id;
  } else if (qualifier == "HOTEL") {
    mergedId = "H" + id;
  } else if (qualifier == "INDIA") {
    mergedId = "I" + id;
  } else if (qualifier == "JULIET") {
    mergedId = "J" + id;
  } else if (qualifier == "KILO") {
    mergedId = "K" + id;
  } else if (qualifier == "LIMA") {
    mergedId = "L" + id;
  } else if (qualifier == "MIKE") {
    mergedId = "M" + id;
  } else if (qualifier == "NOVEMBER") {
    mergedId = "N" + id;
  } else if (qualifier == "OSCAR") {
    mergedId = "O" + id;
  } else if (qualifier == "PAPA") {
    mergedId = "P" + id;
  } else if (qualifier == "QUEBEC") {
    mergedId = "Q" + id;
  } else if (qualifier == "ROMEO") {
    mergedId = "R" + id;
  } else if (qualifier == "SIERRA") {
    mergedId = "S" + id;
  } else if (qualifier == "TANGO") {
    mergedId = "T" + id;
  } else if (qualifier == "UNIFORM") {
    mergedId = "U" + id;
  } else if (qualifier == "VICTOR") {
    mergedId = "V" + id;
  } else if (qualifier == "WISKEY") {
    mergedId = "W" + id;
  } else if (qualifier == "XRAY") {
    mergedId = "X" + id;
  } else if (qualifier == "YANKEE") {
    mergedId = "Y" + id;
  } else if (qualifier == "ZULU") {
    mergedId = "Z" + id;
  } else {
    return -1;
  }

  id = mergedId;
  return 0;

}


////////////////////////////////////////////////
// strip leading spacer character(s)

void Taf2Spdb::_stripLeadingSpacer(const string &spacer,
                                   const string &instr,
                                   string &outstr)

{

  // work on local copy in case instr and outstr are the same object

  string local = instr;

  // look for the spacer
  // This seems to have some problems.
  // Not getting expected returns from find_first_of()
  // or find_first_not_of() so re-implement in a more brittle fashion.
  //   size_t start_spacer = instr.find_first_of(spacer, pos);
  //   size_t start_notspacer = instr.find_first_not_of(spacer, pos);

  size_t pos = 0;
  size_t start_spacer = instr.find(spacer, pos);
  int len_spacer = spacer.length();
  
  // spacer is not in the string or is not the first character in the string

  if ((start_spacer == string::npos) ||
      (start_spacer != 0)) {
    outstr = local;
    return;
  }
  
  // Remove the spacer from the instr

  outstr.assign(instr, len_spacer, string::npos);
  outstr = local;
  
}

////////////////////////////////////////////////
// strip trailing spacer character(s)

void Taf2Spdb::_stripTrailingSpacer(const string &spacer,
                                    const string &instr,
                                    string &outstr)

{

  // work on local copy in case instr and outstr are the same object

  string local = instr;

  // look for the spacer
  // This seems to have some problems.
  // Not getting expected returns from find_last_of()
  // or find_last_not_of() so re-implement in a more brittle fashion.
  //   size_t end_spacer = instr.find_last_of(spacer, pos);
  //   size_t end_notspacer = instr.find_last_not_of(spacer, pos);

  size_t pos = 0;
  size_t start_spacer = instr.find(spacer, pos);
  size_t len = instr.length();
  size_t len_spacer = spacer.length();

  // spacer is not in the string or occurs before the end. Note that length()
  // is 1-based and find() is 0-based.

  if ((start_spacer == string::npos) ||
      (start_spacer == len) ||
      (start_spacer < (len-len_spacer-1))) {
    outstr = local;
    return;
  }
  
  // Remove the spacer from the instr

  local.assign(instr, 0, len-len_spacer);
  outstr = local;

}

////////////////////////////////////////////////
// chop a string starting at the input spacer

void Taf2Spdb::_chopSpacer2End(const string &spacer,
                               const string &instr,
                               string &outstr)

{

  // work on local copy in case instr and outstr are the same object

  string local = instr;

  // look for the spacer
  // This seems to have some problems.
  // Not getting expected returns from find_last_of()
  // or find_last_not_of() so re-implement in a more brittle fashion.
  //   size_t end_spacer = instr.find_last_of(spacer, pos);
  //   size_t end_notspacer = instr.find_last_not_of(spacer, pos);

  size_t pos = 0;
  size_t start_spacer = instr.find(spacer, pos);
  size_t len = instr.length();

  // spacer is not in the string

  if ((start_spacer == string::npos) ||
      (start_spacer == len)) {
    outstr = local;
    return;
  } 
  
  // Remove the spacer and any chars after it from the instr

  local.assign(instr, 0, start_spacer);
  outstr = local;

}


////////////////////////////////////////////////
// strip trailing termination characters

void Taf2Spdb::_stripTermChars(const string &instr,
                               string &outstr)

{

  // work on local copy in case instr and outstr are the same object

  string local = instr;

  if (_lastString(instr, ".")) {
    _stripTrailingSpacer(".", instr, local);
  }

  if (instr.find("=", 0) != string::npos) {
    _chopSpacer2End("=", instr, local);
  }

  outstr = local;

}


////////////////////////////////////////////////
// Return the number of times the wantstr occurs in the tok

int Taf2Spdb::_findStringNTimes(const string &tok,
                                const string &wantstr)

{

  int nfound=0;

  int len=wantstr.length();
  bool done=false;
  size_t pos=0;
  while (!done) {
    pos = tok.find(wantstr, pos);
    if (pos == string::npos) {
      done = true;
    } else {
      nfound++;
    }
    pos=pos+len;
  }
  return nfound;
}



////////////////////////////////////////////////
// check if the input wantstr is the last string in the token

bool Taf2Spdb::_lastString(const string &tok,
                           const string &wantstr)


{
  size_t pos = tok.rfind(wantstr);
  size_t len= tok.length();
  size_t wantstrlen= wantstr.length();

  if ((pos != string::npos) && (pos == len - wantstrlen)){
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////
// check if the input wantstr is the first string in the token

bool Taf2Spdb::_firstString(const string &tok,
                            const string &wantstr)


{
  size_t pos = tok.find(wantstr);

  if ((pos != string::npos) && (pos == 0)){
    return true;
  } else {
    return false;
  }
}


/////////////////////////////////////////////////
// Look for TEST tokens
//
// Return true if TEST found minNumTest times, 
// false otherwise
// 

bool Taf2Spdb::_isTestMessage(size_t &startIdx,
                              const vector <string> &toks,
                              const int &minNumTest)

{
  bool found=false;
  int nFound=0;

  for (size_t ii = startIdx; ii < toks.size(); ii++) {
    if (toks[ii] == "TEST") {
      nFound++;
    }
  }
  
  if (nFound >= minNumTest) {
    found=true;
  }

  return found;
      
}

/////////////////////////////////////////////////
// Look for CORRECT or AMEND tokens
//
// Return true if CORRECT or AMEND token found,
//      false otherwise. 
// Returns the ID and qualifier of the sig/airmet to
//      be corrected/amended
// 

bool Taf2Spdb::_isAmendMessage(size_t &startIdx,
                               const vector <string> &toks,
                               string &id,
                               string &qualifier)

{
  // Set defaults

  bool found=false;
  id="NULL";
  qualifier="NULL";
  size_t foundIdx=0;
  for (size_t ii = startIdx; ii < toks.size(); ii++) {
    if (toks[ii] == "CORRECT" || 
	toks[ii] == "CORRECTS" ||
	toks[ii] == "AMEND" ||
	toks[ii] == "AMENDS") {
      found=true;
      foundIdx=ii;
      break;
    }
  }
  
  if (!found) {
    return found;
  }

  // Can we find the id and qualifier of the corrected/amended
  // sig/airmet? Skip the SIGMET/AIRMET word if found

  if (foundIdx+1 >= toks.size()) {
    return found;
  }

  if (toks[foundIdx+1] == "SIGMET" || 
      toks[foundIdx+1] == "AIRMET") {
    foundIdx=foundIdx+1;
  }

  string possId, possQualifier;
  bool foundId, foundQualifier;
  vector <size_t> usedIdxs;
  if (_findIdAndQualifier(foundIdx+1, toks, possId, possQualifier, 
                          foundId, foundQualifier, usedIdxs)) {
    if (foundId) {
      id=possId;
    }
    if (foundQualifier) {
      qualifier=possQualifier;
    }
    found=true;
  }

  return found;
      
}

////////////////////////////////////////////////
// find if a CANCEL in the message
// determine which ID to cancel
//
// Sets _doCancel, _cancelId and _cancelGroup

void Taf2Spdb::_checkCancel(const size_t &startIdx,
                            const vector <string> toks)

{

  _doCancel = false;
  // _cancelGroup = _decoded.getGroup();
  _cancelId = _decoded.getStationId();

  // Search for a cancel flag

  bool found = false;
  bool foundCancelledStr = false;
  size_t cnlIdx=0;

  for (size_t ii = startIdx; ii < toks.size(); ii++) {
    const string &tok = toks[ii];

    if (tok.find("CANCELLED", 0) != string::npos) {
      found=true;
      cnlIdx=ii;
      foundCancelledStr=true;
      break;
    }
    if (tok.find("CANCEL", 0) != string::npos) {
      found=true;
      cnlIdx=ii;
      break;
    }
    if (tok == "CNL" || tok == "CNCL") {
      found=true;
      cnlIdx=ii;
      break;
    }
    // want CNL but not OCNL
    if ((tok.find("CNL", 0) != string::npos) &&
        (tok.find("OCNL",0) == string::npos)) {
      found=true;
      cnlIdx=ii;
      break;
    }
  }
  
  if (!found) {
    return;
  }

  if (foundCancelledStr) {
    
    // special case for "CANCELLED"
    
  } else if (cnlIdx < (toks.size() - 1) && toks[cnlIdx+1] == "WEF") {

    // special case for "CNL WEF"

  } else {
    
    // Require the cancel string to be followed by
    // a SIGMET or AIRMET ID
    
    if (cnlIdx > toks.size() - 2) {
      return;
    }
    
    if (toks[cnlIdx+1] == "SIGMET") {
      // _cancelGroup = SIGMET_GROUP;
    } else if (toks[cnlIdx+1] == "AIRMET") {
      // _cancelGroup = AIRMET_GROUP;
    } else {
      return;
    }

    string idTok=toks[cnlIdx+2];
    _cancelId = idTok;
    if (_isIdTok(idTok, idTok)) {
      _cancelId = idTok;
    } else {
      if (cnlIdx < toks.size() - 3) {
        string qualifier = toks[cnlIdx+2];
        string idd = toks[cnlIdx+3];
        if (_mergeIdAndQualifier(idd, qualifier) == 0) {
          _cancelId = idd;
        }
      }
    }

  }

  _doCancel = true;

}

/////////////////////////////////////////////////
// Look for REISSUE tokens
//
// Return true if REISSUE token found,
//      false otherwise. 
// Returns the ID and qualifier of the sig/airmet to
//      be reissued
// 

bool Taf2Spdb::_isReissueMessage(size_t &startIdx,
                                 const vector <string> &toks,
                                 string &id,
                                 string &qualifier)

{
  // Set defaults

  bool found=false;
  id="NULL";
  qualifier="NULL";
  size_t foundIdx=0;
  for (size_t ii = startIdx; ii < toks.size(); ii++) {
    if (toks[ii] == "REISSUE" || 
	toks[ii] == "REISSUED") {
      found=true;
      foundIdx=ii;
      break;
    }
  }
  
  if (!found) {
    return found;
  }

  // Can we find the id and qualifier of the corrected/amended
  // sig/airmet? Need to skip weird words after REISSUE

  if (foundIdx+1 >= toks.size()) {
    return found;
  }

  if (toks[foundIdx+1] == "AS") {
    foundIdx=foundIdx+1;
  }

  string possId, possQualifier;
  bool foundId, foundQualifier;
  vector <size_t> usedIdxs;
  if (_findIdAndQualifier(foundIdx+1, toks, possId, possQualifier, 
			  foundId, foundQualifier, usedIdxs)) {
    if (foundId) {
      id=possId;
    }
    if (foundQualifier) {
      qualifier=possQualifier;
    }
    found=true;
  }

  return found;
      
}

/////////////////////////////////////////////////
// Look for SEE and SEE ALSO tokens
//
// Return true if SEE or SEE ALSO token found,
//      false otherwise. 
// Returns the ID and qualifier of the sig/airmet to
//      be seen
// 

bool Taf2Spdb::_isSeeAlsoMessage(size_t &startIdx,
                                 const vector <string> &toks,
                                 string &id,
                                 string &qualifier)

{
  // Set defaults

  bool found=false;
  id="NULL";
  qualifier="NULL";
  size_t foundIdx=0;
  for (size_t ii = startIdx; ii < toks.size(); ii++) {
    if (toks[ii] == "SEE") {
      foundIdx=ii;
      found=true;
      break;
    }
  }
  
  if (!found) {
    return false;
  }
  
  // Look for the SIGMET or AIRMET token within the next 2 tokens
  // need to sometimes skip a token due to the word ALSO or INTL

  if (foundIdx+3 >= toks.size()) {
    return false;
  }

  found=false;
  for (size_t ii = foundIdx+1; ii < foundIdx+3; ii++) {
    if (toks[ii] == "SIGMET" || toks[ii] == "AIRMET") {
      foundIdx=ii;
    }
  }

  // Can we find the id and qualifier of the see also
  // sig/airmet?

  if (foundIdx+1 >= toks.size()) {
    return false;
  }

  found=false;
  string possId, possQualifier;
  bool foundId, foundQualifier;
  vector <size_t> usedIdxs;
  if (_findIdAndQualifier(foundIdx+1, toks, possId, possQualifier, 
			  foundId, foundQualifier, usedIdxs)) {
    if (foundId) {
      id=possId;
    }
    if (foundQualifier) {
      qualifier=possQualifier;
    }
    found=true;
  }

  return found;
      
}

//////////////////////////////////////////////
// tokenize a string into a vector of strings.
// use this version if you have a string spacer
// e.g. TO that could have characters in common
// with other strings. Use tokenize() if you have
// a unique character

void Taf2Spdb::_tokenizeStr(const string &str,
                            const string &spacer,
                            vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t startSpacer = str.find(spacer, pos);
    if (startSpacer != string::npos) {
      string tok;
      tok.assign(str, pos, startSpacer - pos);
      toks.push_back(tok);
    } else {
      string tok;
      tok.assign(str, pos, string::npos);
      toks.push_back(tok);
      return;
    }
    pos = startSpacer + spacer.length();
  }
}


#ifdef JUNK

////////////////////////////////////////////////
// find and set the id and qualifier
//
// Returns 0 on success, -1 on failure

int Taf2Spdb::_setId()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setId()" << endl;
  }

  // Try the first 2 positions after the SIGMET/AIRMET token

  string id, qualifier;
  bool foundId, foundQualifier;
  vector <size_t> usedIdxs;
  if (_findIdAndQualifier(_startTokNum+1, _toks,
                          id, qualifier,
                          foundId, foundQualifier,
                          usedIdxs)) {
    if (foundId) {
      _decoded.setStationId(id);
    }
//     if (foundQualifier) {
//       _decoded.setQualifier(qualifier);
//     }

    // set the used array
    for (size_t ii=0; ii<usedIdxs.size(); ii++) {
      size_t idx=usedIdxs[ii];
      _used[idx] = true;
    }

    // debug

    if (_params.print_decode_problems_to_stderr) {
      if (!foundQualifier) {
        cerr << "ID: " << id << endl;
      } else {
        cerr << "ID: " << _decoded.getStationId() << endl;
      }
    }

  } else {
    // cannot find ID - invalid report
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// handle an amendment or correction. Get the ID of the
// sig/airmet to be amended or corrected
//

void Taf2Spdb::_setAmend()
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setAmend()" << endl;
  }

  // Need to start the search at idx 0 rather than _startTokNum
  // because occasionally AMEND or CORRECT is a modifier to the
  // initial SIG/AIRMET token
  
  string amendId, amendQualifier;
  size_t startIdx=0;
  if (_isAmendMessage(startIdx, _toks, amendId, amendQualifier)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "This is an amend/correction: ID: " << amendId;
      if (amendQualifier != "NULL") {
	cerr << ", qualifier: " << amendQualifier;
      }
      cerr << endl;
    }
  }
}

////////////////////////////////////////////////
// set the weather type
// returns 0 on success, -1 on failure

int Taf2Spdb::_setWx()

{
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setWx()" << endl;
  }

  // Skip types for US AIRMETS. This is set in _setUSAirmetID()
  
  // start search just beyond the SIGMET/AIRMET location, but
  // need to check if US so skip header (and do not find VA the
  // state as VA volcanic ash!)
  
  size_t markTok = _startTokNum + 1;
  _mayBeUS=_skipUSHeader(_toks, markTok, _toks.size(), markTok);

  // search through tokens looking for match with possible wx types
  
  for (size_t ii = markTok; ii < _toks.size(); ii++) {

    string wxType;
    if (_getWxType(ii, wxType) == 0) {
      _decoded.setWx(wxType);
      if (wxType.find("TC", 0) != string::npos) {
        _tcFound = true;
      }
      if (_params.print_decode_problems_to_stderr) {
        cerr << "Weather: " << wxType << endl;
      }
      return 0;
    }

  } // ii

  if (_params.print_decode_problems_to_stderr) {
    cerr << "Cannot match wx string" << endl;
  }

  _decoded.setWx("UNKNOWN");

  return -1;

}

////////////////////////////////////////////////
// get the weather type if we can
//
// Weather string may be split up, so tokens
// are searched for in order
//
// Returns 0 on success, -1 on error

int Taf2Spdb::_getWxType(int startPos, string &wxType)

{
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setWxType()" << endl;
  }
  
  // look through wx types - the long strings are first, so
  // that the more well-defined ones will be found first
  
  for (int ii = 0; ii < _params.wx_translator_n; ii++) {
    
    // get and tokenize the weather string
    
    string msgWxStr = _params._wx_translator[ii].message_text;
    vector<string> wxToks;
    _tokenize(msgWxStr, " \n\t\r", wxToks);
    
    // loop through the tokens in the wx string, checking for a match
    // in each token
    
    bool match = true;
    int pos = startPos;
    for (int jj = 0; jj < (int) wxToks.size(); jj++, pos++) {
      int pos = startPos + jj;
      if (pos >= (int) _toks.size()) {
        match = false;
        break;
      }
      if (wxToks[jj] != _toks[pos]) {
        match = false;
        break;
      }
    }

    // if match found, success

    if (match) {
      wxType = _params._wx_translator[ii].standard_text;
      _lastTokUsed = startPos + wxToks.size() - 1;
     return 0;
    }

  } // ii

  // failure

  return -1;

}

////////////////////////////////////////////////
// find and set the source station name
//

void Taf2Spdb::_setSource()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setSource()" << endl;
  }

  bool confirmedFound = false;
  string possibleSource, confirmedSource;
  for (size_t ii = 0; ii < _startTokNum; ii++) {
    const string &tok = _toks[ii];
    if ((tok.size() == 4) && (tok != "CNCL")) {
      // possible ICAO identifier - check if it is in the list
      double lat, lon, alt;
      string type;
      if (_stationLoc.FindPosition(tok, lat, lon, alt, type) == 0) {
        confirmedSource = tok;
        confirmedFound = true;
        _decoded.setCentroid(lat, lon);
        _decoded.setSource(confirmedSource);
      } else {
        if (_allAlpha(tok)) {
          possibleSource = tok;
          _decoded.setSource(possibleSource);
        }
      }
    }
  }

  if (_params.print_decode_problems_to_stderr) {
    cerr << "Source: " << _decoded.getSource() << endl;
  }

  if ((!confirmedFound) && ((_params.debug) ||
                            (_params.print_decode_problems_to_stderr))) {
    cerr << "   setSource: cannot confirm "
         << _decoded.getSource() << " in st_location_path, will use anyway"
         << endl;
  }

}

////////////////////////////////////////////////
// find and set the times
//

int Taf2Spdb::_setTimes()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setTimes()" << endl;
  }

  // set the issue time
   
  time_t issueTime = _fileTime;
  for (size_t ii = 0; ii < _startTokNum; ii++) {
    const string &tok = _toks[ii];
    if (_computeIssueTime(tok, issueTime) == 0) {
      break;
    }
  }
   
  // correct issue time for closeness to file time, and store

  _correctTimeForFileTime(issueTime);
  _decoded.setIssueTime(issueTime);

  // set the start and end times

  time_t startTime, endTime;
  if (_findStartEndTimes(startTime, endTime)) {
    // check for US airmet
    if (_isUSAirmet && !_isUSAirmetHeader &&
        (_usAirmetHdrInfo.start_time > 0) &&
        (_usAirmetHdrInfo.end_time > 0)) {
      startTime = _usAirmetHdrInfo.start_time;
      endTime = _usAirmetHdrInfo.end_time;
      if (_params.debug) {
        cerr << "setTimes: using header times, start: "
             << utimstr(startTime) << ", end: "
             << utimstr(endTime) << endl;
      }
    } else {
      return -1;
    }
  }
  
  // If start time is later than end time, set both times
  // to the issue time
  
  if (startTime > endTime) {
    if (_params.debug) {
      cerr << "ERROR - start is greater than end time" << endl;
      cerr << "  Start time: " << utimstr(startTime) << endl;
      cerr << "  End time: " << utimstr(endTime) << endl;
      cerr << "  Setting start and end times to issue time." << endl;
    }
    startTime = issueTime;
    endTime = issueTime;
  }

  // if duration is excessive, set both times to issue time

  double maxDuration = _params.sigmet_max_duration;
  if (_decoded.getGroup() == AIRMET_GROUP) {
    maxDuration = _params.airmet_max_duration;
  }
  double duration = (double) endTime - (double) startTime;
  if (duration > maxDuration) {
    if (_params.debug) {
      cerr << "ERROR - duration exceeds secs: "
           << maxDuration << endl;
      cerr << "  Setting start and end times to issue time." << endl;
    }
    startTime = issueTime;
    endTime = issueTime;
  }

  // if start time is more than max_duration after the issue time,
  // set both start and end times to issue time

  double diff = (double) startTime - (double) issueTime;
  if (fabs(diff) > maxDuration) {
    if (_params.debug) {
      cerr << "ERROR - start time differs from issue time by more than secs: "
           << maxDuration << endl;
      cerr << "  Setting start and end times to issue time." << endl;
    }
    startTime = issueTime;
    endTime = issueTime;
  }

  // If a US AIRMET header, set the start and end times

  if (_isUSAirmet) {
    _usAirmetHdrInfo.start_time = startTime;
    _usAirmetHdrInfo.end_time = endTime;
  }

  _decoded.setStartTime(startTime);
  _decoded.setEndTime(endTime);

  // Debug
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "setTimes: start: " << utimstr(startTime)
         << ", end: " << utimstr(endTime) << endl;
  }

  return 0;

}


////////////////////////////////////////////////
// set observed time if possible
//

int Taf2Spdb::_setObsTime(const string &timeStr)

{

  int hour, min;
  if (sscanf(timeStr.c_str(), "%2d%2d", &hour, &min) != 2) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Could not set observed time from str: " << timeStr << endl;
    }
    return -1;
  }

  // fill out based on the issue time
  
  time_t issueTime = _decoded.getIssueTime();
  DateTime obsTime(issueTime);
  obsTime.setHour(hour);
  obsTime.setMin(min);
  time_t obsUtime = obsTime.utime();
  
  // correct for midnight passage
  double diff = (double) obsUtime - (double) issueTime;
  if (diff > 43200) {
    obsUtime -= 86400;
  } else if (diff < -43200) {
    obsUtime += 86400;
  }

  _decoded.setObsTime(obsUtime);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "set observed time: " << DateTime::strm(obsUtime) << endl;
  }

  return 0;

}


////////////////////////////////////////////////
// over-write cancelled sigmet

void Taf2Spdb::_overwriteCancelled()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _overwriteCancelled()" << endl;
  }

  if (_params.debug) {
    cerr << "Searching to cancel "
         << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
         << " "
         << _decoded.getSource() << ":" << _cancelId << endl;
  }

  // set the data type

  si32 data_type = Spdb::hash4CharsToInt32(_decoded.getSource());

  // grab interval holding the entry to cancel
  
  DsSpdb in;
  if(in.getInterval(_params.cancel_input_url,
                    _decoded.getIssueTime() - SECS_IN_DAY,
                    _decoded.getIssueTime(),
                    data_type)) {
    if (_params.debug) {
      cerr << "ERROR - Taf2Spdb::_overwriteCancelled" << endl;
      cerr << "  Cannot retrieve "
           << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
           << " "
           << _decoded.getSource() << ":" << _cancelId << endl;
      cerr << "  " << in.getErrStr() << endl;
    }
    return;
  }

  // get chunk vector
  
  const vector<Spdb::chunk_t> &chunks = in.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug) {
      cerr << "ERROR - Taf2Spdb::_overwriteCancelled" << endl;
      cerr << "  Cannot find "
           << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
           << " "
	   << _decoded.getSource() << ":" << _cancelId << endl;
    }
    return;
  }
  
  for (int ii = chunks.size() - 1; ii >= 0; ii--) {
    
    SigAirMet candidate;
    candidate.disassemble(chunks[ii].data, chunks[ii].len);
    
    if (candidate.getId() != _cancelId ||
        candidate.getGroup() != _cancelGroup) {
      // not the same ID and group
      continue;
    }

    if (candidate.getCancelFlag()) {
      // already cancelled
      if (_params.debug) {
        cerr << "-->> Previously cancelled" << endl;
      }
      continue;
    }

    if (_params.debug) {
      cerr << "Cancelling "
           << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
           << " "
           << _decoded.getSource() << ":" << _cancelId << endl;
      cerr << "================== text =======================" << endl;
      cerr << candidate.getText() << endl;
      cerr << "===============================================" << endl;
    }

    time_t cancelTime = _decoded.getIssueTime();
    candidate.setCancelFlag(true);
    candidate.setCancelTime(cancelTime);
    
    // add to spdb objects as appropriate
    
    candidate.assemble();
    
    DsSpdb out;
    time_t expireTime = chunks[ii].expire_time;
    if (expireTime > cancelTime) {
      expireTime = cancelTime;
    }
    out.addPutChunk(chunks[ii].data_type,
		    chunks[ii].valid_time,
		    expireTime,
		    candidate.getBufLen(),
		    candidate.getBufPtr(),
		    chunks[ii].data_type2);
    if (out.put(_params.decoded_output_url,
		SPDB_SIGAIRMET_ID, SPDB_SIGAIRMET_LABEL)) {
      cerr << "ERROR - Taf2Spdb::_overwriteCancelled" << endl;
      cerr << out.getErrStr() << endl;
    }

  } // ii

}

////////////////////////////////////////////////
// set text from unused tokens
//
// If useMessageString is false, only take text segments which are 4 or more
// tokens long otherwise use the entire tafStr.

void Taf2Spdb::_setText(const string &tafStr,
                        const bool useTafString)
  
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setText()" << endl;
  }

  if (useTafString) {
    _decoded.setText(tafStr);
    return;
  }

  else {
    size_t len = 0;
    string text;
    string subText;
  
    for (size_t ii = _startTokNum + 2; ii < _used.size(); ii++) {
      if (!_used[ii] && _toks[ii].find("END/", 0) == string::npos) {
	subText += _toks[ii];
	subText += " ";
	len++;
      } else {
	if (len >= 4) {
	  text += subText;
	}
	len = 0;
	subText = "";
      }
    }
    if (len >= 4) {
      text += subText;
    }

    _decoded.setText(text);
  }
}

////////////////////////////////////////////////
// handle a reissue. Get the ID of the
// sig/airmet to be reissued.
//

void Taf2Spdb::_setReissue()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setReissue()" << endl;
  }

  string reissueId, reissueQualifier;
  if (_isReissueMessage(_startTokNum, _toks,
			reissueId, reissueQualifier)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "This is an reissue: ID: " << reissueId;
      if (reissueQualifier != "NULL") {
	cerr << ", qualifier: " << reissueQualifier;
      }
      cerr << endl;
    }
  }
}

#endif

#ifdef JUNK

// prototypes


  // Functions to Set() various fields in the object

  int _setId();
  int _setWx();
  int _getWxType(int startPos, string &wxType);
  void _setAmend();
  void _setReissue();
  void _setSource();
  int _setTimes();    
  void _setText(const string &tafStr, const bool useTafString);

  // Functions to handle CANCEL
  
  void _overwriteCancelled();

  // Functions to handle NIL message/report
  
  bool _isNil();
  
  // Functions to handle time -- in Utils.cc

  int _computeIssueTime(const string &timeTok, time_t &issueTime);
  int _findStartEndTimes(time_t &startTime, time_t &endTime);
  int _computeStartEndTimes(const string &timeTok,
                            time_t &startTime, time_t &endTime);
  void _correctTimeForMonthChange(time_t &reportTime);
  void _correctTimeForFileTime(time_t &computedTime);

  // Functions to test whether a token is a specific type -- in Utils.cc
  
  bool _allAlpha(const string &tok);
  bool _allDigits(const string &tok);
  bool _hasDigit(const string &tok);
  bool _isStationBogus(const string &tok);
  bool _isWxTok(const string &tok);
  bool _isNil(const size_t startIdx,
	      const vector <string> toks);
  bool _isStateAbbr(const string &tok,
		    const string &prevTok,
		    const string &nextTok);
  bool _isTimeTok(const string &tok, time_t &isTime);
  int _isSpeedTok(const string &tok);
  bool _isMovementTok(const vector <string> &toks, 
		      const size_t &moveIdx,
		      vector <size_t> &moveToks,
		      string &moveType,
		      string &moveDir,
		      string &moveSpeed);
  bool _isFcastOutlookWxType();
  bool _isSinglePointWxType();
  bool _isIdTok(const string &tok,
		string &strippedTok);
  bool _isLocationIndicator(const size_t &startIdx,
			    const size_t &endIdx,
			    const vector <string> &toks,
			    size_t &idx);

  // Functions for specific message types -- in Utils.cc

  bool _isTestMessage(size_t &startIdx,
		      const vector <string> &toks,
		      const int &minNumTest);
  bool _isAmendMessage(size_t &startIdx,
		       const vector <string> &toks,
		       string &id,
		       string &qualifier);
  void _checkCancel(const size_t &startIdx,
                    const vector <string> toks);
  bool _isReissueMessage(size_t &startIdx,
			 const vector <string> &toks,
			 string &id,
			 string &qualifier);
  bool _isSeeAlsoMessage(size_t &startIdx,
			 const vector <string> &toks,
			 string &id,
			 string &qualifier);

  // Functions to find ID and qualifier -- in Utils.cc

  bool _findIdAndQualifier(const size_t &startIdx,
			   const vector <string> &toks,
			   string &id,
			   string &qualifier,
			   bool &foundId,
			   bool &foundQualifier,
			   vector <size_t> &usedIdxs);

  int _mergeIdAndQualifier(string &id,
                           const string &qualifier);

  // Functions to do conversions -- in Utils.cc

  bool _speed2Kph(const string &speed, double &speed_kph);
  
  // Functions to find specific matches -- in Utils.cc
  
  bool _findFrom(const size_t &startIdx,
		 const size_t &endIdx,
		 const vector <string> &toks,
		 size_t &fromIdx);
  bool _findMatchInToks(const string &tok,
			const size_t &startIdx,
			const size_t &endIdx,
			const vector <string> &toks,
			size_t &idxInToks);
  int _findStringNTimes(const string &tok,
			const string &wantstr);

  // Functions to do string manipulation -- in Utils.cc

  void _stripLeadingSpacer(const string &spacer,
			   const string &instr,
			   string &outstr);
  void _stripTrailingSpacer(const string &spacer,
			    const string &instr,
			    string &outstr);
  void _chopSpacer2End(const string &spacer,
		       const string &instr,
		       string &outstr);
  void _stripTermChars(const string &instr, string &outstr);
  bool _lastString(const string &tok, const string &wantstr);
  bool _firstString(const string &tok, const string &wantstr);
  int _isImbedDelimiter(size_t &startIdx, const vector <string> &toks,
			const string &delimiter);

  // Functions for (re)tokenizing -- in Utils.cc
  
  static void _tokenizeStr(const string &str, const string &spacer,
			   vector<string> &toks);

  bool _replaceSpacerRetokenize(const string &spacer1,
				const string &spacer2,
				const string &spacer3,
				size_t &startTok,
				vector<string> &intoks,
				vector<string> &toks);

  // Functions for stations -- in Points.cc

  bool _searchForStation(const bool &centroid_set,
			 const double &clat, const double &clon,
			 const string &station,
			 double &lat, double &lon, double &alt);
  bool _modifyStationPosition(const string &modloc,
			      double &inlat, double &inlon,
			      double &outlat, double &outlon);
  bool _searchForStationString(size_t &startIdx, vector<string> &toks,
			       double &lat, double &lon, size_t &endIdx);

#endif

  
