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
// PrintDataMap.cc
//
// PrintDataMap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// PrintDataMap contact the DataMapper, and prints out the
// data set information.
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLdataInfo.hh>
#include <dsserver/DmapAccess.hh>
#include "PrintDataMap.hh"
#include "Args.hh"
using namespace std;

// Constructor

PrintDataMap::PrintDataMap(int argc, char **argv)

{

  OK = TRUE;
  
  // set programe name
  
  _progName = "PrintDataMap";

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // initialize Procmap module

  if (_args.cont) {
    PMU_auto_init(_progName.c_str(),
                  _args.instance.c_str(), PROCMAP_REGISTER_INTERVAL);
  }

  return;

}

// destructor

PrintDataMap::~PrintDataMap()

{

  // unregister with Procmap

  if (_args.cont) {
    PMU_auto_unregister();
  }

}

//////////////////////////////////////////////////
// Run

int PrintDataMap::Run()
{

  if (_args.cont) {

    bool forever = true;
    while  (forever) {
      if (_query() == 0) {
	_print();
      }
      for (int i = 0; i < _args.contInterval; i++) {
        PMU_auto_register("Zzzzz ...");
	sleep (1);
      }
    }

  } else {

    if (_query()) {
      cerr << "No data mapper running on host: " << _args.hostName << endl;
      return (-1);
    } else {
      _print();
    }

  }

  return (0);
  
}

////////////////////////////////////////////////////////////
// _query()

int PrintDataMap::_query()

{

  if (_args.cont) {
    PMU_auto_register("Querying ...");
  }

  if (_args.dataType.size() == 0 && _args.dataDir.size() == 0) {

    if (_access.reqAllInfo(_args.hostName.c_str())) {
      return (-1);
    }

  } else {

    if (_access.reqSelectedInfo(_args.dataType.c_str(),
				 _args.dataDir.c_str(),
				 _args.hostName.c_str())) {
      return (-1);
    }

  }

  return (0);

}

////////////////////////////////////////////////////////////
// _print()

int PrintDataMap::_print()

{

  if (_args.cont) {
    PMU_auto_register("Printing ...");
  }

  if (!_args.toFile) {

    // print to std out
    
    if (_args.xml) {
      _printXml(cout);
    } else if (_args.plain) {
      _printPlain(cout);
    } else {
      _printFormatted(cout);
    }

  } else {
    
    // compute output file path
    // This is relative to DATA_DIR or RAP_DATA_DIR, if they exist,
    // unless absolute i.e. starting with / or .

    char filename[MAX_PATH_LEN];
    char path[MAX_PATH_LEN];

    string outputDir;
    RapDataDir.fillPath(_args.outputDir, outputDir);
    if (ta_makedir_recurse(outputDir.c_str())) {
      cerr << "ERROR - PrintDataMap::_queryMapper" << endl;
      cerr << "  Cannot make output dir: " << outputDir << endl;
      return -1;
    }

    // set file ext

    string ext = "table";
    if (_args.xml) {
      ext = "xml";
    } else if (_args.plain) {
      ext = "plain";
    }

    time_t checkTime = time(NULL);
    if (_access.getNInfo() > 0) {
      checkTime = _access.getInfo(0).check_time;
    }
    DateTime hostDateTime(checkTime);

    sprintf(filename, "datasets_%s_%.4d%.2d%.2d_%.2d%.2d%.2d.%s",
            _args.hostName.c_str(),
            hostDateTime.getYear(),
            hostDateTime.getMonth(),
            hostDateTime.getDay(),
	    hostDateTime.getHour(),
            hostDateTime.getMin(),
            hostDateTime.getSec(),
            ext.c_str());

    sprintf(path, "%s%s%s", outputDir.c_str(), PATH_DELIM, filename);

    ofstream outFile;
    outFile.open(path);
    if (outFile.fail()) {
      cerr << "ERROR - PrintDataMap::_queryMapper" << endl;
      cerr << "  Cannot open output file: " << path << endl;
      return -1;
    }
    
    // print to file

    if (_args.xml) {
      _printXml(outFile);
    } else if (_args.plain) {
      _printPlain(outFile);
    } else {
      _printFormatted(outFile);
    }

    outFile.close();

    // write latest data info file

    DsLdataInfo ldata(outputDir);
    ldata.setRelDataPath(filename);
    ldata.setWriter(_progName.c_str());
    ldata.setDataFileExt(ext.c_str());
    if (_args.xml) {
      ldata.setDataType("xml");
    } else {
      ldata.setDataType("text");
    }
    if (ldata.write(checkTime)) {
      cerr << "ERROR - PrintDataMap::_print" << endl;
      cerr << "  Cannot write latest data info file, dir: "
           << outputDir << endl;
      return -1;
    }

  }

  return 0;

}

