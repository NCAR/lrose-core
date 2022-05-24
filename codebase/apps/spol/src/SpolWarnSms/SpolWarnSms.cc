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
// SpolWarnSms.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////
//
// SpolWarnSms reads status XML monitoring data from SPDB,
// for the S-band and K-band.
// It checks the status in the XML, and determines if
// a warning message should be sent
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <dsserver/DsLdataInfo.hh>
#include "SpolWarnSms.hh"

using namespace std;

// Constructor

SpolWarnSms::SpolWarnSms(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "SpolWarnSms";
  ucopyright(_progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // set up the warning periods

  if (_setupWarnPeriods()) {
    isOK = false;
    return;
  }
  _timeLastSms = 0;

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

}

// destructor

SpolWarnSms::~SpolWarnSms()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpolWarnSms::Run ()
{
  
  PMU_auto_register("Run");
  
  while (true) {
    
    PMU_auto_register("Getting status");
    time_t now = time(NULL);

    // read status, append to the message

    string warningMsgSband;
    if (_params.monitor_the_sband) {
      if (_readStatus(now,
                      _params.sband_monitoring_spdb_url,
                      _params.sband_spdb_margin_secs,
                      _params._sband_xml_entries,
                      _params.sband_xml_entries_n,
                      warningMsgSband)) {
        cerr << "ERROR - SpolWarnSms::Run" << endl;
        cerr << "  Problems monitoring the sband" << endl;
        warningMsgSband = "#Sband down#";
      }
    }

    string warningMsgKband;
    if (_params.monitor_the_kband) {
      if (_readStatus(now,
                      _params.kband_monitoring_spdb_url,
                      _params.kband_spdb_margin_secs,
                      _params._kband_xml_entries,
                      _params.kband_xml_entries_n,
                      warningMsgKband)) {
        cerr << "ERROR - SpolWarnSms::Run" << endl;
        cerr << "  Problems monitoring the kband" << endl;
        warningMsgKband = "#Kband down#";
      }
    }
    
    // combine messages

    string warningMsg;
    if (warningMsgSband.size() > 0 || warningMsgKband.size() > 0) {
      warningMsg += "SPOL:";
      warningMsg += DateTime::strm(now);
      warningMsg += warningMsgSband;
      warningMsg += warningMsgKband;
    }

    // write message to dir for SMS
    
    if (warningMsg.size() > 0) {
      if (_params.debug) {
        cerr << "================ warning message =====================" << endl;
        cerr << warningMsg << endl;
        cerr << "======================================================" << endl;
      }
      _writeMessageToDir(now, warningMsg);
    }

    // write to SPDB?

    if (_params.write_warnings_to_spdb) {
      if (warningMsg.size() > 0) {
        _writeMessageToSpdb(now, warningMsg);
      } else {
        string msg("No warnings");
        _writeMessageToSpdb(now, msg);
      }
    }
    
    // sleep
    
    umsleep(_params.monitoring_interval_secs * 1000);

  } // while
  
  return 0;
  
}

/////////////////////////////
// read the status
// update warning msg

int SpolWarnSms::_readStatus(time_t now,
                             char *spdbUrl,
                             int marginSecs,
                             Params::xml_entry_t *entries,
                             int nEntries,
                             string &warningMsg)

{
  
  if (_params.debug) {
    cerr << "==>> Checking status, time: " << DateTime::strm(now) << endl;
  }

  // read in status
  
  DsSpdb spdb;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdb.setDebug();
  }

  if (spdb.getLatest(spdbUrl, marginSecs, 0, 0)) {
    cerr << "ERROR - SpolWarnSms::_readStatus()" << endl;
    cerr << "  Calling getLatest for url: " << spdbUrl << endl;
    cerr << "  margin (secs): " << marginSecs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=======>> Got status" << endl;
    cerr << "url: " << spdbUrl << endl;
    cerr << "Prod label: " << spdb.getProdLabel() << endl;
    cerr << "Prod id:    " << spdb.getProdId() << endl;
    cerr << "N Chunks:   " << spdb.getNChunks() << endl;
  }
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  int nChunks = (int) chunks.size();
  if (nChunks < 1) {
    cerr << "ERROR - SpolWarnSms::_readStatus()" << endl;
    cerr << "  No chunks returned from SPDB" << endl;
    cerr << "  Calling getLatest for url: " << spdbUrl << endl;
    cerr << "  margin (secs): " << marginSecs << endl;
    return -1;
  }

  // set the xml string from the chunk data
  
  const Spdb::chunk_t &latest = chunks[chunks.size() - 1];
  time_t validTime = latest.valid_time;
  if (now - validTime > marginSecs) {
    cerr << "ERROR - SpolWarnSms::_readStatus()" << endl;
    cerr << "  Data too old" << endl;
    cerr << "  Calling getLatest for url: " << spdbUrl << endl;
    cerr << "  margin (secs): " << marginSecs << endl;
    cerr << "  now: " << DateTime::strm(now) << endl;
    cerr << "  status valid time: " << DateTime::strm(validTime) << endl;
    return -1;
  }

  string statusXml((const char *) latest.data);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "===>> time: " << DateTime::strm(validTime) << endl;
    cerr << "==================== STATUS XML =====================" << endl;
    cerr << statusXml;
    cerr << "=====================================================" << endl;
  }
  
  // check status, append to warning message as needed
  
  for (int ii = 0; ii < nEntries; ii++) {
    
    const Params::xml_entry_t &entry = entries[ii];
    
    switch (entry.entry_type) {
      
      case Params::XML_ENTRY_BOOLEAN:
        _handleBooleanEntry(now, statusXml, entry, warningMsg);
        break;
        
      case Params::XML_ENTRY_NUMBER:
        _handleNumberEntry(now, statusXml, entry, warningMsg);
        break;
        
    } // switch (entry.entry_type)
    
  } // ii

  return 0;

}

