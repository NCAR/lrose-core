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
// Server.cc
//
// File Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
// Jan 2005 Modified by Sue Dettling for handling Edr data type. 
//
///////////////////////////////////////////////////////////////

#include <cstdio>  
#include <toolsa/toolsa_macros.h>
#include "Server.hh"
#include "Params.hh"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

Server::Server(const string &prog_name,
	       Params *initialParams)
  : DsSymprodServer(prog_name,
		 initialParams->instance,
		 (void*)(initialParams),
		 initialParams->port,
		 initialParams->qmax,
		 initialParams->max_clients,
		 initialParams->no_threads,
		 initialParams->debug >= Params::DEBUG_NORM,
		 initialParams->debug >= Params::DEBUG_VERBOSE)
{
}

//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int
Server::loadLocalParams( const string &paramFile, void **serverParams)

{
   Params  *localParams;
   char   **tdrpOverrideList = NULL;
   bool     expandEnvVars = true;

   const char *routine_name = "_allocLocalParams";

   if (_isDebug) {
     cerr << "Loading new params from file: " << paramFile << endl;
   }

   localParams = new Params( *((Params*)_initialParams) );
   if ( localParams->load( (char*)paramFile.c_str(),
                           tdrpOverrideList,
                           expandEnvVars,
                           _isVerbose ) != 0 ) {
      cerr << "ERROR - " << _executableName << "::" << routine_name << endl
           << "Cannot load parameter file: " << paramFile << endl;
      delete localParams;
      return( -1 );
   }

   if (_isVerbose) {
     localParams->print(stderr, PRINT_SHORT);
   }

   *serverParams = (void*)localParams;
   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
                             const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
  
{
  
  // check prod_id

  if (prod_id != SPDB_EDR_POINT_ID && prod_id != SPDB_EDR_VER2_POINT_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_EDR_POINT_ID: " << SPDB_EDR_POINT_ID <<
	  " or SPDB_EDR_VER2_POINT_ID : " << SPDB_EDR_VER2_POINT_ID << endl;
    return -1;
  }

  bool newFormat = false;
  if (prod_id == SPDB_EDR_VER2_POINT_ID)
    newFormat = TRUE;

  Params *serverParams = (Params*) params;

  if (spdb_len < (int) sizeof(Edr::edr_t)) {
    return 0;
  }

  Edr::edr_t edrRep;
  if(newFormat) {
    EDR edr;
    edr.disassemble(spdb_data, spdb_len); 
    EDR::Edr_t EDRrep = edr.getRep();
    edrRep.lat = EDRrep.lat;
    edrRep.lon = EDRrep.lon;
    edrRep.alt = EDRrep.alt;
    edrRep.qcConf = EDRrep.qcConf;
    edrRep.edrAve = EDRrep.edrAve;
    edrRep.edrPeak = EDRrep.edrPeak;
    edrRep.sat = EDRrep.sat;
    edrRep.wspd = EDRrep.wspd;
    edrRep.wdir = EDRrep.wdir;
    sprintf(edrRep.flightNum, "%s", EDRrep.flightNum);
  } else {
    Edr edr;
    
    edr.disassemble(spdb_data, spdb_len); 
    edrRep = edr.getRep();
  }
 
  // check clipping

  if (serverParams->apply_flight_level_limits) {
    double fl = edrRep.alt / 100.0;
    if (fl < serverParams->min_flight_level ||
	fl > serverParams->max_flight_level) {
      return -1;
    }
  }

  if (serverParams->apply_lat_lon_limits) {
    if (edrRep.lat < serverParams->min_lat ||
	edrRep.lat > serverParams->max_lat) {
      return -1;
    }
    if (edrRep.lon < serverParams->min_lon ||
	edrRep.lon > serverParams->max_lon) {
      return -1;
    }
  }
  if (serverParams->apply_qcConf_filtering) {
    if (edrRep.qcConf != 1) {
      return -1;
    }
  }


  // create Symprod object

  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
               chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Edr2Symprod"); 
  
  // icon

  if (serverParams->plot_icon) {
    _addIcon(serverParams, edrRep, prod);
  }

  // text fields
  
  _addText(serverParams, edrRep, prod);

  // copy internal representation of product to output buffer

  prod.serialize(symprod_buf);

  return(0);
  
}

void Server::_addText(const Params *serverParams,
		      Edr::edr_t &edrRep,
		      Symprod &prod)

{
  
  int iconSize = serverParams->icon_size;
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // flight num
  
  if (serverParams->flight_num_label.do_draw ) {
  
    char *text = (char*)edrRep.flightNum;
    char *color = serverParams->flight_num_label.color;
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Flight_Num text : " << text << endl;
    }
    
    prod.addText(text,
		 edrRep.lat, edrRep.lon, color,
		 serverParams->flight_num_label.background_color,
		 serverParams->flight_num_label.x_offset,
		 serverParams->flight_num_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->flight_num_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->flight_num_label.horiz_align,
		 serverParams->flight_num_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->flight_num_label.font_name,
		 0, detail_level);
  }

  // flight level
  
  if (serverParams->flight_level_label.do_draw ) {
    char *color = serverParams->flight_level_label.color;
    char text[64];
    int flevel = (int) (edrRep.alt / 100.0 + 0.5);
    sprintf(text, "FL%.3d", flevel);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Flight_Level text : " << text << endl;
    }

    prod.addText(text,
		 edrRep.lat, edrRep.lon, color,
		 serverParams->flight_level_label.background_color,
		 serverParams->flight_level_label.x_offset,
		 serverParams->flight_level_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->flight_level_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->flight_level_label.horiz_align,
		 serverParams->flight_level_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->flight_level_label.font_name,
		 0, detail_level);
  }

  // Edr Average 

  if (serverParams->edr_ave_label.do_draw ) {
	
    char *color = NULL;
    for(int k = serverParams->NumEdrVals-1; k >= 0; k--)
      if(edrRep.edrAve <= serverParams->_EdrMaxVals[k])
	color = serverParams->_EdrColors[k];

    char text[64];
    sprintf(text, "%.2fA", edrRep.edrAve);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Edr Ave text : " << text << endl;
    }

    prod.addText(text,
		 edrRep.lat, edrRep.lon, color,
		 serverParams->edr_ave_label.background_color,
		 serverParams->edr_ave_label.x_offset,
		 serverParams->edr_ave_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->edr_ave_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->edr_ave_label.horiz_align,
		 serverParams->edr_ave_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->edr_ave_label.font_name,
		 0, detail_level);
  }


  // Edr Peak 

  if (serverParams->edr_peak_label.do_draw ) {
	
    char *color = NULL;
    for(int k = serverParams->NumEdrVals-1; k >= 0; k--)
      if(edrRep.edrPeak <= serverParams->_EdrMaxVals[k])
	color = serverParams->_EdrColors[k];

    char text[64];
    sprintf(text, "%.2fP", edrRep.edrPeak);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Edr Peak text : " << text << endl;
    }

    prod.addText(text,
		 edrRep.lat, edrRep.lon, color,
		 serverParams->edr_peak_label.background_color,
		 serverParams->edr_peak_label.x_offset,
		 serverParams->edr_peak_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->edr_peak_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->edr_peak_label.horiz_align,
		 serverParams->edr_peak_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->edr_peak_label.font_name,
		 0, detail_level);
  }

  // Temperature

  if (serverParams->temperature_label.do_draw ) {
    char *color = serverParams->temperature_label.color;
    char text[64];
    sprintf(text, "%.1fF", edrRep.sat);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Temperature text : " << text << endl;
    }

    prod.addText(text,
		 edrRep.lat, edrRep.lon, color,
		 serverParams->temperature_label.background_color,
		 serverParams->temperature_label.x_offset,
		 serverParams->temperature_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->temperature_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->temperature_label.horiz_align,
		 serverParams->temperature_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->temperature_label.font_name,
		 0, detail_level);
  }

  // Wind Direction and Speed

  if (serverParams->wind_label.do_draw ) {
    char *color = serverParams->wind_label.color;
    char text[64];
    sprintf(text, "%.0f/%.0fdeg", edrRep.wspd, edrRep.wdir);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Wind text : " << text << endl;
    }

    prod.addText(text,
		 edrRep.lat, edrRep.lon, color,
		 serverParams->wind_label.background_color,
		 serverParams->wind_label.x_offset,
		 serverParams->wind_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->wind_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->wind_label.horiz_align,
		 serverParams->wind_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->wind_label.font_name,
		 0, detail_level);
  }

}

