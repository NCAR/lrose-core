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
///////////////////////////////////////////////////////
// Procinfo.cc
//
// Procmap info class
//
// Mike Dixon
//
// RAP NCAR Boulder Colorado USA
//
// Jan 1997
//
////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stream.h>

#include "Args.h"
#include "Procinfo.h"
#include <toolsa/str.h>

// Define an instance to be available throughout the framework

Procinfo *Procinfo::_instance = 0;

#define BASIC_FORMAT  "%-12.12s %-12.12s %-10.10s %-10.10s %-6.6s"

Procinfo::Procinfo()
{
  _procs = NULL;
  _nProcs = 0;
  _upTime = 0;
  _textWidget = NULL;
}

Procinfo *Procinfo::Inst()
{
  if (_instance == 0)
    _instance = new Procinfo();
  return _instance;
}

void Procinfo::registerTextWidget(Widget text)
{
  cerr << "RegisterText, text = " << text << "\n";
  _textWidget = text;
  queryMapper();
}

void Procinfo::unregisterTextWidget()
{
  cerr << "UnRegisterText\n";
  _textWidget = NULL;
}

/////////////////////////////////
// queryMapper()
//
/////////////////////////////////

void Procinfo::queryMapper()
     
{

  cerr << "QueryMapper\n";

  Procinfo *pInfo = Procinfo::Inst();

  if (pInfo->_textWidget == NULL) {
    return;
  }

  cerr << "QueryMapper22\n";

  // Initialize text position

  pInfo->initText();

  // retrieve the procs from the proc mapper
  
  pInfo->getProcInfo();
  
  // display the proc information
  
  if (pInfo->_nProcs > 0) {
    pInfo->displayProcs();
  }

  pInfo->putText2Widget();

  return;
  
}

//////////////////////////////////////////////////////
// getProcInfo : get the procs from the proc mapper
//
//////////////////////////////////////////////////////

void Procinfo::getProcInfo()
     
{
  
  cerr << "getProcInfo\n";

  PROCMAP_request_t request;
  
  // prepare the request for the proc mapper
  
  memset((char *)&request, 0, sizeof(request));
  
  request.name[0] = '\0';
  request.instance[0] = '\0';

  // send the request and process the proc mapper response
  
  if (!PMU_requestInfo(&request, &_nProcs, &_upTime, &_procs,
		       Args::Inst()->procmapHost, "")) {
    sprintf(_tmpBuf, "No procs registered with %s\n",
	    Args::Inst()->procmapHost);
    appendText();
    _nProcs = 0;
    _procs = NULL;
    return;
  } // endif - got reply from proc mapper

  if (_nProcs == 0) {
    sprintf(_tmpBuf, "No procmap reachable on %s\n",
	    Args::Inst()->procmapHost);
    appendText();
    _procs = NULL;
    return;
  }

  // sort procs
  
  qsort(_procs, _nProcs, sizeof(PROCMAP_info_t), sortInfo);

}

//////////////////////////////////////////////////////////////////
// displayProcs : display the information received from the proc
//                mapper.
//
//////////////////////////////////////////////////////////////////

void Procinfo::displayProcs()
     
{

  int i;
  
  cerr << "displayProcs\n";

  time_t now = time(NULL);
  PROCMAP_info_t *dinfo;
  
  // realtime header

  sprintf(_tmpBuf, "\nPROCS REGISTERED - %s - %s",
	  Args::Inst()->procmapHost, ctime(&now));
  appendText();
  sprintf(_tmpBuf, "_UpTime: %ld secs\n\n", _upTime);
  appendText();
  printHeader();
  
  for (i=0; i< _nProcs; i++) {
    dinfo = &_procs[i];
    printData(dinfo, now);
  }
  
  fflush(stdout);
  
  return;
  
}

/****************
 * printData()
 */

void Procinfo::printData(PROCMAP_info_t *dinfo, time_t now)

{

  sprintf(_tmpBuf, BASIC_FORMAT,
	  dinfo->name,
	  dinfo->instance,
	  stripHost(dinfo->host),
	  dinfo->user,
	  pidString(dinfo->pid));
  appendText();
  
  sprintf(_tmpBuf, " %-7.7d", (int) dinfo->max_reg_interval);
  appendText();

  sprintf(_tmpBuf, " %-10.10s", secsStr(dinfo->heartbeat_time, now));
  appendText();

  sprintf(_tmpBuf, " %-10.10s", secsStr(dinfo->start_time, now));
  appendText();

  sprintf(_tmpBuf, " %-5d", dinfo->n_reg);
  appendText();

  sprintf(_tmpBuf, " %-5d", dinfo->status);
  appendText();

  sprintf(_tmpBuf, " \"%s\"", dinfo->status_str);
  appendText();

  sprintf(_tmpBuf, "\n");
  appendText();

}
    