/////////////////////////////////
// write message to dir, for SMS

int SpolWarnSms::_writeMessageToDir(time_t now,
                                    const string &warningMsg)
  
{

  if (_params.debug) {
    cerr << "==>> writing warning message, now: " << DateTime::strm(now) << endl;
  }
  
  // compute dir path

  DateTime ntime(now);
  char outputDir[MAX_PATH_LEN];
  sprintf(outputDir, "%s%s%.4d%.2d%.2d",
          _params.warning_message_dir,
          PATH_DELIM,
          ntime.getYear(), ntime.getMonth(), ntime.getDay());
  
  // create day directory as needed

  if (ta_makedir_recurse(outputDir)) {
    int errNum = errno;
    cerr << "ERROR - SpolWarnSms::_writeMessageToDir" << endl;
    cerr << "  Cannot make directory for output files: " << outputDir << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // compute file path

  char outputPath[MAX_PATH_LEN];
  sprintf(outputPath, "%s%swarning_message_%.2d%.2d%.2d.txt",
          outputDir,
          PATH_DELIM,
          ntime.getHour(), ntime.getMin(), ntime.getSec());
  
  // write message to path
  
  FILE *out = fopen(outputPath, "w");
  if (out == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolWarnSms::_writeMessageToDir" << endl;
    cerr << "  Cannot open file for writing: " << outputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "%s\n", warningMsg.c_str());
  fclose(out);
  

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Wrote message to file: " << outputPath << endl;
  }

  // trigger a write to SMS?

  if (!_params.send_warnings_to_sms) {
    // no SMSs
    return 0;
  }

  // has enough time elapsed?

  int timeSinceLastSms = now - _timeLastSms;
  if (timeSinceLastSms < _params.time_between_sms_secs) {
    if (_params.debug) {
      cerr << "Time last SMS message: " << DateTime::strm(_timeLastSms) << endl;
      cerr << "  time to next SMS (secs): "
           << _params.time_between_sms_secs - timeSinceLastSms << endl;
    }
    return 0;
  }

  // get appropriate sms name and number info
  // from the warning periods
  
  vector<string> names, numbers;
  _getSmsNumbers(now, names, numbers);
  if (numbers.size() < 1) {
    if (_params.debug) {
      cerr << "No phone numbers, skipping SMS" << endl;
    }
    return 0;
  }
  string numberStr = numbers[0];
  for (size_t ii = 1; ii < numbers.size(); ii++) {
    numberStr += ",";
    numberStr += numbers[ii];
  }
  
  // write LdataInfo file to trigger SMS
  
  DsLdataInfo ldata(_params.warning_message_dir,
                    _params.debug >= Params::DEBUG_VERBOSE);
  
  string relPath;
  Path::stripDir(_params.warning_message_dir, outputPath, relPath);

  ldata.setLatestTime(now);
  ldata.setRelDataPath(relPath);
  ldata.setWriter("SpolWarnSms");
  ldata.setDataType("txt");
  ldata.setUserInfo1(warningMsg.substr(0, 160));
  ldata.setUserInfo2(numberStr);
  if (ldata.write(now)) {
    cerr << "ERROR - cannot write LdataInfo" << endl;
    cerr << " outputDir: " << _params.warning_message_dir << endl;
    return -1;
  }
  
  _timeLastSms = now;
  return 0;

}

/////////////////////////////
// update the SPDB data base

int SpolWarnSms::_writeMessageToSpdb(time_t now,
                                     const string &warningMsg)

