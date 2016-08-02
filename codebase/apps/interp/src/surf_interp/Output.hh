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
// Output.hh
//
// Output class - handles the output to MDV files
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////

#ifndef Output_HH
#define Output_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh> 
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>   
#include <toolsa/umisc.h>
#include "Params.hh"
using namespace std;


class Output {
  
public:
  
  // constructor
  // Sets up a master header and makes room for 
  // N field headers in a handle.
  // Specific to the needs of the surface interpolation.
  //

  Output(date_time_t DataTime,
		 time_t DataDuration,
		 int NumFields, int nx, int ny, int nz,
		 float lon, float lat, float alt,
		 char *SetInfo, char *SetName, char *SetSource,
		 tdrp_bool_t flat);

  // destructor
  ~Output();

  // AddField
  // Sets up field header m where 0 <= m < N
  void AddField(char *var_name,char *short_var_name, char *var_units,
			int Field, int NumFields,
			float *data, float bad, float missing,
			float dx, float dy, float dz,
			float x0, float y0, float z0,
			int FieldCode);
  // Write the file.
  // Returns 0 on success, -1 on failure

  int write(char *output_url);


protected:
  
private:
  

  // Data.
  DsMdvx mdvx;

};

#endif



















