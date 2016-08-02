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
 // SigAirMet2Spdb object -- utilities functions
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
 #include "SigAirMet2Spdb.hh"

/////////////////////////////////////////////////
// Look for imbedded delimiter
// Return the number found.

int SigAirMet2Spdb::_isImbedDelimiter(size_t &startIdx,
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

bool SigAirMet2Spdb::_replaceSpacerRetokenize(const string &spacer1,
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

int SigAirMet2Spdb::_computeIssueTime(const string &timeTok,
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

int SigAirMet2Spdb::_findStartEndTimes(time_t &startTime,
                                       time_t &endTime)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setStartEndTimes()" << endl;
  }
  
  // set the start and end times - take the first set which decodes
  // set the defaults to the file time
  
  // look for "VALID .... "
  
  for (int ii = _startTokNum + 1; ii < (int) _msgToks.size() - 1; ii++) {

    if (_msgToks[ii] == "VALID") {

      // VALID start_time/end_time
      
      if (_computeStartEndTimes(_msgToks[ii+1], startTime, endTime) == 0) {
        _used[ii] = true;
        _used[ii+1] = true;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Valid times: found start and end times: "
               << _msgToks[ii] << " "
               << _msgToks[ii+1] << endl;
        }
        return 0;
      }

      // VALID UNTIL end_time?
      
      if (ii < (int) _msgToks.size() - 2) {
        if (_msgToks[ii+1] == "UNTIL" &&
            _isTimeTok(_msgToks[ii+2], endTime)) {
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
                 << _msgToks[ii] << " "
                 << _msgToks[ii+1] << " " 
                 << _msgToks[ii+2] << endl;
          }
          return 0;
        }
      }

      // VALID start_time end time
        
      if (ii < (int) _msgToks.size() - 2) {
        if (_isTimeTok(_msgToks[ii+1], startTime) &&
            _isTimeTok(_msgToks[ii+2], endTime)) {
          _used[ii] = true;
          _used[ii+1] = true;
          _used[ii+2] = true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "Valid times: found start and end times: "
                 << _msgToks[ii] << " "
                 << _msgToks[ii+1] << " " 
                 << _msgToks[ii+2] << endl;
          }
          return 0;
        }
      }

      // VALID start_time - end time
        
      if (ii < (int) _msgToks.size() - 3) {
        if (_isTimeTok(_msgToks[ii+1], startTime) &&
            _isTimeTok(_msgToks[ii+3], endTime)) {
          _used[ii] = true;
          _used[ii+1] = true;
          _used[ii+2] = true;
          _used[ii+3] = true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "Valid times: found start and end times: "
                 << _msgToks[ii] << " "
                 << _msgToks[ii+1] << " " 
                 << _msgToks[ii+2] << " " 
                 << _msgToks[ii+3] << endl;
          }
          return 0;
        }
      }

    } // if (_msgToks[ii] == "VALID")

  } // ii

  return -1;

}

//////////////////////////////////////////////////////////
// compute start and end times from the file time and a
// time token string