{

  if (_params.debug) {
    cerr << "==>> updating SPDB" << endl;
  }
  
  DsSpdb spdb;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdb.setDebug();
  }
  spdb.addPutChunk(0, now, now + _params.monitoring_interval_secs,
                   warningMsg.size() + 1, warningMsg.c_str());
  
  if (spdb.put(_params.warning_spdb_url,
               SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
    cerr << "ERROR - SpolWarnSms::_writeMessageToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Wrote SPDB data to: " << _params.warning_spdb_url << endl;
  }

  return 0;

}

///////////////////////////////////////////
// handle a boolean entry in the status xml

int SpolWarnSms::_handleBooleanEntry(time_t now,
                                     const string &statusXml,
                                     const Params::xml_entry_t &entry,
                                     string &warningMsg)
  
{
  
  // get tag list
  
  vector<string> tags;
  TaStr::tokenize(entry.xml_tags, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - SpolWarnSms::_handleBooleanEntry" << endl;
    cerr << "  No tags found: " << entry.xml_tags << endl;
    return -1;
  }
  
  // read through the outer tags in status XML
  
  string buf(statusXml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - SpolWarnSms::_handleBooleanEntry" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << entry.xml_tags << endl;
      return -1;
    }
    buf = val;
  }

  // get the boolean value

  bool bval;
  if (TaXml::readBoolean(buf, bval)) {
    cerr << "ERROR - SpolWarnSms::_handleBooleanEntry" << endl;
    cerr << "  Cannot read bool value, buf: " << buf << endl;
    return -1;
  }

  // check value

  if (bval == entry.ok_boolean) {
    // no problem with this entry
    return 0;
  }
  
  // create message for this entry

  string msg("#");
  msg += entry.label;
  msg += ":";
  msg += buf;
  msg += "#";
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding msg: " << msg << endl;
  }

  // add to the warning message

  warningMsg += msg;

  return 0;

}

///////////////////////////////////////////
// handle a number entry in the status xml

int SpolWarnSms::_handleNumberEntry(time_t now,
                                    const string &statusXml,
                                    const Params::xml_entry_t &entry,
                                    string &warningMsg)
  
{
  
  // get tag list
  
  vector<string> tags;
  TaStr::tokenize(entry.xml_tags, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - SpolWarnSms::_handleNumberEntry" << endl;
    cerr << "  No tags found: " << entry.xml_tags << endl;
    return -1;
  }
  
  // read through the outer tags in status XML
  
  string buf(statusXml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - SpolWarnSms::_handleNumberEntry" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << entry.xml_tags << endl;
      return -1;
    }
    buf = val;
  }

  // get the numerical value

  double dval;
  if (TaXml::readDouble(buf, dval)) {
    cerr << "WARNING - SpolWarnSms::_handleNumberEntry" << endl;
    cerr << " Cannot read numerical value from: " << buf << endl;
    return -1;
  }
  
  // check value

  if (dval >= entry.valid_lower_limit &&
      dval <= entry.valid_upper_limit) {
    // no problem with this entry
    return 0;
  }
  
  // create message for this entry
  
  string msg("#");
  msg += entry.label;
  msg += ":";
  msg += buf;
  msg += "#";
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding msg: " << msg << endl;
  }

  // add to the warning message

  warningMsg += msg;

  return 0;

}

///////////////////////////////////////////////////////
// set up the warning periods from the parameters

int SpolWarnSms::_setupWarnPeriods()

