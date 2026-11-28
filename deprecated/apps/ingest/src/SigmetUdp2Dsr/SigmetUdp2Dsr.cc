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
// SigmetUdp2Dsr.cc
//
// SigmetUdp2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// SigmetUdp2Dsr reads raw Sigmet IQ time-series data, computes the
// moments and writes the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/ucopyright.h>
#include <toolsa/uusleep.h>
#include "SigmetUdp2Dsr.hh"
#include "OutputFmq.hh"
using namespace std;
// Constructor

SigmetUdp2Dsr::SigmetUdp2Dsr(int argc, char **argv)

{

  _udp = NULL;
  _fmq = NULL;

  _volNum = 0;
  _nBeamsThisVol = 0;
  _prevAz = -1.0;
  _prevEl = -180;
  _volMinEl = 180.0;
  _volMaxEl = -180.0;

  isOK = true;

  // set programe name

  _progName = "SigmetUdp2Dsr";
  ucopyright((char *) _progName.c_str());

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
  }

  // create input udp object

  _udp = new InputUdp(_progName, _params);

  // create the output queue
  
  _fmq = new OutputFmq(_progName, _params);
  if (!_fmq->isOK) {
    isOK = FALSE;
    return;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

SigmetUdp2Dsr::~SigmetUdp2Dsr()

{

  if (_udp) {
    delete _udp;
  }

  if (_fmq) {
    delete _fmq;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SigmetUdp2Dsr::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");

  // loop forever, opening the udp object and then processing data

  while (true) {

    if (_udp->openUdp() == 0) {
      if (_processUdp()) {
	iret = -1;
      }
      _udp->closeUdp();
    }

    umsleep(1000);

  }

  return iret;

}

//////////////////////////////////////////////////
// process data from UDP

int SigmetUdp2Dsr::_processUdp()
{

  while (true) {
    
    if (_udp->readPacket() == 0) {
      
      bool acceptPacket = true;

      if (_params.check_task_name &&
          strcmp(_udp->getVolHdr().taskName, _params.task_name)) {
        acceptPacket = false;
      }

      if (acceptPacket) { 
      
        // if a volume header has been read, write volume
        // and field params to the FMQ.
        
        if (_udp->volHdrIsNew() )
          _fmq->writeParams(_udp->getVolHdr());
        
        // if we have a new beam, write beam to FMQ
        
        if (_udp->beamIsNew())
          _processBeam();

        //ipmet: testar se fim do vol ativo (quando pType=0x1000) 

        if( _udp->EndVol() )
          _fmq->writeEndOfVolume(_volNum, _udp->getBeam().getTime());

      } // if (acceptPacket)

    } // if (_udp->readPacket() == 0)

  } // while()

  return 0;

}

/////////////////////////////////////
// process a new beam


int SigmetUdp2Dsr::_processBeam()

{

  int iret = 0;

  const Beam &beam = _udp->getBeam();
  time_t btime = beam.getTime();
  double az = beam.getAz();
  double el = beam.getEl();
  int nGates = beam.getNGates();
  double prf = beam.getPrf();

  if (el < _prevEl && el < _volMinEl) {
    _volMinEl = el;
  }
  if (el > _prevEl && el > _volMaxEl) {
    _volMaxEl = el;
  }

  double deltaAz = 0;
  if (_prevAz >= 0) {
    deltaAz = fabs(az - _prevAz);
    if (deltaAz > 180) {
      deltaAz = fabs(deltaAz - 360.0);
    }
  }

  // search for end of vol
  
  bool endOfVol = FALSE;


  if (_nBeamsThisVol >= _params.min_beams_per_vol) {
    if (_params.volume_starts_at_bottom) {
      double deltaEl = _volMaxEl - el;

      if (deltaEl > _params.el_change_for_end_of_vol) {
	endOfVol = TRUE;
      }
    } else {
      double deltaEl = el - _volMinEl;
      if (deltaEl > _params.el_change_for_end_of_vol) {
	endOfVol = TRUE;
      }
    }

  }
   //ipmet: a variavel endOfVol fica true no inicio da tarefa (startOfVol)
   if (endOfVol)
   {
      cout << "End_of_vol: " << _params.set_end_of_vol_flag << endl;    
      if (_params.set_end_of_vol_flag)
      {

//ipmet:linha abaixo comentada, ela eh executado na _proccessUdp() 
//         if (_fmq->writeEndOfVolume(_volNum, btime)) iret = -1;

         if (_fmq->writeStartOfVolume(_volNum + 1, btime)) 
         	iret = -1;
      }
    
      _volNum++;
      _volMinEl = 180.0;
      _volMaxEl = -180.0;
      _nBeamsThisVol = 0;
    
   }
  
  // put the beam
  
  if (_fmq->writeBeam(beam, _volNum)) {
    iret = -1;
  }
  
  if (_params.debug) {
    fprintf(stderr,
	    "Vol,el,az,deltaAz,ngates,prf,time: ");
    DateTime beamTime(btime);
    fprintf(stdout,
	    "%3d %6.2f %6.2f %6.2f %4d %5g %s\n",
	    _volNum, el, az, deltaAz, nGates, prf, DateTime::str(btime).c_str());
    if (endOfVol) {
      fprintf(stderr, "-----> End of volume <-----\n");
    }
  }
  
  _prevAz = az;
  _prevEl = el;
  _nBeamsThisVol++;

  return iret;
  
}

