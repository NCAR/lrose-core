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
// Era5Nc2Mdv.hh
//
// Era5Nc2Mdv object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2023
//
///////////////////////////////////////////////////////////////

#ifndef Era5Nc2Mdv_H
#define Era5Nc2Mdv_H

#include <string>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <tdrp/tdrp.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "HtInterp.hh"
#include "PresInterp.hh"
#include "Era5Data.hh"
#include "WRFGrid.hh"

using namespace std;

class Era5Data;
class InputPath;
class ItfaIndices;

class Era5Nc2Mdv {
  
public:

  // constructor

  Era5Nc2Mdv();
  bool init(int argc, char **argv);

  // destructor
  
  ~Era5Nc2Mdv();

  // run 

  bool Run();

private:

  /////////////////////
  // Private members //
  /////////////////////

  // Params::afield_name_map_t *_field_name_map;

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  PresInterp _presInterp;

  MdvxProj _outputProj;
  bool _rotateOutputUV;
  
  /////////////////////
  // Private methods //
  /////////////////////

  bool _checkParams();
  bool _initVertInterp();
  
  bool _run(InputPath &input_path);

  bool _processInData(Era5Data &inData);

  bool _acceptLeadTime(int lead_time);
  
  int _processForecast(Era5Data &inData,
		       time_t gen_time,
		       int lead_time,
		       time_t forecast_time);

  void _loadCrossOutputFields(Era5Data &inData,
			      DsMdvx &mdvx);
  
  void _loadEdgeOutputFields(Era5Data &inData,
			    DsMdvx &mdvx);
  void _loadUEdgeOutputFields(Era5Data &inData,
			    DsMdvx &mdvx);
  void _loadVEdgeOutputFields(Era5Data &inData,
			    DsMdvx &mdvx);
  
  void _interp3dField(Era5Data &inFile,
		      const char *field_name,
		      fl32 ***field_data,
		      DsMdvx &mdvx,
		      int planeOffset,
		      int nPointsPlane,
		      fl32 missingDataVal,
		      const WRFGrid &mGrid,
		      double mult = 1.0);

 void _interp3dField(Era5Data &inData,
		     // const  Params::output_field_name_t &field_name_enum,
		     fl32 ***field_data,
		     DsMdvx &mdvx,
		     const int planeOffset,
		     const int nPointsPlane,
		     const fl32 missingDataVal,
		     const WRFGrid &mGrid,
		     const double factor  = 1.0 );
 
  void _interp2dField(Era5Data &inFile,
		      const char *field_name,
		      fl32 **field_data,
		      DsMdvx &mdvx,
		      int planeOffset,
		      fl32 missingDataVal,
		      const WRFGrid &mGrid,
		      double mult = 1.0);

  void _interp2dField(Era5Data &inData,
		      // const  Params::output_field_name_t &field_name_enum,
		      fl32 **field_data,
		      DsMdvx &mdvx,
		      int planeOffset,
		      fl32 missingDataVal,
		      const WRFGrid &mGrid,
		      double factor  = 1.0 );

  void _initFieldNameMap();


};

#endif
