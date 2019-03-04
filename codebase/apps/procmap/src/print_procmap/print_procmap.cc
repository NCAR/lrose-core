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
// print_procmap.cc
//
// print_procmap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
///////////////////////////////////////////////////////////////

#include "print_procmap.hh"
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/PmuInfo.hh>
#ifndef NO_RAP_DATA_DIR
#include <didss/RapDataDir.hh>
#endif
#ifndef NO_LDATAINFO
#include <dsserver/DsLdataInfo.hh>
#endif
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
using namespace std;

#define BASIC_FORMAT  "%-12.12s %-12.12s %-10.10s %-10.10s %-6.6s"

// Constructor

print_procmap::print_procmap(int argc, char **argv)
  
{

  OK = true;
  strcpy(_basicFormat, BASIC_FORMAT);

  // set programe name

  _progName = "print_procmap";
  

  // set up _tokenMap
  _tokenMap["<"] = "{";
  _tokenMap[">"] = "}";
  _tokenMap["&"] = "and";
  _tokenMap["'"] = "`";
  _tokenMap["\""] = "``";
  

  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = false;
    return;
  }
  
  return;

}

// destructor

print_procmap::~print_procmap()

{


}

//////////////////////////////////////////////////
// Run

int print_procmap::Run()

{

  if (_args.printContinuous) {

    while (true) {
      _queryMapper();
      sleep(_args.printInterval);
    }

  } else {

    if (_queryMapper()) {
      return -1;
    }

  }

  return 0;
  
}

//////////////////////////////////////////////////
// query_mapper
//
// Returns 0 on success, -1 on failure

int print_procmap::_queryMapper()
  
