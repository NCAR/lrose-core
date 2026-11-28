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

#include <Fmq/DsRadarQueue.hh>
#include <toolsa/MsgLog.hh>
#include <dataport/port_types.h>
#include <vector>
#include <toolsa/umisc.h>

#include "Params.hh"

int main(int argc, char *argv[]){


  // Get TDRP params.

  Params *tdrp = new  Params();

  if (tdrp->loadFromArgs(argc, argv, NULL, NULL)){
    cerr << "Specify params file with -params option." << endl;
    exit(-1);
  }


  date_time_t dTime;
  if (6 != sscanf(tdrp->dataTime, "%d/%d/%d %d:%d:%d",
		  &dTime.year, &dTime.month, &dTime.day,
		  &dTime.hour, &dTime.min, &dTime.sec)){
    cerr << "Could not decode " << tdrp->dataTime << endl;
    return -1;
  }

  uconvert_to_utime( &dTime );


  // Set up data to be all missing.

  fl32 *beamData = (fl32 *)malloc(sizeof(fl32) * tdrp->geom.nGates * tdrp->geom.nFields );
  if (beamData == NULL){
    cerr << "Malloc failed!" << endl;
    return -1;
  }

  for (int ifld=0; ifld < tdrp->geom.nFields; ifld++){
    for (int ig=0; ig < tdrp->geom.nGates; ig++){
      beamData[ig * ifld + ifld] = -999; // Doesn't really matter as these values never make it to output
    }
  }

  MsgLog msgLog;
  msgLog.enableMsg( DEBUG, false );
  msgLog.enableMsg( INFO, false );

  if ( msgLog.setOutputDir( tdrp->fmq.fmqMsgLogDir ) != 0 ) {
    cerr << "Failed to set up message log to directory ";
    cerr << tdrp->fmq.fmqMsgLogDir;
    cerr << endl;
    exit(-1);
  }

  DsRadarQueue  radarQueue;

  if( radarQueue.init( tdrp->fmq.fmqUrl,
                        "clearRadarFmq",
                        false,
                        DsFmq::READ_WRITE, DsFmq::END,
                        tdrp->fmq.fmqCompress,
                        tdrp->fmq.fmqNumSlots,
                        tdrp->fmq.fmqSizeBytes, 1000,
                        &msgLog )) {
    cerr << "Could not initialize fmq " << tdrp->fmq.fmqUrl << endl;
    exit(-1);
  }

  //
  // Use blocking writes, if desired.
  //
  if ( tdrp->fmq.fmqBlockingWrites ){
    if ( radarQueue.setBlockingWrite()){
      cerr << "Failed to set blocking write!" << endl;
      exit(-1);
    }
  }

 DsRadarMsg  radarMsg;

 double az = tdrp->geom.startAz;

 while (az <=  tdrp->geom.endAz){

   DsRadarBeam& radarBeam = radarMsg.getRadarBeam();

   radarBeam.dataTime   = dTime.unix_time;

   radarBeam.volumeNum  = 0;
   radarBeam.tiltNum    = 0;
   radarBeam.azimuth    = az;
   radarBeam.elevation  = tdrp->geom.elevation;
   radarBeam.targetElev = radarBeam.elevation;

   radarBeam.loadData( beamData, tdrp->geom.nGates * tdrp->geom.nFields * sizeof(fl32), sizeof(fl32) );

   int content = DsRadarMsg::RADAR_BEAM;
   if( radarQueue.putDsMsg(radarMsg, content ) != 0 ) {
     cerr << "Failed to send beam data." << endl;
     exit(-1);
   }
   umsleep( tdrp->fmq.microSecBetweenBeams );
   az +=  tdrp->geom.delAz;
 }


 free(beamData);

 return 0;

}

