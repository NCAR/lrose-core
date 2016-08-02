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
// FmqExecute.cc
//
// FmqExecute object -- reads strings from a DsFmq and executes them.
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2001
//
///////////////////////////////////////////////////////////////

#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <Fmq/DsFmq.hh>
#include <rapformats/DsRadarMsg.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include "FmqExecute.hh"
using namespace std;

// Constructor

FmqExecute::FmqExecute(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "FmqExecute";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

FmqExecute::~FmqExecute()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int FmqExecute::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // while (true) {
  _run();
  //   cerr << "FmqExecute::Run:" << endl;
  //   cerr << "  Trying to contact server at url: "
  //        << _params.input_url << endl;
  // }

  return (0);

}

//////////////////////////////////////////////////
// _run

int FmqExecute::_run ()
{

  // register with procmap
  
  PMU_auto_register("_run");

  // open input and output FMQ's

  DsFmq input;
  char * buf;
  char message[200];
  MsgLog msgLog(_progName);

  if (input.initReadBlocking(_params.input_url,
                             _progName.c_str(),
                             _params.debug >= Params::DEBUG_VERBOSE,
                             DsFmq::END,
                             _params.msecs_sleep_blocking,
                             &msgLog)) {
    cerr << "ERROR - FmqExecute::_run" << endl;
    cerr << "  Cannot open input FMQ at url: " << _params.input_url << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened input from URL: " << _params.input_url << endl;
  }
  
  time_t start = time(NULL);
  // time_t end = start + _params.monitor_interval;

  int count = 0;
  int nbytesCompressed = 0;
  int nbytesUncompressed = 0;
  int nMessages = 0;
  
  while (true) {

    PMU_auto_register("_run: read loop");
    
    if (input.readMsgBlocking()) {
      cerr << "ERROR - FmqExecute::_run" << endl;
      cerr << "  Cannot read from FMQ at url: " << _params.input_url << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "    Read message, len: " << setw(8) << input.getMsgLen()
           << ", type: " << setw(8) << input.getMsgType()
           << ", subtype: " << setw(8) << input.getMsgSubtype()
           << endl;
    }
    nMessages++;
    nbytesUncompressed += input.getMsgLen();
    nbytesCompressed += input.getCompressedLen();

    int len = input.getMsgLen();
    buf = (char *) input.getMsg();
    strncpy(message, buf, len);
    message[len] = '\0';

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got DsFmq Entry (Id: " << input.getMsgId()
           << "): \"" << message << "\"" << endl;
    }

    system(message);

/*
    time_t now = time(NULL);

    if (now >= end) {
      
      double dtime = now - start;
      double rateCompressed = (double) nbytesCompressed / dtime;
      double rateUncompressed = (double) nbytesUncompressed / dtime;

      if ((count % 40) == 0) {
        
        cout << setw(19) << ""
             << setw(7) << "Dtime"
             << setw(8) << "nMess"
             << setw(10) << "nBytes"
             << setw(10) << "nBytes"
             << setw(10) << "Rate b/s"
             << setw(10) << "Rate b/s" << endl;

        cout << setw(19) << ""
             << setw(7) << "secs"
             << setw(8) << ""
             << setw(10) << "uncomp"
             << setw(10) << "comp"
             << setw(10) << "uncomp"
             << setw(10) << "comp" << endl;
        
        cout << setw(19) << ""
             << setw(7) << "====="
             << setw(8) << "====="
             << setw(10) << "======"
             << setw(10) << "======"
             << setw(10) << "========"
             << setw(10) << "========" << endl;
        

      }

      cout << setw(19) << utimstr(now)
           << setw(7) << (int) dtime
           << setw(8) << nMessages
           << setw(10) << nbytesUncompressed
           << setw(10) << nbytesCompressed
           << setw(10) << (int) rateUncompressed
           << setw(10) << (int) rateCompressed << endl;

      start = now;
      end = start + _params.monitor_interval;
      
      nbytesCompressed = 0;
      nbytesUncompressed = 0;
      nMessages = 0;
      count++;
  
      // in debug more print out radar beam
      
      if (_params.debug) {
        DsMessage dsMsg;
        if (dsMsg.disassemble(input.getMsg(), input.getMsgLen()) == 0) {
          DsMsgPart *part;
          if ((part = dsMsg.getPartByType(DsRadarMsg::RADAR_BEAM)) != NULL) {
            DsBeamHdr_t bhdr;
            memcpy(&bhdr, part->getBuf(), sizeof(DsBeamHdr_t));
            BE_to_DsBeamHdr(&bhdr);
            cerr << "Radar beam: "
                 << " Time: " << utimstr(bhdr.time)
                 << " Elev: " << bhdr.elevation
                 << " Az: " << bhdr.azimuth << endl;
          }
        }
      }

    }
*/

  } // while
  
  return (0);

}