////////////////////////////////////////////////////////////
// _printPlain()

void PrintDataMap::_printPlain(ostream &out)

{

  int nInfo = _access.getNInfo();

  out << nInfo << endl;
  time_t now = time(NULL);

  for (int i = 0; i < nInfo; i++) {

    const DMAP_info_t &info = _access.getInfo(i);

    if (_args.lateThreshold >= 0) {
      int age = now - info.latest_time;
      if (age < _args.lateThreshold) {
	continue;
      }
    }

    out << info.datatype << " "
        << info.dir << " "
        << info.hostname << " "
        << info.ipaddr << " "
        << info.latest_time << " "
        << info.forecast_lead_time << " "
        << info.last_reg_time << " "
        << info.start_time << " "
        << info.end_time << " "
        << info.nfiles << " "
        << info.total_bytes << endl;
    
  }
  
}

////////////////////////////////////////////////////////////
// _printXml()

void PrintDataMap::_printXml(ostream &out)

{

  out << "<monitored hostname=\"" << _args.hostName << "\">" << endl;

  int nInfo = _access.getNInfo();

  for (int i = 0; i < nInfo; i++) {

    const DMAP_info_t &info = _access.getInfo(i);

    out << "  <dataset "
        << "hostname=\"" << _args.hostName << "\" "
        << "datatype=\"" << info.datatype << "\" "
        << "dir=\"" << info.dir << "\">" << endl;

    out << "    <monitor_time>"
        << info.check_time << "</monitor_time>" << endl;
    out << "    <latest_time>"
        << info.latest_time << "</latest_time>" << endl;
    out << "    <forecast_lead_time>"
        << info.forecast_lead_time << "</forecast_lead_time>" << endl;
    out << "    <last_reg_time>"
        << info.last_reg_time << "</last_reg_time>" << endl;
    out << "    <start_time>"
        << info.start_time << "</start_time>" << endl;
    out << "    <end_time>"
        << info.end_time << "</end_time>" << endl;
    out << "    <nfiles>"
        << info.nfiles << "</nfiles>" << endl;
    out << "    <total_bytes>"
        << info.total_bytes << "</total_bytes>" << endl;

    out << "  </dataset>" << endl;

  }
  
  out << "</monitored>" << endl;

}

////////////////////////////////////////////////////////////
// _printFormatted()

void PrintDataMap::_printFormatted(ostream &out)