void Server::_addIcon(const Params *serverParams,
		      Edr::edr_t &edrRep,
		      Symprod &prod)

{
  
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // set up the icon
  
  int sz = serverParams->icon_size;
  
  Symprod::ppt_t icon[3] =
  {
    {  0,          sz  },
    { (si32) (sz * 1.1 + 0.5),   (si32) (-sz * 0.5 + 0.5) },
    { (si32) (-sz * 1.1 + 0.5),  (si32) (-sz * 0.5 + 0.5) }
  };
  
  // Add icon
  float edrVal = edrRep.edrPeak;
  if(serverParams->use_edrAve)
    edrVal = edrRep.edrAve;

  char *color = NULL;
  for(int k = serverParams->NumEdrVals-1; k >= 0; k--)
    if(edrVal <= serverParams->_EdrMaxVals[k])
      color = serverParams->_EdrColors[k];

  prod.addIconline(edrRep.lat, edrRep.lon,
		   3, icon,
		   color,
		   Symprod::LINETYPE_SOLID, 1,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL,
		   true, Symprod::FILL_SOLID,
		   0, detail_level);
  
  if (_isVerbose) {
    cerr << "Adding icon, lat, lon: "
	 << edrRep.lat << ", "
	 << edrRep.lon << endl;
  }

}

double Server::_nearest(double target, double delta)
{
  
  double answer;
  double rem;                                                                 
  
  delta = fabs(delta);                   
  rem = remainder(target,delta);
  
  if(target >= 0.0) {
    if(rem > (delta / 2.0)) {
      answer = target + (delta - rem);
    } else {
      answer = target -  rem;
    }
  } else {
    if(fabs(rem) > (delta / 2.0)) {
      answer = target - (delta + rem);
    } else {
      answer = target -  rem;
    }
  }
  
  return answer;

}