{

  size_t colonPos = _args.hostname.rfind(":", string::npos);
  string hostName, hostList;
  bool isRelay;
  if (colonPos == string::npos) {
    hostName = _args.hostname;
    isRelay = false;
  } else {
    hostName.assign(_args.hostname, colonPos + 1, string::npos);
    hostList = _args.hostname;
    isRelay = true;
  }

  PmuInfo pmuInfo;
  if (pmuInfo.read(_args.hostname)) {
    if (!_args.hostTimeOnly && !_args.hostUtimeOnly) {
      fprintf(stdout, "No procmap reachable on %s\n", hostName.c_str());
      if (isRelay) {
	fprintf(stdout, "Relay host list: '%s'\n", hostList.c_str());
      }
      if (_args.debug) {
	cerr << pmuInfo.getErrStr() << endl;
      }
    }
    cerr << "Cannot contact procmap, host: " << _args.hostname << endl;
    return -1;
  }

  time_t hostTime = pmuInfo.getReplyTime();
  DateTime hostDateTime(hostTime);
  if (_args.hostTimeOnly) {
    fprintf(stdout, "%.4d %.2d %.2d %.2d %.2d %.2d\n",
	    hostDateTime.getYear(),
	    hostDateTime.getMonth(),
	    hostDateTime.getDay(),
	    hostDateTime.getHour(),
	    hostDateTime.getMin(),
	    hostDateTime.getSec());
    return 0;
  } else if (_args.hostUtimeOnly) {
    fprintf(stdout, "%ld\n", (long) hostTime);
    return 0;
  }

  time_t upTime = pmuInfo.getUpTime();
  int nProcs = pmuInfo.getNProcs();

  if (nProcs == 0 && !_args.printXml) {
    fprintf(stdout, "No procs registered with %s\n", hostName.c_str());
    if (_args.printPlain) {
      fprintf(stdout, "%ld\n", upTime);
    } else {
      if (isRelay) {
	fprintf(stdout, "Relay host list: '%s'\n", hostList.c_str());
      }
      fprintf(stdout, "Uptime: %s\n\n", _tdiffStr(upTime).c_str());
    }
    return 0;
  }
  
  // sort procs

  PROCMAP_info_t *info = (PROCMAP_info_t *) pmuInfo.getInfoArray();
  qsort(info, nProcs, sizeof(PROCMAP_info_t), _sortInfo);

  if (!_args.printToFile) {

    // print to stdout
    
    if (_args.printXml) {
      _printXml(stdout, hostName, hostTime, upTime, nProcs, info);
    } else if (_args.printPlain) {
      _printPlain(stdout, hostName, hostTime, upTime, nProcs, info);
    } else {
      _printTable(stdout, hostName, hostList, hostTime, upTime, isRelay, nProcs, info);
    }
    
  } else {
#ifndef NO_RAP_DATA_DIR    
    // make output dir, relative to DATA_DIR or RAP_DATA_DIR, if they exist

    string outputDir;
    RapDataDir.fillPath(_args.outputDir, outputDir);

    if (ta_makedir_recurse(outputDir.c_str())) {
      cerr << "ERROR - print_procmap::_queryMapper" << endl;
      cerr << "  Cannot make output dir: " << outputDir << endl;
      return -1;
    }

    // set file ext

    string ext = "table";
    if (_args.printXml) {
      ext = "xml";
    } else if (_args.printPlain) {
      ext = "plain";
    }

    // compute output file path

    char filename[MAX_PATH_LEN];
    char path[MAX_PATH_LEN];
    
    sprintf(filename, "processes_%s_%.4d%.2d%.2d_%.2d%.2d%.2d.%s",
            hostName.c_str(),
	    hostDateTime.getYear(),
	    hostDateTime.getMonth(),
	    hostDateTime.getDay(),
	    hostDateTime.getHour(),
	    hostDateTime.getMin(),
	    hostDateTime.getSec(),
            ext.c_str());

    sprintf(path, "%s%s%s", outputDir.c_str(), PATH_DELIM, filename);
    
    FILE *out = stdout;
    if ((out = fopen(path, "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - print_procmap::_queryMapper" << endl;
      cerr << "  Cannot open output file: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
    // print the proc information to file
    
    if (_args.printXml) {
      _printXml(out, hostName, hostTime, upTime, nProcs, info);
    } else if (_args.printPlain) {
      _printPlain(out, hostName, hostTime, upTime, nProcs, info);
    } else {
      _printTable(out, hostName, hostList, hostTime, upTime, isRelay, nProcs, info);
    }
    fclose(out);

#ifndef NO_LDATAINFO

    // write latest data info file

    DsLdataInfo ldata(outputDir);
    ldata.setRelDataPath(filename);
    ldata.setWriter(_progName.c_str());
    ldata.setDataFileExt(ext.c_str());
    if (_args.printXml) {
      ldata.setDataType("xml");
    } else {
      ldata.setDataType("text");
    }
    if (ldata.write(hostTime)) {
      cerr << "ERROR - print_procmap::_queryMapper" << endl;
      cerr << "  Cannot write latest data info file, dir: " << outputDir << endl;
      return -1;
    }
#endif

#endif 
  }

  return 0;
  
}

/////////////////////////////////////////////////////////////////
// _printTable
// Print procmap info as table

void print_procmap::_printTable(FILE *out,
                                const string &procmapHostName,
                                const string &relayHostList,
                                time_t procmapHostTime,
                                time_t procmapUpTime,
                                bool isRelay,
                                int nProcs,
                                const PROCMAP_info_t *procs)
     
{

  // realtime header

  fprintf(out, "\n");
  fprintf(out, "PROCS REGISTERED - %s - %s",
	  procmapHostName.c_str(), ctime(&procmapHostTime));
  if (isRelay) {
    fprintf(out, "Relay host list: '%s'\n", relayHostList.c_str());
  }
  fprintf(out, "Uptime: %s\n\n", _tdiffStr(procmapUpTime).c_str());

  _computeFormat(nProcs, procs);
  _printHeader(out);
  
  for (int i=0; i< nProcs; i++) {
    const PROCMAP_info_t *dinfo = &procs[i];
    _printData(out, dinfo, procmapHostTime);
  }
  
  fflush(out);
  
}

///////////////////////////////////////////////////////////
// _printPlain : display the information in plain format

void print_procmap::_printPlain(FILE *out,
                                const string &procmapHostName,
                                time_t procmapHostTime,
                                time_t procmapUpTime,
                                int nProcs,
                                const PROCMAP_info_t *procs)
     
{

  fprintf(out, "%s\n", procmapHostName.c_str());
  fprintf(out, "%ld\n", procmapUpTime);

  for (int i=0; i< nProcs; i++) {
    
    const PROCMAP_info_t *dinfo = &procs[i];
    
    fprintf(out, "%s %s %s %s %d %d %d %d %d %d",
	    dinfo->name,
	    dinfo->instance,
	    dinfo->host,
	    dinfo->user,
	    (int) dinfo->pid,
	    (int) dinfo->max_reg_interval,
	    (int) (dinfo->heartbeat_time - procmapHostTime),
	    (int) (dinfo->start_time - procmapHostTime),
	    (int) dinfo->n_reg,
	    (int) dinfo->status);
    
    fprintf(out, " \"%s\"\n", dinfo->status_str);
    
  } // i
  
  fflush(out);
  
}

///////////////////////////////////////////////////////////
// _printPlain : display the information in plain format

void print_procmap::_printXml(FILE *out,
                              const string &procmapHostName,
                              time_t procmapHostTime,
                              time_t procmapUpTime,
                              int nProcs,
                              const PROCMAP_info_t *procs)
                              
{
  
  fprintf(out, "<monitored hostname=\"%s\">\n", procmapHostName.c_str());
  
  for (int i=0; i< nProcs; i++) {
    
    const PROCMAP_info_t *dinfo = &procs[i];
    
    fprintf(out,
            "  <process hostname=\"%s\" name=\"%s\" "
            "instance=\"%s\" user=\"%s\">\n",
	    _clean4XML(procmapHostName).c_str(),
	    _clean4XML(dinfo->name).c_str(),
	    _clean4XML(dinfo->instance).c_str(),
	    _clean4XML(dinfo->user).c_str());

    fprintf(out, "    <monitor_time>%ld</monitor_time>\n", (long) procmapHostTime);
    fprintf(out, "    <procmap_uptime>%ld</procmap_uptime>\n", (long) procmapUpTime);
    fprintf(out, "    <pid>%d</pid>\n", (int) dinfo->pid);
    fprintf(out, "    <max_reg_interval>%d</max_reg_interval>\n",
            (int) dinfo->max_reg_interval);
    fprintf(out, "    <heartbeat_time>%ld</heartbeat_time>\n",
            (long) dinfo->heartbeat_time);
    fprintf(out, "    <start_time>%ld</start_time>\n",
            (long) dinfo->start_time);
    fprintf(out, "    <n_reg>%d</n_reg>\n", (int) dinfo->n_reg);
    fprintf(out, "    <status>%d</status>\n", dinfo->status);
    fprintf(out, "    <status_str>%s</status_str>\n", 
	    _clean4XML(dinfo->status_str).c_str());
    fprintf(out, "  </process>\n");
    
  } // i
  
  fprintf(out, "</monitored>\n");

  fflush(out);
          
}

//////////////////
//  _clean4XML()
// Based upon http://www.w3schools.com/xml/xml_cdata.asp
// it is good practice to not use the characters <>&'" 
// in XML data.  This function replaces them as follows:
//    '<' --> '{'
//    '>' --> '}'
//	'&' --> 'and'
//    ''' --> '`'
//    '"' --> '``'
/////////////////
string print_procmap::_clean4XML(string s)
{
  map< string, string >::const_iterator iter;
  for(iter = _tokenMap.begin(); iter != _tokenMap.end(); ++iter) {
    
    string::size_type idx = 0;
    while((idx = s.find(iter->first)) != string::npos) {

      // handle the "and" and "''" swaps differently
      if(iter->second.length() == 1) {
	s = s.replace(idx, 1, iter->second);
      }
      else {
	s = s.erase(idx, 1);
	s = s.insert(idx, iter->second);
      }
      
    }
  } 

  return s;
}

/////////////////
// _printData()
///

void print_procmap::_printData(FILE *out, const PROCMAP_info_t *dinfo, time_t hostTime)

{
  
  fprintf(out, _basicFormat,
	  dinfo->name,
	  dinfo->instance,
	  _stripHost(dinfo->host).c_str(),
	  dinfo->user,
	  _pidString(dinfo->pid).c_str());

  if(_args.printMaxintv) {
    fprintf(out, " %-7.7d", (int) dinfo->max_reg_interval);
  }

  if(_args.printHb) {
    fprintf(out, " %-8.8s", _secsStr(dinfo->heartbeat_time, hostTime).c_str());
  }
  
  if(_args.printUptime) {
    fprintf(out, " %-8.8s", _secsStr(dinfo->start_time, hostTime).c_str());
  }

  if(_args.printNreg) {
    fprintf(out, " %-5d", dinfo->n_reg);
  }

  if(_args.printStatus) {
    // fprintf(out, " %-5d", dinfo->status); */
    fprintf(out, " %s", dinfo->status_str);
  }
  
  fprintf(out, "\n");

}
    
///////////////////////////////////////////////
// _printHeader()

void print_procmap::_printHeader(FILE *out)
  
{

  fprintf(out, _basicFormat,
	  "Name", "Instance", "Host", "User", "Pid");
  
  if(_args.printMaxintv) {
    fprintf(out, " %-7.7s", "Maxint");
  }
  
  if(_args.printHb) {
    fprintf(out, " %-8.8s", "Htbeat");
  }
  
  if(_args.printUptime) {
    fprintf(out, " %-8.8s", "Uptime");
  }

  if(_args.printNreg) {
    fprintf(out, " %-5.5s", "Nreg ");
  }

  if(_args.printStatus) {
    fprintf(out, " %-6.6s", "Status ");
  }

  fprintf(out, "\n");

  fprintf(out, _basicFormat,
	  "====", "========", "====", "====", "===");
  
  if(_args.printMaxintv) {
    fprintf(out, " %-7.7s", "======");
  }

  if(_args.printHb) {
    fprintf(out, " %-8.8s", "======");
  }

  if(_args.printUptime) {
    fprintf(out, " %-8.8s", "======");
  }

  if(_args.printNreg) {
    fprintf(out, " %-5.5s", "==== ");
  }

  if(_args.printStatus) {
    fprintf(out, " %-6.6s", "====== ");
  }

  fprintf(out, "\n");

}
    
////////////////////////////////////////////
// _pidString()

string print_procmap::_pidString(int pid)

{

  char pidstr[32];
  sprintf(pidstr, "%d", pid);
  return (pidstr);

}

/////////////////////////////////////////////
// _secsStr
//
// Converts secs to str of hr:min:sec

string print_procmap::_secsStr(time_t refTime, time_t hostTime)

{

  char str[32];
  long tdiff;
  
  if (hostTime == -1 || hostTime == 0) {
    sprintf(str, "-1");
  } else if (refTime > hostTime) {
    tdiff = refTime - hostTime;
    sprintf(str, "-%s", _tdiffStr(tdiff).c_str());
  } else {
    tdiff = hostTime - refTime;
    sprintf(str, "%s", _tdiffStr(tdiff).c_str());
  }

  return (str);
  
}

///////////////////////////////////////////////
// _stripHost()
//
// Strips fully qualified data from hostname.
// i.e. remove characters past first '.'

string print_procmap::_stripHost(const char *hostname)

{

  char stripped[128];
  char *ptr;
  STRncopy(stripped, hostname, 128);

  ptr = strchr(stripped, '.');
  if (ptr != NULL) {
    *ptr = '\0';
  }

  return (stripped);

}


///////////////////////////////////
// _sortInfo()
//
// qsort compare routine

int print_procmap::_sortInfo(const void *p1, const void *p2)

{
  
  PROCMAP_info_t *i1 = (PROCMAP_info_t *) p1;
  PROCMAP_info_t *i2 = (PROCMAP_info_t *) p2;
  int ret;
  
  ret = strcmp(i1->host, i2->host);
  if (0 == ret) {
    ret = strcmp(i1->user, i2->user);
    if (0 == ret) {
      ret = strcmp(i1->name, i2->name);
      if (0 == ret) {
	ret = strcmp(i1->instance, i2->instance);
      }
    }
  }
  return ret;

}

///////////////////////////////////
// _tdiffStr
//
// Converts time diff to string

string print_procmap::_tdiffStr(int tdiff)

{

  char str[32];
  tdiff = abs(tdiff);
  
  if (tdiff > 86399) {
    sprintf(str, "%.3g d", (double) tdiff / 86400.0);
  } else {
    sprintf(str, "%d:%02d:%02d",
	    tdiff/3600,
	    (tdiff%3600)/60,
	    tdiff%60);
  }

  return (str);
  
}

///////////////////////////////////////////////////////////
// set the format for the printout

void print_procmap::_computeFormat(int nProcs,
                                   const PROCMAP_info_t *procs)
  
{

  int nameColWidth = 0;
  int instanceColWidth = 0;
  int hostColWidth = 0;
  int userColWidth = 0;

  for (int i = 0; i < nProcs; i++) {

    const PROCMAP_info_t *dinfo = &procs[i];
    
    const char *name = dinfo->name;
    const char *instance = dinfo->instance;
    const char *user = dinfo->user;
    const string host = _stripHost(dinfo->host); 

    if ((int) strlen(name) > nameColWidth) {
      nameColWidth = (int) strlen(name);
    }

    if ((int) strlen(instance) > instanceColWidth) {
      instanceColWidth = (int) strlen(instance);
    }

    if ((int) strlen(host.c_str()) > hostColWidth) {
      hostColWidth = (int) strlen(host.c_str());
    }
    
    if ((int) strlen(user) > userColWidth) {
      userColWidth = (int) strlen(user);
    }

  } // i
  
  sprintf(_basicFormat, "%%-%d.%ds  %%-%d.%ds  %%-%d.%ds  %%-%d.%ds  %%-6.6s",
          nameColWidth, nameColWidth,
          instanceColWidth, instanceColWidth,
          hostColWidth, hostColWidth,
          userColWidth, userColWidth);

}
     