{

  time_t check_time = time(NULL);
  int nInfo = _access.getNInfo();
  if (nInfo > 0) {
    check_time = _access.getInfo(0).check_time;
  }
  
  size_t colonPos = _args.hostName.rfind(":", string::npos);
  string hostName, hostList;
  bool isRelay;
  if (colonPos == string::npos) {
    hostName = _args.hostName;
    isRelay = false;
  } else {
    hostName.assign(_args.hostName, colonPos + 1, string::npos);
    hostList = _args.hostName;
    isRelay = true;
  }

  if (nInfo == 0) {
    out << "==== No data sets on host '" << hostName
        << "' at time " << utimstr(check_time)
        << " ====" << endl;
    if (isRelay) {
      out << "     Relay host list: '" << hostList  << "'" << endl;
    }
    return;
  }

  out << endl;
  out << "=========== Data on host '" << hostName
      << "' at time " << utimstr(check_time)
      << " ==========" << endl;
  if (isRelay) {
    out << "            Relay host list: '" << hostList
	 << "'" << endl;
  }
  out << endl;
  
  // compute string field widths

  unsigned int maxDataTypeLen = strlen("DataType");
  unsigned int maxDirLen = strlen("Dir");
  unsigned int maxHostNameLen = strlen("HostName");
  unsigned int maxIpAddrLen = strlen("IpAddr");
  bool haveFcast = false;
  
  for (int i = 0; i < nInfo; i++) {
    const DMAP_info_t &info = _access.getInfo(i);
    maxDataTypeLen = MAX(maxDataTypeLen, strlen(info.datatype));
    maxDirLen = MAX(maxDirLen, strlen(info.dir));
    maxHostNameLen = MAX(maxHostNameLen, strlen(info.hostname));
    maxIpAddrLen = MAX(maxIpAddrLen, strlen(info.ipaddr));
    if (info.forecast_lead_time > 0) {
      haveFcast = true;
    }
  }

//   maxDirLen += 2;
//   maxHostNameLen += 2;
//   maxIpAddrLen += 2;

  // Headings

  int total_width = 0;
  out << setw(maxDataTypeLen) << left << "DataType";
  out << "  " << setw(maxDirLen) << left << "Dir";
  out << "  " << setw(maxHostNameLen) << left << "HostName";
  total_width += (maxDataTypeLen + maxDirLen + maxHostNameLen + 4);
  if (_args.ip) {
    out << "  " << setw(maxIpAddrLen) << left << "IpAddr";
    total_width += maxIpAddrLen + 2;
  }
  out << right;
  if (_args.latest) {
    if (_args.relt) {
      out << setw(11) << "Latest";
      total_width += 11;
    } else {
      out << setw(22) << "Latest time";
      total_width += 22;
    }
    if (haveFcast) {
      out << setw(11) << "FcastLead";
      total_width += 11;
    }
  }
  if (_args.lreg) {
    if (_args.relt) {
      out << setw(11) << "Last reg";
      total_width += 11;
    } else {
      out << setw(22) << "Last reg time";
      total_width += 22;
    }
  }
  if (_args.dates) {
    out << setw(12) << "Start date";
    out << setw(12) << "End date";
    total_width += 24;
  }
  if (_args.size) {
    out << setw(8) << "nFiles";
    out << setw(8) << "nBytes";
    total_width += 16;
  }
  if (_args.status) {
    out << "  Status";
  }
  out << endl;

  out << setw(maxDataTypeLen) << left << "========";
  out << "  " << setw(maxDirLen) << left << "===";
  out << "  " << setw(maxHostNameLen) << left << "========";
  if (_args.ip) {
    out << "  " << setw(maxIpAddrLen) << left << "======";
  }
  out << right;

  if (_args.latest) {
    if (_args.relt) {
      out << setw(11) << "======";
    } else {
      out << setw(22) << "===========";
    }
    if (haveFcast) {
      out << setw(11) << "=========";
    }
  }
  if (_args.lreg) {
    if (_args.relt) {
      out << setw(11) << "========";
    } else {
      out << setw(22) << "=============";
    }
  }
  if (_args.dates) {
    out << setw(12) << "==========";
    out << setw(12) << "========";
  }
  if (_args.size) {
    out << setw(8) << "======";
    out << setw(8) << "======";
  }
  if (_args.status) {
    out << "  ======";
  }
  out << endl;

  // field info

  double total_bytes = 0;
  double total_files = 0;
  time_t now = time(NULL);

  for (int i = 0; i < nInfo; i++) {

    const DMAP_info_t &info = _access.getInfo(i);
    
    if (_args.lateThreshold >= 0) {
      int age = now - info.latest_time;
      if (age < _args.lateThreshold) {
	continue;
      }
    }

    out << setw(maxDataTypeLen) << left << info.datatype;
    out << "  " << setw(maxDirLen) << left << info.dir;
    out << "  " << setw(maxHostNameLen) << left << info.hostname;
    if (_args.ip) {
      out << "  " << setw(maxIpAddrLen) << left << info.ipaddr;
    }
    out << right;

    if (_args.latest) {
      if (_args.relt) {
	_printLatest(out, 11, info.latest_time, info.check_time);
      } else {
	_printLatest(out, 22, info.latest_time, info.check_time);
      }
      if (haveFcast) {
	_printLead(out, 11, info.forecast_lead_time);
      }
    }
    if (_args.lreg) {
      if (_args.relt) {
	_printLatest(out, 11, info.last_reg_time, info.check_time);
      } else {
	_printLatest(out, 22, info.last_reg_time, info.check_time);
      }
    }
    if (_args.dates) {
      _printDate(out, 12, info.start_time);
      _printDate(out, 12, info.end_time);
    }
    if (_args.size) {
      _printSize(out, 8, info.nfiles);
      _printSize(out, 8, info.total_bytes);
    }
    total_files += info.nfiles;
    total_bytes += info.total_bytes;

    if (_args.status) {
      out << "  " << info.status;
    }
    out << endl;

  } // i

  if (_args.size &&
      (total_files > 0 || total_bytes > 0)) {
    out << setw(total_width) << "======  ======" << endl;
    _printSize(out, total_width - 8, total_files);
    _printSize(out, 8, total_bytes);
    out << endl;
  }

  out << endl;
  
}