int SigAirMet2Spdb::_computeStartEndTimes(const string &timeTok,
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

void SigAirMet2Spdb::_correctTimeForMonthChange(time_t &reportTime)
  
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

void SigAirMet2Spdb::_correctTimeForFileTime(time_t &computedTime)
  
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
// find a width in the input tokens
//
// Return true if a width found, false otherwise

bool SigAirMet2Spdb::_findWidth(const size_t &startIdx,
				const size_t &endIdx,
				const vector <string> &toks,
				int &width,
				size_t &usedStartIdx, 
				size_t &usedEndIdx)
{

  // set defaults

  usedStartIdx=0;
  usedEndIdx=0;
  width=0;

  // find width

  bool found = false;
  for (size_t ii = startIdx; ii < endIdx; ii++) {

    // e.g., nnNM

    if (toks[ii].find("NM", 0) != string::npos) {
      if (sscanf(toks[ii].c_str(), "%dNM", &width) == 1) {
	found = true;
	usedStartIdx= ii;
	break;
      }
    }

    // e.g., nn NM

    if ((toks[ii] == "NM") && (ii-1 >= startIdx)) {

      string prevTok=toks[ii-1];

      // Need to be careful about finding NM -- the state!
      
      if ((_allDigits(prevTok)) && (sscanf(prevTok.c_str(), "%d", &width) == 1)) {
	found = true;
	usedStartIdx= ii - 1;
	usedEndIdx=ii;
	break;
      }
    }

    // e.g., Dnn

    if (_isDiameterTok(toks[ii], width)) {
      found = true;
      usedStartIdx= ii;
      break;
    }
  }

  return found;
}


///////////////////////////////////////////////////
// find a FROM in the input tokens, this is used for a start point
// for line points
//
// Return true if a from found, false otherwise

bool SigAirMet2Spdb::_findFrom(const size_t &startIdx,
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

bool SigAirMet2Spdb::_isLocationIndicator(const size_t &startIdx,
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

bool SigAirMet2Spdb::_findMatchInToks(const string &tok,
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

bool SigAirMet2Spdb::_allAlpha(const string &tok)

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

bool SigAirMet2Spdb::_allDigits(const string &tok)

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

bool SigAirMet2Spdb::_hasDigit(const string &tok)

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

bool SigAirMet2Spdb::_isStationBogus(const string &tok)

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

bool SigAirMet2Spdb::_isNil(const size_t startIdx,
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

bool SigAirMet2Spdb::_isWxTok(const string &tok)

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

bool SigAirMet2Spdb::_isIdTok(const string &tok,
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

////////////////////////////////////////////////////
// check if the token is a possible state abbreviation,
// need to see the tokens on both sides as well
//
// Return true if a possible state abbreviation token, false if not

bool SigAirMet2Spdb::_isStateAbbr(const string &tok,
				  const string &prevTok,
				  const string &nextTok)

{
  if (_hasDigit(tok)) {
    return false;
  }

  if (!_allAlpha(tok)) {
    return false;
  }

  int nmatches=0;
  if (_readStates.IsStateAbbr(tok)) {
    nmatches++;
  } else {
    return false;
  }

  if (_readStates.IsStateAbbr(prevTok)) {
    nmatches++;
  }

  if (_readStates.IsStateAbbr(nextTok)) {
    nmatches++;
  }

  // If we get a perfect score (3) return true

  if (nmatches == 3) {
    return true;
  }

  // Not sure what to do if we get a 2 out of 3 score
  // For now, return true

  if (nmatches == 2) {
    return true;
  }

  // With US SIGMETs, we can get a single state that occurs right 
  // after the valid time

  time_t isTime;
  if ((nmatches == 1) && (_isTimeTok(prevTok, isTime))) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////
// check if the token is a possible LINE token
//
// Return true if a possible LINE token, false otherwise

bool SigAirMet2Spdb::_isLineTok(const string &tok)

{
  bool found=false;
  if ((tok == "LINE") || (tok == "LIN") || (tok == "LN") || (tok == "BAND")) {
    found=true;
  }
  return found;
}

////////////////////////////////////////////////
// check if the token is a time string. This does not
// work for the ICAO standard valid times which do not
// have a Z in them.
// 
// Return true if a time, false if not. Also return in the
// desired DateTime format

bool SigAirMet2Spdb::_isTimeTok(const string &tok, time_t &isTime)

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
// check if the token is a flight level token
//
// Return 1 if tok is a FL token, 0 if not. Return 2 if tok+tok2 is FL token
//

int SigAirMet2Spdb::_isFLTok(const string &tok, const string &tok2)

{
  size_t keyPos=tok.find("FL", 0);
  if ((keyPos != string::npos) && (_hasDigit(tok))) {
    if (keyPos < tok.size()) {
      if (isdigit(tok[keyPos+2])) {
	return 1;
      }
    }
  }

  if ((_lastString(tok, "M")) && (_hasDigit(tok))) {
    return 1;
  }
  if ((_lastString(tok, "FT")) && (_hasDigit(tok))) {
    return 1;
  }
  if ((tok == "FL" ||
       tok == "ABV" ||
       tok == "BLW" ||
       tok == "BLO" ||
       tok == "BOT" ||
       tok == "BTN" ||
       tok == "GND" ) && (_hasDigit(tok2))) {
    return 2;
  }
  return 0;
}

////////////////////////////////////////////////////
// check if the token contains a N/S/E/W direction
//
// Return true if contains a direction, false if not
//

bool SigAirMet2Spdb::_isDirTok(const string &tok)

{
  if (_allDigits(tok)) {
    return false;
  }

  string dir;
  size_t dirLen = 0;
  size_t dirPos = 0;
  bool foundDir=false;
  for (size_t ii=0; ii<_directions.size(); ii++) {
    dirPos=tok.find(_directions[ii], 0);
    if (dirPos != string::npos) {
      foundDir=true;
      dir=_directions[ii];
      dirLen=dir.length();
      break;
    }
  }
  if (!foundDir) {
    return false;
  }

  // Are there other things in the token besides a direction character?
  // Do not want to find SBAT for S. Remove the direction from the token
  // then check the remaining characters

  string tmpTok=tok;
  tmpTok.erase(dirPos, dirLen);

  if (_allDigits(tmpTok)) {
    return true;
  }

  if (_allAlpha(tmpTok)) {
    return false;
  }

  if (tmpTok.length() <= 0) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////
// skipUSHeader
//
// return true if a possible US report; also return the first
// token to start parsing for locations and weather (skip the
// header)
//

bool SigAirMet2Spdb::_skipUSHeader(const vector <string> toks,
				   const size_t useStartIdx,
				   const size_t useEndIdx,
				   size_t &wantStartIdx) 
{

  // Set defaults

  wantStartIdx=useStartIdx;

  // Loop through the toks array to find the end of the header

  bool may_be_US=false;
  bool found_start=false;

  for (size_t ii=useStartIdx; ii<useEndIdx; ii++) {

    string tok=toks[ii];
    
    // Need to be careful about finding VA -- the state!!

    bool isState=false;
    if ((ii-1 >= 0) && (ii+1 <toks.size())) {
      string prevTok=toks[ii-1];
      string nextTok=toks[ii+1];
      isState=_isStateAbbr(tok, prevTok, nextTok);
    }
    if (isState) {
      may_be_US=true;
    }

    // If we are in the US and we have NOT hit the word FROM, we
    // are still in the header
	   
    if ((may_be_US) && (tok == "FROM")) {
      found_start=true;
      wantStartIdx=ii;
      break;
    }
  }

  return found_start;
}



////////////////////////////////////////////////////
// convert a direction to an angle.
// force the direction into an angle such that the angle
// is positive if east of North, negative (or > PI) if west of North,
// 0 = North
//
// Return true if could do the convert, false if not
//

bool SigAirMet2Spdb::_dir2Angle(const string &dir, double &angle)

{
  if (_allDigits(dir)) {
    return false;
  }

  angle=-9999;

  if (dir == "N") {
    angle=0;
  } else if (dir == "NNE") {
    angle=22.5;
  } else if (dir == "NE") {
    angle=45;
  } else if (dir == "ENE") {
    angle=77.5;
  } else if (dir == "E") {
    angle=90;
  } else if (dir == "ESE") {
    angle=112.5;
  } else if (dir == "SE") {
    angle=135;
  } else if (dir == "SSE") {
    angle=177.5;
  } else if (dir == "S") {
    angle=180;
  } else if (dir == "SSW") {
    angle=-177.5;
  } else if (dir == "SW") {
    angle=-135;
  } else if (dir == "WSW") {
    angle=-112.5;
  } else if (dir == "W") {
    angle=-90;
  } else if (dir == "WNW") {
    angle=-77.5;
  } else if (dir == "NW") {
    angle=-45;
  } else if (dir == "NNW") {
    angle=-22.5;
  } else {
    return false;
  }
  
  if (angle != -9999) {
    return true;
  }
  return false;
}

////////////////////////////////////////////////////
// convert a direction to the REVERSE angle. This is 
// needed to EXCLUDE the direction.
//
// force the direction into an angle such that the angle
// is positive if east of North, negative (or > PI) if west of North,
// 0 = North.
//
// Return true if could do the convert, false if not
//

bool SigAirMet2Spdb::_dir2ReverseAngle(const string &dir, double &angle)

{
  if (_allDigits(dir)) {
    return false;
  }

  angle=-9999;

  if (dir == "N") {
    angle=180;
  } else if (dir == "NNE") {
    angle=-177.5;
  } else if (dir == "NE") {
    angle=-135;
  } else if (dir == "ENE") {
    angle=-112.5;
  } else if (dir == "E") {
    angle=-90;
  } else if (dir == "ESE") {
    angle=-77.5;
  } else if (dir == "SE") {
    angle=-45;
  } else if (dir == "SSE") {
    angle=-22.5;
  } else if (dir == "S") {
    angle=0;
  } else if (dir == "SSW") {
    angle=22.5;
  } else if (dir == "SW") {
    angle=45;
  } else if (dir == "WSW") {
    angle=77.5;
  } else if (dir == "W") {
    angle=90;
  } else if (dir == "WNW") {
    angle=112.5;
  } else if (dir == "NW") {
    angle=135;
  } else if (dir == "NNW") {
    angle=177.5;
  } else {
    return false;
  }
  
  if (angle != -9999) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////
// convert a speed to km/hr.
//
// Return true if can do the conversion, false if not
//

bool SigAirMet2Spdb::_speed2Kph(const string &speed, double &speed_kph)

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
  unitStrings.push_back("MPS");


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

      // If this is MPS, do conversion
      
      if (unitStrings[ii].find("MPS", 0) != string::npos) {
	speed_kph=(double)speedInt * MPERSEC_TO_KMPERHOUR;
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
// check if the token contains a diameter
//
// Return true if contains a diameter, false if not
//

bool SigAirMet2Spdb::_isDiameterTok(const string &tok, int &diam)

{
  diam=0;
  bool found=false;

  if (_allDigits(tok)) {
    return false;
  }
  if (_allAlpha(tok)) {
    return false;
  }

  if ((_firstString(tok, "D")) && (_hasDigit(tok))) {
    if (sscanf(tok.c_str(), "D%d", &diam) == 1) {
      found=true;
    }
  }
  
  return found;
}


////////////////////////////////////////////////////
// check if the token contains a speed
//
// Return 1 if contains a speed and units, 0 if no units,
// 2 if contains units and no speed
//

int SigAirMet2Spdb::_isSpeedTok(const string &tok)

{
  if (_allDigits(tok)) {
    return 0;
  }

  bool found=false;
  if ((tok.find("KMH", 0) != string::npos) || 
      (tok.find("KT", 0) != string::npos) ||
      (tok.find("MPS", 0) != string::npos) ||
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
// check if the token contains an intensity
//
// Return true if contains an intensity, false if not
//

bool SigAirMet2Spdb::_isIntensityTok(const string &tok)

{
  if (_allDigits(tok)) {
    return false;
  }
  if (tok == "INTSF" || tok == "WKN" || tok == "NC") {
    return true;
  }
  return false;
}

////////////////////////////////////////////////////
// check if the token(s) contain a movement
//
// Return true if contains a movement, false if not
//

bool SigAirMet2Spdb::_isMovementTok(const vector <string> &toks, 
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

    if ((dirTok < toks.size()) && (_isDirTok(toks[dirTok]))) {
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
  if (unitPos == string::npos) {
    unitPos=moveSpeed.find("MPS", 0);
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


////////////////////////////////////////////////////
// check if the token contains stationary movement
//
// Return true if contains stationary movement, false if not
//

bool SigAirMet2Spdb::_isStatMovTok(const string &tok, const string &tok2,
				   bool &useTok2)

{
  // Set defaults

  useTok2=false;

  if (_allDigits(tok)) {
    return false;
  }
  if (tok == "STNR") {
    return true;
  }
  if (tok2 != "NULL") {
    string tmpstr=tok;
    tmpstr += tok2;
    if (tmpstr == "STNR") {
      useTok2=true;
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////
// check if the Weather type is one with forecast and
// outlook positions. According to the ICAO standard,
// this is TC and VA only.
//
// Return true if weather type with forecast and outlook
// positions, false otherwise
//

bool SigAirMet2Spdb::_isFcastOutlookWxType()

{
   // Is this a TC or VA? These are the only Wx types with fcast positions

   bool isWantedWx=false;
   string wxType=_decoded.getWx();
   if ((wxType.find("TC", 0) != string::npos) ||
       (wxType.find("VA", 0) != string::npos)) {
     isWantedWx=true;
   }

   return isWantedWx;
}


////////////////////////////////////////////////////
// check if the Weather type is one with a possible
// single center point. Currently this is TC only.
//
// Return true if weather type with a possible
// single center point, false otherwise
//

bool SigAirMet2Spdb::_isSinglePointWxType()

{
   // Is this a TC?

   bool isWantedWx=false;
   string wxType=_decoded.getWx();
   if (wxType.find("TC", 0) != string::npos) {
     isWantedWx=true;
   }

   return isWantedWx;
}

////////////////////////////////////////////////////
// find N/S and E/W characters in the input string.
//
// Return true if found reasonable latlon type characters, return
// false otherwise. Will return true if find either a NS or EW
// character
//

bool SigAirMet2Spdb::_findNSEWchars(const string &tok,
				    bool &foundNS,
				    size_t &nsPos,
				    bool &foundEW,
				    size_t &ewPos)

{
  // set defaults

  foundNS=false;
  foundEW=false;

  // look for N/S and E/W characters. Bail if we find something
  // other than N/S/E/W.
  
  int nsNfound=0;
  int ewNfound=0;
  int nDots=0;
  for (size_t ii = 0; ii < tok.size(); ii++) {
    if (isalpha(tok[ii])) {
      if (tok[ii] == 'N' || tok[ii] == 'S') {
	nsPos = ii;
	nsNfound++;
      }
      else if (tok[ii] == 'E' || tok[ii] == 'W') {
	ewPos = ii;
	ewNfound++;
      }
      else if (tok[ii] == '.') {
	nDots++;
      }
      else {
	return false;
      }
    }
  }

  if ((nsNfound > 1) || (ewNfound > 1)) {
    return false;
  }

  if (nsNfound == 1) {
    foundNS=true;
  }

  if (ewNfound == 1) {
    foundEW=true;
  }

  if (foundNS || foundEW) {
    return true;
  }

  return false;
}



////////////////////////////////////////////////////
// check if the vector contains a FIR
//
// Return true if find the keystring FIR, false if not.
//

bool SigAirMet2Spdb::_isFirTok(const string &tok)

{
  // Set defaults

   bool found=false;
 
   // Search for the FIR string itself

   if (tok == "FIR" ||
       tok == "UIR" ||
       tok == "FIR/UIR" ||
       tok == "CTA" ||
       tok == "FIR:" ||
       tok == "FIR.") {
     found=true;
   }
   if (!found) {
     if ((tok.find("-FIR", 0) != string::npos) && (!_hasDigit(tok))) {
       found=true;
     }
   }
   if (!found) {
     if (_lastString(tok, "FIR") && (!_hasDigit(tok))) {
       found=true;
     }
   }
   return found;

}

/////////////////////////////////////////////////
// find min and max lat and lon
// 
// return True if able to find min/max, false otherwise

bool SigAirMet2Spdb::_getMinMaxLatLon(const vector <double> &lats,
				      const vector <double> &lons,
				      double &minLat,
				      double &maxLat,
				      double &minLon,
				      double &maxLon)

{
  // set defaults

  bool found=false;
  minLat=lats[0];
  maxLat=minLat;
  minLon=lons[0];
  maxLon=minLon;
  string func_name="getMinMaxLatLon";

  for (size_t ii=1; ii< lats.size(); ii++) {
    if (lats[ii] < minLat) {
      minLat=lats[ii];
    }
    if (lats[ii] > maxLat) {
      maxLat=lats[ii];
    }
  }

  for (size_t ii=1; ii< lons.size(); ii++) {
    if (lons[ii] < minLon) {
      minLon=lons[ii];
    }
    if (lons[ii] > maxLon) {
      maxLon=lons[ii];
    }
  }

  if ((minLat != maxLat) && (minLon != maxLon)) {
    found=true;
  }

  return found;
}
 
////////////////////////////////////////////////////
// check if input vector, within limits, contains a FIR
//
// Return true if contains a FIR can find, false if not.
// Also returns the vector of indices into toks[] that
// defined the FIR and the name of the FIR (not including
// the FIR string itself).
//
// This should be used AFTER a call to _isFirTok() to
// retrieve the input firIdx for this function. These were
// intentionally split for use in _findPoints().
//

bool SigAirMet2Spdb::_isFirName(const vector <string> &toks, 
				size_t &firIdx,
				vector <size_t> &firToks,
				string &firName)

{
  // Set defaults

   bool found=false;
   firToks.clear();
   firName="UNKNOWN";

   // FIR locations usually appear before or after the FIR/UIR/CTA string
   // Some FIRs are multiple words. Some FIRs occur before a :

   // Try the word before the FIR

   if ((!found) && (firIdx-1 >=0)) {
     firName=toks[firIdx - 1];
     if (_firLoc.FirExists(firName)) {
       found=true;
       firToks.push_back(firIdx-1);
       firToks.push_back(firIdx);
     }
   }
   
   // Try the word after the FIR

   if ((!found) && (firIdx+1 < toks.size())) {
     firName=toks[firIdx + 1];
     if (_firLoc.FirExists(firName)) {
       found=true;
       firToks.push_back(firIdx);
       firToks.push_back(firIdx+1);
     }
   }

   // Try the 2 words before the FIR

   if ((!found) && (firIdx-2 >= 0)) {
     firName=toks[firIdx - 2];
     firName += " ";
     firName += toks[firIdx - 1];
     if (_firLoc.FirExists(firName)) {
       found=true;
       firToks.push_back(firIdx-2);
       firToks.push_back(firIdx-1);
       firToks.push_back(firIdx);
     }
   }

   // Try the 2 words after the FIR

   if ((!found) && (firIdx+2 < toks.size())) {
     firName=toks[firIdx + 1];
     firName += " ";
     firName += toks[firIdx + 2];
     if (_firLoc.FirExists(firName)) {
       found=true;
       firToks.push_back(firIdx);
       firToks.push_back(firIdx+1);
       firToks.push_back(firIdx+2);
     }
   }

   // Try the name and the FIR in one token
   // eg. NTAA-FIR

   if ((!found) && (toks[firIdx].find("-FIR", 0) != string::npos) &&
       (!_hasDigit(toks[firIdx]))) {
     firName=toks[firIdx];
     _chopSpacer2End("-", firName, firName);
     if (_firLoc.FirExists(firName)) {
       found=true;
       firToks.push_back(firIdx);
     }
   }

   // Try the previous token, but split it on a dash (-)
   // eg. WSSS-SINGAPORE FIR

   if ((!found) && (firIdx-1 >=0)) {
     firName=toks[firIdx - 1];
     if (_findStringNTimes(firName, "-") > 0) {
       vector <string> tmpToks;
       _tokenize(firName, "-", tmpToks);

       for (size_t ii=0; ii<tmpToks.size(); ii++) {
	 firName=tmpToks[ii];
	 if (_firLoc.FirExists(firName)) {
	   found=true;
	   firToks.push_back(firIdx);
	   break;
	 }
       }
     }
   }

   // Try the previous 2 tokens, but split it on a dash (-)
   // eg. SLLF-LA PAZ FIR

   if ((!found) && (firIdx-2 >=0)) {
     firName=toks[firIdx - 2];
     firName += " ";
     firName += toks[firIdx -1];
     if (_findStringNTimes(firName, "-") > 0) {
       vector <string> tmpToks;
       _tokenize(firName, "-", tmpToks);

       for (size_t ii=0; ii<tmpToks.size(); ii++) {
	 firName=tmpToks[ii];
	 if (_firLoc.FirExists(firName)) {
	   found=true;
	   firToks.push_back(firIdx);
	   firToks.push_back(firIdx-1);
	   firToks.push_back(firIdx-2);
	   break;
	 }
       }
     }
   }

   if (!found) {
     firToks.clear();
     firName="UNKNOWN";
   }

   return found;
}

/////////////////////////////////////////////////////
// Find the ID and qualifier
//
// Return true if found, false otherwise
// 
//
bool SigAirMet2Spdb::_findIdAndQualifier(const size_t &startIdx,
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

int SigAirMet2Spdb::_mergeIdAndQualifier(string &id,
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


////////////////////////////////////////////////////
// Look for flight levels within the input vector
//
// Return the top and bottom flight level in bot and top
//
// Return 1 if a well-defined top and bottom span
// Return 0 if a not-so-well-defined span
// Return -1 if nothing found
//

int SigAirMet2Spdb::_findFlightLevels(const vector <string> &toks, 
				      const vector <bool> &usedToks,
				      const size_t &startIdx,
				      const size_t &endIdx,
				      bool &foundTop,
				      bool &foundBot,
				      int &bot,
				      int &top,
				      vector <size_t> &usedIdxs)
{

  // Set defaults
  
  usedIdxs.clear();
  foundBot = false;
  foundTop = false;

  int wellDefined=1;
  int notWellDefined=0;
  int noneFound=-1;

  // First try for well-defined span...
  //
  // try for FL320-FL400 style, with variations

  for (size_t ii = startIdx ; ii < endIdx; ii++) {
    const string &tok = toks[ii];
    if (tok.size() >= 11) {
      if (sscanf(tok.c_str(), "FL%3d-FL%3d", &bot, &top) == 2) {
	usedIdxs.push_back(ii);
	return wellDefined;
      }
    }

    if (tok.size() >= 9) {
      if (sscanf(tok.c_str(), "FL%3d-%3d", &bot, &top) == 2) {
	usedIdxs.push_back(ii);
	return wellDefined;
      }
    }

    if (tok.size() >= 11) {
      if (sscanf(tok.c_str(), "FL%3d/FL%3d", &bot, &top) == 2) {
	usedIdxs.push_back(ii);
	return wellDefined;
      }
    }

    if (tok.size() >= 9) {
      if (sscanf(tok.c_str(), "FL%3d/%3d", &bot, &top) == 2) {
	usedIdxs.push_back(ii);
	return wellDefined;
      }
    }

    if (tok.size() >= 7) {
      if (sscanf(tok.c_str(), "%3d-%3d", &bot, &top) == 2) {
	usedIdxs.push_back(ii);
	return wellDefined;
      }
    }

    if (tok.size() >= 9) {
      if (sscanf(tok.c_str(), "SFC/FL%3d", &top) == 1) {
	bot = 0;
	usedIdxs.push_back(ii);
	return wellDefined;
      }
    }

  } // ii

  // Did not find a well-defined span, now try for other styles

  int level1 = -1, level2 = -1;
  int level;

  // Loop through the tokens again looking for other styles

  for (size_t ii = startIdx; ii < endIdx; ii++) {

    // skip used

    if (!usedToks[ii]) {

      // Set the next and previous token for searching

      string tok = toks[ii];

      bool hasPrevTok=false;
      bool hasNextTok=false;
      bool hasNextNextTok=false;
      string nextTok="NULL";
      string prevTok="NULL";
      string nextNextTok="NULL";
      if (ii+1 < endIdx) {
	nextTok = toks[ii+1];
	hasNextTok=true;
      }
      if (ii+2 < endIdx) {
	nextNextTok = toks[ii+2];
	hasNextNextTok=true;
      }
      if (ii-1 >= startIdx) {
	prevTok = toks[ii-1];
	hasPrevTok=true;
      }

      // try for FL in one token and the span in the next token
      // e.g., FL 320-440 style

      if (tok == "FL" && hasNextTok) {
	// int bot, top;
	if (sscanf(nextTok.c_str(), "%3d-%3d", &bot, &top) == 2) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	  return wellDefined;
	}

	if (sscanf(nextTok.c_str(), "%3d/%3d", &bot, &top) == 2) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	  return wellDefined;
	}

	if (sscanf(nextTok.c_str(), "%3d/%3d", &bot, &top) == 2) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	  return wellDefined;
	}

	// try for FL 300 /400 style

	if (ii < endIdx -2) {
	  string nextNextTok = _msgToks[ii+2];
	  if ((sscanf(nextTok.c_str(), "%3d", &bot) == 1) &&
	      (sscanf(nextNextTok.c_str(), "/%3d", &top) == 1)) {
	    usedIdxs.push_back(ii);
	    usedIdxs.push_back(ii+1);
	    usedIdxs.push_back(ii+2);
	    return wellDefined;
	  }
	}
      } //endif FL and hasNextTok

      
      // Look for keyword strings to indicate whether flight levels
      // found might be a top or bottom

      if (tok.find("TOP", 0) != string::npos) {
	foundTop = true;
	usedIdxs.push_back(ii);
      }
      if (tok.find("BELOW", 0) != string::npos) {
	foundTop = true;
	usedIdxs.push_back(ii);
      }
      if (tok.find("BLW", 0) != string::npos) {
	foundTop = true;
	usedIdxs.push_back(ii);
      }
      if (tok.find("BLO", 0) != string::npos) {
	foundTop = true;
	usedIdxs.push_back(ii);
      }
      if (tok.find("BOT", 0) != string::npos) {
	foundBot = true;
	usedIdxs.push_back(ii);
      }
      if (tok.find("ABV", 0) != string::npos) {
	foundBot = true;
	usedIdxs.push_back(ii);
      }
      if (tok.find("BTN", 0) != string::npos) {
	foundBot = true;
	foundTop = true;
      }
      if (tok.find("GND", 0) != string::npos) {
	foundBot = true;
	if (level1 < 0) {
	  level1 = 0;
	}
      }

      if ((tok.find("TOP", 0) != string::npos) && hasNextTok) {
	if (nextTok == "ABV") {
	  foundTop=false;
	  foundBot=true;
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	}
      }

      // try for FL320 style

      if (sscanf(tok.c_str(), "FL%3d", &level) == 1) {
	usedIdxs.push_back(ii);
	if (level1 < 0) {
	  level1 = level;
	} else if (level2 < 0) {
	  level2 = level;
	} else {
	  break;
	}
      }

      // try for FL 320 style

      if (tok == "FL" && ii < hasNextTok) {
	if (sscanf(nextTok.c_str(), "%3d", &level) == 1) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	  if (level1 < 0) {
	    level1 = level;
	  } else if (level2 < 0) {
	    level2 = level;
	  } else {
	    break;
	  }
	}
      }

      // try for 5000 FT style

      if (tok == "FT" && hasPrevTok) {
	if (sscanf(prevTok.c_str(), "%d", &level) == 1) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii-1);
	  if (level1 < 0) {
	    level1 = level / 100;
	  } else if (level2 < 0) {
	    level2 = level / 100;
	  } else {
	    break;
	  }
	}
      }

      // try for 29000FT style

      if (_lastString(tok, "FT")) {
	if (sscanf(tok.c_str(), "%dFT", &level) == 1) {
	  usedIdxs.push_back(ii);
	  if (level1 < 0) {
	    level1 = level / 100;
	  } else if (level2 < 0) {
	    level2 = level / 100;
	  } else {
	    break;
	  }
	}
      }

      // try for A050 style. Need to force to 4 chars so do not find
      // an A2 id

      if ((tok.size() == 4) && (sscanf(tok.c_str(), "A%3d", &level) == 1)) {
	usedIdxs.push_back(ii);
	if (level1 < 0) {
	  level1 = level;
	} else if (level2 < 0) {
	  level2 = level;
	} else {
	  break;
	}
      }

      // try for BLW 080 or BLO 20 style

      if ((tok == "BLW") || (tok == "BLO")) {
	if (sscanf(nextTok.c_str(), "%d", &level) == 1) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	  if (level1 < 0) {
	    level1 = level;
	  } else if (level2 < 0) {
	    level2 = 0;
	  } else {
	    break;
	  }
	}
      }

      // try for GND/FL400 style

      if ((tok.find("GND", 0) != string::npos) && 
	  (tok.find("FL", 0) != string::npos)) {
	if (sscanf(tok.c_str(), "GND/FL%3d", &level) == 1) {
	  usedIdxs.push_back(ii);
	  if (level1 < 0) {
	    level1 = 0;
	  } else if (level2 < 0) {
	    level2 = level;
	  } else {
	    break;
	  }
	}
      }

      // try for 7000M style

      if (_lastString(tok, "M")) {
	if (sscanf(tok.c_str(), "%dM", &level) == 1) {
	  usedIdxs.push_back(ii);

	  // Convert meters to ft

	  double ft=(level/1000) * FT_PER_KM;
	  level=(int)ft;

	  if (level1 < 0) {
	    level1 = level / 100;
	  } else if (level2 < 0) {
	    level2 = level / 100;
	  } else {
	    break;
	  }
	}
      }

      // try for 5KM style

      if (_lastString(tok, "KM")) {
	if (sscanf(tok.c_str(), "%dKM", &level) == 1) {
	  usedIdxs.push_back(ii);

	   // Convert km to ft

	  double ft=level * FT_PER_KM;
	  level=(int)ft;

	  if (level1 < 0) {
	    level1 = level / 100;
	  } 
	  if (level2 < 0) {
	    level2 = level / 100;
	  }
	  else {
	    break;
	  }
	}
      }

      // try for 5/11 KM style

      if (tok == "KM" && hasPrevTok) {
	int lev1, lev2;
	if (sscanf(prevTok.c_str(), "%d/%d", &lev1, &lev2) == 2) {
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii-1);

	  // Convert km to ft

	  double km1=lev1 * FT_PER_KM;
	  double km2=lev2 * FT_PER_KM;

	  if (level1 < 0) {
	    level1 = (int) (km1 / 100);
	  } 
	  if (level2 < 0) {
	    level2 = (int) (km2 / 100);
	  }
	  else {
	    break;
	  }
	}
      }

      // try for TOP 350 style, also may get TOP 450 MAX 500
      // added check for TOP FL 480 - P.Prestopnik 

      if ((tok.find("TOP", 0) != string::npos) && hasNextTok) {
	if (sscanf(nextTok.c_str(), "%3d", &level) == 1){
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);

	  if ((level2 < 0) || (level2 < level)) {
	    level2 = level;
	  }	  
	}
	if ((nextTok == "FL") &&
	    (sscanf(nextNextTok.c_str(), "%3d", &level) == 1)){
	  usedIdxs.push_back(ii);
	  usedIdxs.push_back(ii+1);
	  usedIdxs.push_back(ii+2);

	  if ((level2 < 0) || (level2 < level)) {
	    level2 = level;
	  }	  	  	  
	}
      }
    } // endif (!_used[ii])
  } // ii

  // Setup return values

  // Best case, found a top and a bottom

  if (level1 > 0 && level2 > 0) {
    if (level1 > level2) {
      top=level1;
      bot=level2;
    } else {
      top=level2;
      bot=level1;
    }
    return wellDefined;
  }
    
  // Mixed case
  // Return -1 for the bottom if have a top but no bottom
  // Return -1 for the top if have a bottom but no top

  if (level1 >= 0 && level2 < 0) {
    if (foundTop && !foundBot) {
      top=level1;
      bot=level2;
    }
    else if (foundBot && !foundTop) {
      top=level2;
      bot=level1;
    } else {
      top=level1;
      bot=level2;
    }
    return notWellDefined;
  }

  if (level1 < 0 && level2 >= 0) {
    if (foundTop && !foundBot) {
      top=level2;
      bot=level1;
    }
    else if (foundBot && !foundTop) {
      top=level1;
      bot=level2;
    }
    else {
      top=level2;
      bot=level1;
    }
    return notWellDefined;
  }

  if (level1 >= 0 && level2 >= 0) {
     if (level2 > level1) {
       top=level2;
       bot=level1;
     } else {
       top=level1;
       bot=level2;
     }
     return notWellDefined;
  }

  // Could not find anything

  return noneFound;
}


////////////////////////////////////////////////
// strip leading spacer character(s)

void SigAirMet2Spdb::_stripLeadingSpacer(const string &spacer,
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

void SigAirMet2Spdb::_stripTrailingSpacer(const string &spacer,
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

void SigAirMet2Spdb::_chopSpacer2End(const string &spacer,
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

void SigAirMet2Spdb::_stripTermChars(const string &instr,
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

int SigAirMet2Spdb::_findStringNTimes(const string &tok,
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

bool SigAirMet2Spdb::_lastString(const string &tok,
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

bool SigAirMet2Spdb::_firstString(const string &tok,
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

bool SigAirMet2Spdb::_isTestMessage(size_t &startIdx,
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

bool SigAirMet2Spdb::_isAmendMessage(size_t &startIdx,
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

void SigAirMet2Spdb::_checkCancel(const size_t &startIdx,
                                  const vector <string> toks)

{

  _doCancel = false;
  _cancelGroup = _decoded.getGroup();
  _cancelId = _decoded.getId();

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
      _cancelGroup = SIGMET_GROUP;
    } else if (toks[cnlIdx+1] == "AIRMET") {
      _cancelGroup = AIRMET_GROUP;
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

bool SigAirMet2Spdb::_isReissueMessage(size_t &startIdx,
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

bool SigAirMet2Spdb::_isSeeAlsoMessage(size_t &startIdx,
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



/////////////////////////////////////////////////
// Convert lat,lon to x,y
//
//

void SigAirMet2Spdb::_latLon2XY(const double &lat, 
				const double &lon,
				double &x,
				double &y)

{
  // Set defaults

  double latOffset=180.0;
  double lonOffset=360.0;

  x = lat + latOffset;
  y = lon + lonOffset;
}

/////////////////////////////////////////////////
// Convert x,y to lat,lon
//
//

void SigAirMet2Spdb::_XY2LatLon(const double &x,
				const double &y,
				double &lat,
				double &lon)

{
  // Set defaults

  double latOffset=180.0;
  double lonOffset=360.0;

  lat = x - latOffset;
  lon = y - lonOffset;
}


//////////////////////////////////////////////
// tokenize a string into a vector of strings.
// use this version if you have a string spacer
// e.g. TO that could have characters in common
// with other strings. Use tokenize() if you have
// a unique character

void SigAirMet2Spdb::_tokenizeStr(const string &str,
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


//////////////////////////////////////////////
// tokenize a string into a vector of strings

void SigAirMet2Spdb::_tokenize(const string &str,
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
