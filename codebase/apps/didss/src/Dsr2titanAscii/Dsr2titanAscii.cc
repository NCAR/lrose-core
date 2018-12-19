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
// Dsr2titanAscii.cc
//
// Dsr2titanAscii object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////////////
//
// Dsr2titanAscii prints out info from a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#include <Fmq/DsRadarQueue.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>


#include "Dsr2titanAscii.hh"
using namespace std;

// Constructor

Dsr2titanAscii::Dsr2titanAscii(int argc, char **argv)
{



  // set programe name

  _progName = "Dsr2titanAscii";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
  }

  _ofp = NULL;

  if (ta_makedir_recurse( _params.output_dir )){
    cerr << "Failed to create directory " << _params.output_dir << endl;
    exit(-1);
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Dsr2titanAscii::~Dsr2titanAscii()

{

  // unregister process

  PMU_auto_unregister();

  return;

}

//////////////////////////////////////////////////
// Run

int Dsr2titanAscii::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "Dsr2titanAscii::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.fmq_url << endl;
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int Dsr2titanAscii::_run ()
{



  // register with procmap
  
  PMU_auto_register("Run");

  // Instantiate and initialize the DsRadar queue and message
  
  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;
  DsFmq::openPosition open_pos;
  
  if (_params.seek_to_start_of_input) {
    open_pos = DsFmq::START;
  } else {
    open_pos = DsFmq::END;
  }

  int iret = radarQueue.init(_params.fmq_url, _progName.c_str(),
			     _params.debug,
			     DsFmq::READ_ONLY, open_pos);

  if (iret) {
    cerr << "ERROR - Dsr2titanAscii::_run()" << endl;
    cerr << "  Could not initialize radar queue: "
	 << _params.fmq_url << endl;
    return -1;
  }

  int contents;
  while (true) { 

    PMU_auto_register("Reading radar queue");
    
    // get a message from the radar queue

    int radarQueueRetVal = radarQueue.getDsMsg( radarMsg, &contents );

    if (radarQueueRetVal) {
      
      cerr << "radarQueue:getDsBeam() failed, returned "
	   << radarQueueRetVal << endl;

      sleep (1);

    } else { 

      _processMessage(contents, radarMsg);
      
    } // if (radarQueue ...

  } // while

  return 0; // In fact never get here.

}

////////////////////////////////////////////
// print beam information

void Dsr2titanAscii::_processMessage(int contents,
				     const DsRadarMsg &radarMsg)
  
{


  if (contents & DsRadarMsg::RADAR_FLAGS) {

    if (_params.debug) cerr << "Radar flags found" << endl;

    const DsRadarFlags& flags = radarMsg.getRadarFlags();

    _openOutputFile(flags.time);
    if (_ofp != NULL){

    fprintf(_ofp,"RADAR_FLAGS\n{ ");

    if (flags.startOfVolume) {
      fprintf(_ofp, "TRUE ");
    } else {
      fprintf(_ofp, "FALSE ");
    }

    if (flags.startOfTilt) {
      fprintf(_ofp, "TRUE ");
    } else {
      fprintf(_ofp, "FALSE ");
    }

    if (flags.endOfVolume) {
      fprintf(_ofp, "TRUE ");
    } else {
      fprintf(_ofp, "FALSE ");
    }

    if (flags.endOfTilt) {
      fprintf(_ofp, "TRUE ");
    } else {
      fprintf(_ofp, "FALSE ");
    }

    fprintf(_ofp,"}\n");

    if (flags.endOfVolume) {
      _closeOutputFile();
    }
    }

  } // End of radar flags


  if (contents & DsRadarMsg::RADAR_PARAMS) {

    if (_params.debug) cerr << "Radar params found" << endl;

    const DsRadarParams &radarParams = radarMsg.getRadarParams();

    if (_ofp != NULL){

      fprintf(_ofp, "RADAR_PARAMS\n");
    }
  }


  if (contents & DsRadarMsg::FIELD_PARAMS) {

    if (_params.debug) cerr << "Field params found" << endl;
	
    const vector<DsFieldParams*> &fieldParams = radarMsg.getFieldParams();

    if ((_ofp != NULL) && (fieldParams.size() > 0)) {

      fprintf(_ofp, "FIELD_PARAMS %d {\n", (int)fieldParams.size());

      for (size_t fieldNum = 0; fieldNum < fieldParams.size(); fieldNum++) {
	fprintf(_ofp, "{ %s %s %lf }\n",
		fieldParams[fieldNum]->name.c_str(),
		fieldParams[fieldNum]->units.c_str(),
		(double) fieldParams[fieldNum]->missingDataValue);
      }
      fprintf(_ofp, "}\n");
    }
  }


  if (contents & DsRadarMsg::RADAR_BEAM) {

    if (_params.debug) cerr << "Radar beam found" << endl;

    const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();

    _openOutputFile(radarBeam.dataTime);

    if (_ofp != NULL){
      fprintf(_ofp,"RADAR_BEAM\n");
    }
  }




  if (contents & DsRadarMsg::RADAR_CALIB) {

    if (_params.debug) cerr << "Radar calibration found" << endl;

    if (_ofp != NULL){
      fprintf(_ofp,"# RADAR_CALIB structure encountered but not converted\n");
    }
  }

  return;

} 

////////////////////////////////////////////
// Close the output file, set pointer to NULL.

void Dsr2titanAscii::_closeOutputFile(){

  if (_ofp != NULL){
    fclose(_ofp); _ofp = NULL;
  }

  return;
}

////////////////////////////////////////////
// Open output file using timestamp - only if file is
// not currently open already and the time is not 0L.

void Dsr2titanAscii::_openOutputFile(time_t msgTime){

  if (msgTime == 0L) return;

  if (_ofp != NULL) return;

  date_time_t T;
  T.unix_time = msgTime;
  uconvert_from_utime( &T );

  char outFileName[MAX_PATH_LEN];
  sprintf(outFileName,"%s/Titan_%04d%02d%02d_%02d%02d%02d.ascii",
	  _params.output_dir, T.year, T.month, T.day,
	  T.hour, T.min, T.sec);

  _ofp = fopen(outFileName, "w");
  if (_ofp == NULL){
    cerr << "Failed to create file " << outFileName << endl;
    exit(-1);
  }

  return;
}
