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
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2008
//
///////////////////////////////////////////////////////////////


#ifndef _Server_HH
#define _Server_HH

#include <string>

#include <Spdb/Symprod.hh>
#include <Spdb/DsSymprodServer.hh>
#include <toolsa/MemBuf.hh>

#include "Params.hh"
//#include "IcingInstance.hh"
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

  // overload transformData()

  void transformData(const void *serverParams,
		     const string &dir_path,
		     int prod_id,
		     const string &prod_label,
		     int n_chunks_in,
		     const Spdb::chunk_ref_t *chunk_refs_in,
		     const Spdb::aux_ref_t *aux_refs_in,
		     const void *chunk_data_in,
		     int &n_chunks_out,
		     MemBuf &refBufOut,
		     MemBuf &auxBufOut,
		     MemBuf &dataBufOut);
  
  
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


private:

  // Create a symprod from the cell, and add it to prod
  void _addCell(Params *serverParams, Symprod &prod,
	       int floor, int ceiling, float center_lat,
	       float center_lon, vector<double> lats,
	       vector<double> lons);

  void _addPolygon(Params *serverParams, Symprod &prod,
		   float center_lat, float center_lon, 
		   vector<double> lats, vector<double> lons,
		   char *color);


  void _addText(Params *serverParams, Symprod &prod, int floor, int ceiling,
		float center_lat,float center_lon);

  // convert XML icing airmet data to symprod format
  int _xmlToSymprod(Params *serverParams,
		    const string &dir_path,
		    const int prod_id,
		    const string &prod_label,
		    const Spdb::chunk_ref_t &chunk_ref,
		    const Spdb::aux_ref_t &aux_ref,
		    const char *xml_str,
		    MemBuf &symprod_buf);
  



  // Convert the TDRP capstyle parameter to the matching symprod value.

  Symprod::capstyle_t _convertCapstyleParam(int capstyle);
  
  // Convert the TDRP joinstyle parameter to the matching symprod value.

  Symprod::joinstyle_t _convertJoinstyleParam(int joinstyle);
  
  // Convert the TDRP line type parameter to the matching symprod value.

  Symprod::linetype_t _convertLineTypeParam(int line_type);

  // convert XML version to Symprod
};

#endif
