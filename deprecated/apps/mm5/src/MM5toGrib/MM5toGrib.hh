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
// MM5toGrib.hh
//
// MM5toGrib object
//
// Carl Drews, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 30, 2004
//
///////////////////////////////////////////////////////////////

#ifndef MM5toGrib_H
#define MM5toGrib_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <string>

#include <mm5/MM5Grid.hh>
#include <mm5/SigmaInterp.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

class MM5Data;
class InputPath;
class ItfaIndices;

class MM5toGrib {
  
public:

  // constructor

  MM5toGrib (int argc, char **argv);

  // destructor
  
  ~MM5toGrib();

  // run 

  int Run();

  // data members

  int OK;

protected:

  /**
   *	Write out GRIB file representing the MDV data.
   *	@param outputFile is the full path of the output GRIB file
   */
  virtual void writeGrib(DsMdvx &inputMdv, string outputFile);

private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  SigmaInterp _sigmaInterp;

  int _checkParams();
  
  int _run(InputPath &input_path);

  int _processInData(MM5Data &inData);

  bool _acceptLeadTime(int lead_time);
  
  int _processForecast(MM5Data &inData,
		       time_t gen_time,
		       int lead_time,
		       time_t forecast_time);

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

  void _trimTurbField(DsMdvx &mdvx, int turbFieldNum);

};

#endif
