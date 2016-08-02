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
////////////////////////////////////////////////////////////////////////
// PowerInfo2Spdb.cc
//
// PowerInfo2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2006
//
///////////////////////////////////////////////////////////////
//
// PowerInfo2Spdb reads power info from UDP and
// writes the info to an SPDB data base.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/sockutil.h>
#include <toolsa/ushmem.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <dataport/bigend.h>
#include "PowerInfo2Spdb.hh"
using namespace std;

// Constructor

PowerInfo2Spdb::PowerInfo2Spdb(int argc, char **argv)

{

  isOK = true;
  _timeLastWrite = 0;
  _powerShmem = NULL;

  // set programe name

  _progName = "PowerInfo2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // create shmem if needed

  if (_params.write_to_shmem) {
    _powerShmem =
      (power_info_packet_t *) ushm_create(_params.shmem_key,
                                          sizeof(power_info_packet_t),
                                          0666);
    if (_powerShmem == NULL) {
      cerr << "ERROR - PowerInfo2Spdb" << endl;
      cerr << "  Cannot attach shared memory for power info" << endl;
      cerr << "  key: " << _params.shmem_key << endl;
    }
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;

}

// destructor

PowerInfo2Spdb::~PowerInfo2Spdb()

{

  // detach from shared memory

  if (_powerShmem) {
    ushm_detach(_powerShmem);
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PowerInfo2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // open socket for UDP datagrams
  
  struct sockaddr_in my_addr;
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    cerr << "ERROR - PowerInfo2Spdb" << endl;
    cerr << "  Cannot open UDP socket" << endl;
    return -1;
  }

  // set socket parameters

  int val = 1;
  int valen = sizeof(val);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);
  memset(&my_addr, 0, sizeof(my_addr));

  // bind to port
  
  my_addr.sin_family = AF_INET;
  short int port = _params.udp_port;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int rcb = bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
  if (rcb < 0) {
    cerr << "ERROR - PowerInfo2Spdb::_runUdp()" << endl;
    cerr << "  Cannot bind to port: " << port  << endl;
    cerr << "  " << strerror(rcb) << endl;
    return -1;
  }

  // read UDP packets
  
  while (true) {

    PMU_auto_register("Waiting for UDP ...");

    // check for available data, waiting 1 sec before timeout
    // returns 1 on success, -1 on timeout, -2 on failure

    int iret = SKU_read_select(sock, 1000);
    
    if (iret < -1) {
      cerr << "ERROR - PowerInfo2Spdb::_runUdp()" << endl;
      cerr << "  SKU_read_select failed" << endl;
      return -1;
    }
    
    if (iret == -1) {
      // timeout
      continue;
    }
    
    // read

    PMU_auto_register("Reading UDP ...");

    power_info_packet_t info;
    int nread = recv(sock, &info, sizeof(info), 0);
    
    if (nread == sizeof(info)) {
      
      // swap bytes

      BE_to_array_32(&info, sizeof(info));

      // save to shmem if appropriate
      
      if (_powerShmem != NULL) {
        memcpy(_powerShmem, &info, sizeof(info));
      }

      // process packet

      _processInfo(info);

    } // nread

  } // while

  return -1;

}

//////////////////////////////
// process power info

int PowerInfo2Spdb::_processInfo(const power_info_packet_t &info)

{
  
  PMU_auto_register("processing UDP ...");
  
  int iret = 0;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=====>> INCOMING UDP PACKET <<=====" << endl;
    _printInfo(info, cerr);
  }
  
  // check if sufficuent time has elapsed to allow a write
  
  time_t now = time(NULL);
  double timeSinceWrite = (double) now - (double) _timeLastWrite;
  if (timeSinceWrite < _params.write_freq_secs) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Ignoring, timeSinceWrite: " << timeSinceWrite << endl;
    }
    return 0;
  }
  
  DsRadarPower power;
  power.setTime(info.utime_secs, info.utime_nano_secs);
  power.setXmitPowerH(info.xmit_power_h);
  power.setXmitPowerV(info.xmit_power_v);
  power.assemble();

  // add chunk
  
  DsSpdb spdb;
  spdb.addPutChunk(0,
                   info.utime_secs,
                   info.utime_secs + _params.write_freq_secs * 2,
                   power.getBufLen(),
                   power.getBufPtr());

  if (_params.debug) {
    cerr << "===>> Writing out power <<===" << endl;
    power.print(cerr, " ");
  }
  
  // put to SPDB
  
  if (_doPut(spdb)) {
    iret = -1;
  }
  
  // set time last written
  
  _timeLastWrite = now;

  return iret;

}


////////////////////////////////
// do put to SPDB

int PowerInfo2Spdb::_doPut(DsSpdb &spdb)
  
{
  
  if (spdb.put(_params.output_url,
               SPDB_DS_RADAR_POWER_ID,
               SPDB_DS_RADAR_POWER_LABEL)) {
    cerr << "ERROR - PowerInfo2Spdb::_doPut" << endl;
    cerr << "  Cannot put power info to: "
         << _params.output_url << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// Print info

void PowerInfo2Spdb::_printInfo(const power_info_packet_t &info,
                                ostream &out)

{
  
  out << "--------- POWER INFO -----------" << endl;
  out << "  utime_secs: " << info.utime_secs << endl;
  out << "  date/time: " << DateTime::strm(info.utime_secs) << endl;
  out << "  utime_nano_secs: " << info.utime_nano_secs << endl;
  out << "  xmit_power_h: " << info.xmit_power_h << endl;
  out << "  xmit_power_v: " << info.xmit_power_v << endl;
  out << "----------------------------------" << endl;

}

