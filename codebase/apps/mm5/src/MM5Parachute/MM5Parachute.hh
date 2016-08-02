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
/////////////////////////////////////////////////////////////
// MM5Parachute.hh
//
// MM5Parachute object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef MM5Parachute_H
#define MM5Parachute_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <string>
#include <fstream>

#include <mm5/MM5Grid.hh>
#include <mm5/SigmaInterp.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

class MM5Data;
class InputPath;
class ItfaIndices;

class MM5Parachute {
  
public:

  // constructor

  MM5Parachute (int argc, char **argv);

  // destructor
  
  ~MM5Parachute();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  SigmaInterp _sigmaInterp;

  int _checkParams();
  
  int _run(InputPath &input_path);

  int _processInData(MM5Data &inData,
		     time_t gen_time);

  bool _acceptLeadTime(int lead_time);
  
  int _processForecast(MM5Data &inData,
		       time_t gen_time,
		       int lead_time,
		       time_t forecast_time);

  void _loadCrossOutputFields(MM5Data &inData, MM5Grid &mGrid,
			      const ItfaIndices *itfa,
			      DsMdvx &mdvx);
  
  void _loadDotOutputFields(MM5Data &inData,
			    DsMdvx &mdvx);
  
  void _interp3dField(MM5Data &inFile,
		      const char *field_name,
		      fl32 ***field_data,
		      DsMdvx &mdvx,
		      int planeOffset,
		      int nPointsPlane,
		      fl32 missingDataVal,
		      const MM5Grid &mGrid,
		      double mult = 1.0);
  
  void _interp2dField(MM5Data &inFile,
		      const char *field_name,
		      fl32 **field_data,
		      DsMdvx &mdvx,
		      int planeOffset,
		      fl32 missingDataVal,
		      const MM5Grid &mGrid,
		      double mult = 1.0);

  bool _needItfa();
  bool _needIcing();

  void _computeItfa(MM5Data &inData, ItfaIndices *indices);

  void _trimTurbField(DsMdvx &mdvx);

  // Parachute calculations
  double _dropSpeed;             // during stable descent
  double _stabilizationHeight;   // transition between deployment and stable descent
  double _heightStep;            // height interval during stable descent
  int _numHeightLevels;          // how many regular height levels
  vector<double> _heightLevels;  // altitude in meters

  void calculateHeightLevels();

  void calculateParachute(MM5Data &inData, MM5Grid &mGrid);
  void logTrajectory(ofstream &logFile,
    double timeFromImpact,
    double heightLevel, double latitude, double longitude,
    double xCumDrift, double yCumDrift,
    double uWind, double vWind,
    bool firstCall = false);

  int getLatIndex(MM5Data &inData, double latitude);
  int getLonIndex(MM5Data &inData, double longitude);

  // retrieve wind data from MM5 structure
  double getUWind(MM5Data &inData, MM5Grid &mGrid,
    double lat, double lon, int heightIndex);
  double getVWind(MM5Data &inData, MM5Grid &mGrid,
    double lat, double lon, int heightIndex);

};

#endif