///////////////
// _printLatest

void PrintDataMap::_printLatest(ostream &out, int width,
				time_t latest_time,
				time_t check_time)
  
{

  char str[32];

  if (_args.relt) {

    if (latest_time <= 0) {

      sprintf(str, " ");

    } else if (latest_time == check_time) {

      sprintf(str, "00:00:00");

    } else if (check_time > latest_time) {
      time_t tdiff = check_time - latest_time;
      sprintf(str, "-%.2d:%.2d:%.2d",
	      (int) tdiff/3600,
	      (int) (tdiff%3600)/60,
	      (int) tdiff%60);
    } else {
      time_t tdiff = latest_time - check_time;
      sprintf(str, "+%.2d:%.2d:%.2d",
	      (int) tdiff/3600,
	      (int) (tdiff%3600)/60,
	      (int) tdiff%60);
    }

  } else {

    if (latest_time == -1 || latest_time == 0) {
      
      sprintf(str, " ");

    } else {

      date_time_t latest;
      latest.unix_time = latest_time;
      uconvert_from_utime(&latest);
      
      sprintf(str, "%.4d/%.2d/%.2d-%.2d:%.2d:%.2d",
	      latest.year, latest.month, latest.day,
	      latest.hour, latest.min, latest.sec);

    }
	    
  }

  out << setw(width) << str;

}

///////////////
// _printLead

void PrintDataMap::_printLead(ostream &out, int width,
			      int lead_time)
  
{

  char str[32];

  if (lead_time < 0) {
    
    sprintf(str, " ");
    
  } else {

    sprintf(str, "+%.2d:%.2d:%.2d",
	    (int) lead_time/3600,
	    (int) (lead_time%3600)/60,
	    (int) lead_time%60);

  }
  
  out << setw(width) << str;

}

/////////////
// _printDate

void PrintDataMap::_printDate(ostream &out, int width,
			      time_t ref_time)
  
{

  char str[32];

  if (ref_time <= 0) {

    sprintf(str, " ");

  } else {

    date_time_t ref;
    ref.unix_time = ref_time;
    uconvert_from_utime(&ref);
    
    sprintf(str, "%.4d/%.2d/%.2d",
	    ref.year, ref.month, ref.day);

  }
    
  out << setw(width) << str;

}

/////////////
// _printSize

void PrintDataMap::_printSize(ostream &out, int width,
			      double size)
  
{

  char str[32];

  if (size < 1) {
    sprintf(str, " ");
  }  else if (size < 1e3) {
    sprintf(str, "%g", size);
  }  else if (size < 10e3) {
    sprintf(str, "%.1fK", size/1e3);
  }  else if (size < 1000e3) {
    sprintf(str, "%.0fK", size/1e3);
  }  else if (size < 10e6) {
    sprintf(str, "%.1fM", size/1e6);
  }  else if (size < 1000e6) {
    sprintf(str, "%.0fM", size/1e6);
  }  else if (size < 10e9) {
    sprintf(str, "%.1fG", size/1e9);
  }  else if (size < 1000e9) {
    sprintf(str, "%.0fG", size/1e9);
  }  else if (size < 10e12) {
    sprintf(str, "%.1fT", size/1e12);
  }  else {
    sprintf(str, "%.0fT", size/1e12);
  }

  out << setw(width) << str;

}


