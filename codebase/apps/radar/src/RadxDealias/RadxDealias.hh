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
/*
 *   Module: RadxDealias.hh
 *
 *   Author: Sue Dettling
 *
 *   Date:   10/5/01
 *
 *   Description: Class RadxDealias handles dealiasing DsRadarBeams using 
 *                the Curtis James 4DD ( Four Dimensional Dealiser ) algorithm. 
 *                DsRadarBeams are read from an fmq, reformatted and collected
 *                and stored in  RSL (TRMM Radar Software Library) structs.
 *                Once a complete volume of beams has been collected, 
 *                the volume is processed by 4DD. ( The C functions which make up 
 *                the 4DD algorithm have been put in the FourDD class). 
 *                Finally, the volume is written beam by beam to an output fmq 
 *                in DsRadarBeam format. 
 *
 */

#ifndef RADXD_HH
#define RADXD_HH

#include <string>
#include <vector>
#include <sys/stat.h>
#include "Args.hh"
#include "Params.hh"
//#include <rapformats/DsRadarMsg.hh>
//#include <Fmq/DsRadarQueue.hh>
#include <Radx/RadxVol.hh>
#include "Rsl.hh"
//#include "Dsr2Radar.hh"
#include "FourDD.hh"
using namespace std;

class RadxDealias {
  
public:

  RadxDealias (int argc, char **argv);

  ~RadxDealias();

  int Run();

  bool isOK;
  
private:

  int _run();

  int _useCommandLineStartEndTimes();
  int _useCommandLineFileList();

  //
  //  Read message from fmq
  //
  /*  int _readMsg(DsRadarQueue &radarQueue,
	       DsRadarMsg &radarMsg,
	       bool &end_of_vol,
	       int &contents);
  */
  void _readFile(const string &filePath, RadxVol &vol);


  //
  // Dealias _currRadarVol if possible
  // 
  void _processVol(Volume *_prevVelVol, Volume *_currVelVol,
		   Volume *currDbzVol, time_t volTime);
  //
  // Write beams in _currRadarVol to fmq
  //
  void _writeVol(RadxVol &vol);

  void statusReport(int nError, int nGood);


  //
  // Reset the Dsr2Radar _currRadarVol and _prevRadarVol in
  // preparation for start of new volume.
  //
  void _reset();

  inline bool file_exists (const std::string& name) {
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0); 
  }

  //
  // Data members
  //
  string _progName;
  
  char *_paramsPath;
  
  Args _args;
  
  Params _params;


  //  Volume *_extractVel(const RadxVol &radxVol); // , Volume *currVelVol);
  Volume *_extractFieldData(const RadxVol &radxVol, string fieldName); // , Volume *currDbzVol);
  
  void _insertFieldData(RadxVol *radxVol, string fieldName, Volume *volume); 

  //
  // Reformatter and holder of beams for current radar vol
  //
  //Dsr2Radar *_currRadarVol;
  
  //
  // Holder of beams for previous radar vol
  //
  //Dsr2Radar *_prevRadarVol;
  
  //
  // Dealiser methods
  //
  FourDD    *_fourDD;

  //
  // Copyright inforamtion for the 4DD algorithm
  //
  void jamesCopyright();
  
};

#endif