/****************
 * printHeader()
 */

void Procinfo::printHeader()

{

  sprintf(_tmpBuf, BASIC_FORMAT,
	  "Name", "Instance", "Host", "User", "Pid");
  appendText();

  sprintf(_tmpBuf, " %-7.7s", "Maxint");
  appendText();

  sprintf(_tmpBuf, " %-10.10s", "Heartbeat");
  appendText();

  sprintf(_tmpBuf, " %-10.10s", "_UpTime");
  appendText();

  sprintf(_tmpBuf, " %-5.5s", "Nreg ");
  appendText();

  sprintf(_tmpBuf, " %-5.5s", "Stat ");
  appendText();

  sprintf(_tmpBuf, "\n");
  appendText();


  sprintf(_tmpBuf, BASIC_FORMAT,
	  "====", "========", "====", "====", "===");
  appendText();

  sprintf(_tmpBuf, " %-7.7s", "======");
  appendText();

  sprintf(_tmpBuf, " %-10.10s", "=========");
  appendText();

  sprintf(_tmpBuf, " %-10.10s", "======");
  appendText();

  sprintf(_tmpBuf, " %-5.5s", "==== ");
  appendText();

  sprintf(_tmpBuf, " %-5.5s", "==== ");
  appendText();

  sprintf(_tmpBuf, "\n");
  appendText();

}

////////////////
// initText()
//

void Procinfo::initText()
{
  _textBuf[0] = '\0';
}
  
////////////////
// appendText()
//

void Procinfo::appendText()
{
  cerr << _tmpBuf;
  STRconcat(_textBuf, _tmpBuf, TEXTBUFLEN);
}
  
////////////////
// putText2Widget()
//

void Procinfo::putText2Widget()
{
  cerr << _textBuf;
  if (_textWidget) {
    XmTextInsert(_textWidget, 0, _textBuf);
  }
}
  
/***************
 * pidString()
 */

char *Procinfo::pidString(int pid)

{

  static char pidstr[32];

  sprintf(pidstr, "%d", pid);

  return (pidstr);

}

/****************
 * secsStr
 *
 * Converts secs to str of hr:min:sec
 */

char *Procinfo::secsStr(time_t now, si32 refTime)

{

  static char str[32];
  long tdiff;
  
  if (refTime == -1 || refTime == 0) {

    sprintf(str, "-1");

  } else if (now > refTime) {
    
    tdiff = now - refTime;

    sprintf(str, "-%s", tdiffStr(tdiff));

  } else {

    tdiff = refTime - now;

    sprintf(str, "%s", tdiffStr(tdiff));

  }

  return (str);
  
}

/**************
 * stripHost()
 *
 * Strips fully qualified data from hostname.
 * i.e. remove characters past first '.'
 */

char *Procinfo::stripHost(char *hostname)

{

  static char stripped[128];
  char *ptr;

  STRncopy(stripped, hostname, 128);

  ptr = strchr(stripped, '.');
  if (ptr != NULL) {
    *ptr = '\0';
  }

  return (stripped);

}

/***********************************
 * sortInfo()
 *
 * qsort compare routine
 */

int Procinfo::sortInfo(const void *p1, const void *p2)

{

  PROCMAP_info_t *i1 = (PROCMAP_info_t *) p1;
  PROCMAP_info_t *i2 = (PROCMAP_info_t *) p2;
  int ret;
  
  ret = strcmp(i1->host, i2->host);
  if (0 == ret) {
    ret = strcmp(i1->name, i2->name);
    if (0 == ret) {
      ret = strcmp(i1->instance, i2->instance);
    }
  }
  return ret;

}

/****************
 * tdiffStr
 *
 * Converts time diff to string
 */

char *Procinfo::tdiffStr(long tdiff)

{

  static char str[32];
  
  tdiff = abs(tdiff);
  
  if (tdiff > 86399) {
    sprintf(str, "%.3g d", (double) tdiff / 86400.0);
  } else {
    sprintf(str, "%ld:%ld:%ld",
	    tdiff/3600,
	    (tdiff%3600)/60,
	    tdiff%60);
  }

  return (str);
  
}

