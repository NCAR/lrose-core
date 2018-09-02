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
// Server.hh
//
// FileServerobject
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////


#ifndef _Server_HH
#define _Server_HH


#include <string>
#include <Spdb/Symprod.hh>
#include <Spdb/DsSymprodServer.hh>
#include <toolsa/MemBuf.hh>

#include "Params.hh"
using namespace std;

class Server : public DsSymprodServer
{

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  Server(const string &prog_name,
	 Params       *initialParams);

  // destructor

  virtual ~Server(){};
  
protected:

  // methods invoked from the base class for managing 
  // local override of parameter file

  int      loadLocalParams( const string &paramFile, void **params);
  void     freeLocalParams( void *params )
                          { delete (Params*)params; }

  // Convert the given data chunk from the SPDB database to symprod format.

  int convertToSymprod(const void *params,
		       const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const Spdb::aux_ref_t &aux_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       MemBuf &symprod_buf);

  void compute_geometry(double center_lat,
			double center_lon,
			double radius,
			double *SegLat, double *SegLon,
			double &TimeLat, double &TimeLon);
 
  void add_circle(Params *serverParams,
		  Symprod &prod,
		  int start_index,
		  int end_index,
		  char *color, int lineWidth,
		  double *SegLat, double *SegLon);

  Symprod::capstyle_t convert_capstyle_param(int capstyle);
  Symprod::joinstyle_t convert_joinstyle_param(int joinstyle);
  Symprod::linetype_t convert_line_type_param(int line_type);

private:

};

#endif