{

  int iret = 0;
  _warnPeriods.clear();

  for (int ii = 0; ii < _params.monitoring_periods_n; ii++) {

    Params::monitoring_period_t &period = _params._monitoring_periods[ii];
    DateTime endTime;
    if (endTime.setFromW3c(period.end_time)) {
      cerr << "ERROR - SpolWarnSms::_setupWarnPeriods()" << endl;
      cerr << "  Bad time: " << period.end_time << endl;
      iret = -1;
    }
    WarnPeriod wp;
    wp.endTime = endTime;

    DailyInterval interval1;
    if (_setupDailyInterval(endTime, period.interval_1, interval1) == 0) {
      wp.intervals.push_back(interval1);
    }
    DailyInterval interval2;
    if (_setupDailyInterval(endTime, period.interval_2, interval2) == 0) {
      wp.intervals.push_back(interval2);
    }
    DailyInterval interval3;
    if (_setupDailyInterval(endTime, period.interval_3, interval3) == 0) {
      wp.intervals.push_back(interval3);
    }
    DailyInterval interval4;
    if (_setupDailyInterval(endTime, period.interval_4, interval4) == 0) {
      wp.intervals.push_back(interval4);
    }

    _warnPeriods.push_back(wp);
    
  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {

    for (size_t ii = 0; ii < _warnPeriods.size(); ii++) {
      
      const WarnPeriod &wp = _warnPeriods[ii];

      cerr << "===>>> monitoring period: " << ii << " <<<===" << endl;
      cerr << "  mon period endTime: " << wp.endTime.getStr() << endl;

      for (size_t jj = 0; jj < wp.intervals.size(); jj++) {
        const DailyInterval &interval = wp.intervals[jj];
        fprintf(stderr, "    intervalStartTime: %.2d-%.2d\n",
                interval.startTime.getHour(),
                interval.startTime.getMin());
        fprintf(stderr, "    intervalEndTime  : %.2d-%.2d\n",
                interval.endTime.getHour(),
                interval.endTime.getMin());
        for (size_t kk = 0; kk < interval.names.size(); kk++) {
          cerr << "    name, number     : " 
               << interval.names[kk] << ", "
               << interval.numbers[kk] << endl;
        } // kk
      } // jj

    } // ii

  } // if (_params.debug >= Params::DEBUG_VERBOSE)

  return iret;

}

///////////////////////////////////////////////////////
// set up the daily intervals

int SpolWarnSms::_setupDailyInterval(const DateTime &endTime,
                                     const string &paramLine,
                                     DailyInterval &interval)
  
{

  // tokenize line on commas

  vector<string> tags;
  TaStr::tokenize(paramLine, ", ", tags);
  if (tags.size() < 2) {
    // no names, so no warnings
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - SpolWarnSms::_setupDailyInterval" << endl;
      cerr << "  No names for interval: " << paramLine << endl;
      return -1;
    }
  }
  
  // read times

  int startHour, startMin, endHour, endMin;
  if (sscanf(tags[0].c_str(), "%2d:%2d-%2d:%2d",
             &startHour, &startMin, &endHour, &endMin) != 4) {
    cerr << "WARNING - SpolWarnSms::_setupDailyInterval" << endl;
    cerr << "  Bad start and end time format: " << paramLine << endl;
    return -1;
  };
  int startSec = 0;
  int endSec = 0;
  if (endHour == 24 && endMin == 0) {
    endHour = 23;
    endMin = 59;
    endSec = 59;
  }
  
  // set daily start and end times

  interval.startTime = endTime;
  interval.startTime.setHour(startHour);
  interval.startTime.setMin(startMin);
  interval.startTime.setSec(startSec);
  
  interval.endTime = endTime;
  interval.endTime.setHour(endHour);
  interval.endTime.setMin(endMin);
  interval.endTime.setSec(endSec);

  // set names and numbers

  for (size_t ii = 1; ii < tags.size(); ii++) {
    string name(tags[ii]);
    string number;
    if (_lookUpNumber(name, number) == 0) {
      interval.names.push_back(name);
      interval.numbers.push_back(number);
    } else {
      cerr << "WARNING - SpolWarnSms::_setupDailyInterval" << endl;
      cerr << "  Bad name, not in phone book: " << name << endl;
      return -1;
    }
  }
  
  return 0;

}

///////////////////////////////////////////////////////
// look up phone number from name

int SpolWarnSms::_lookUpNumber(const string &name,
                               string &number)
  
{

  for (int ii = 0; ii < _params.phone_book_n; ii++) {

    Params::phone_book_entry_t &entry = _params._phone_book[ii];
    if (entry.name == name) {
      number = entry.number;
      return 0;
    }

  } // ii

  return -1;

}

///////////////////////////////////////////////////////
// get numbers for SMS

void SpolWarnSms::_getSmsNumbers(time_t now,
                                 vector<string> &names,
                                 vector<string> &numbers)
  
{

  // get appropriate sms name and number info
  // from the warning periods
  
  for (size_t ii = 0; ii < _warnPeriods.size(); ii++) {

    const WarnPeriod &wp = _warnPeriods[ii];
    if (now > wp.endTime.utime()) {
      // period in the past
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Period in the past, skipping: "
             << DateTime::strm(wp.endTime.utime()) << endl;
      }
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Using period ending at: "
           << DateTime::strm(wp.endTime.utime()) << endl;
    }

    for (size_t jj = 0; jj < wp.intervals.size(); jj++) {

      const DailyInterval &interval = wp.intervals[jj];
      
      DateTime startPeriod(now);
      startPeriod.setTime(interval.startTime.getHour(),
                          interval.startTime.getMin(), 0);

      DateTime endPeriod(now);
      endPeriod.setTime(interval.endTime.getHour(),
                        interval.endTime.getMin(), 0);

      if (now >= startPeriod.utime() && now <= endPeriod.utime()) {
        names = interval.names;
        numbers = interval.numbers;
        if (_params.debug) {
          cerr << "Getting names & numbers for time: " 
               << DateTime::strm(now) << endl;
          for (size_t kk = 0; kk < numbers.size(); kk++) {
            cerr << "  name, number: " << names[kk] << ", "
                 << numbers[kk] << endl;
          } // kk
        }
        return;
      }

    } // jj

  } // ii

}


  
