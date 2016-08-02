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
// OutputMdv.hh
//
// OutputMdv class - handles debug output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////

#ifndef OutputMdv_HH
#define OutputMdv_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <rapformats/DsRadarParams.hh>

#include "Params.hh"
using namespace std;

typedef enum {
  CART_OUTPUT_GRID, PPI_OUTPUT_GRID, POLAR_OUTPUT_GRID, RHI_OUTPUT_GRID
} output_grid_geom_t;

class OutputMdv {
  
public:
  
  // constructor
  
  OutputMdv(const string &prog_name,
	    const Params &params,
	    output_grid_geom_t geom_type);
  
  // destructor
  
  virtual ~OutputMdv();
  
  // set the master header
  
  void setMasterHeader(time_t start_time,
		       time_t mid_time,
		       time_t end_time,
		       int nx,
		       int ny,
		       int nz,
		       double radar_lat,
		       double radar_lon,
		       double radar_alt,
		       const char *radar_name);

  // addField()
  
  void addField(const char *field_name,
		const char *units,
		bool isDbz,
		int nx,
		double dx,
		double minx,
		int ny,
		double dy,
		double miny,
		int nz,
		double dz,
		double minz,
		const vector<double> &vlevel_array,
		double radar_lat,
		double radar_lon,
		double radar_alt,
		int input_byte_width,
		double input_scale,
		double input_bias,
		Mdvx::encoding_type_t encoding,
		Mdvx::compression_type_t compression,
		const fl32 *data);
  
  // add the radarParams as a chunk

  void addChunks(const DsRadarParams &rparams,
                 const vector<double> &elevArray);

  // write out merged volume

  int writeVol(const char *url);

protected:
  
private:
  
  const string _progName;
  const Params &_params;
  DsMdvx _mdvx;
  output_grid_geom_t _geomType;

};

#endif

